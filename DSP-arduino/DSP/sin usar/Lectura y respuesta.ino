// #include "usart.h"
#include "adc.h"
#include "timer1.h"
#include "tablas.h"
#include "usart.h"
#include <avr/io.h>
#include <avr/interrupt.h>

ADCController adc;
Timer1 timer1(3840.0);

void write(uint8_t valor){
  uint8_t lsb = valor << 2;
  uint8_t msb = valor >> 6;
  // Escribir los 6 LSB a los pines 2-7
  PORTD = lsb;
  // Escribir los 2 MSB a los pines 8 y 9
  PORTB = msb;
}

uint8_t counter = 0;
uint8_t valor = 0;
volatile bool beat = false;
ISR(TIMER1_COMPA_vect)
{
  // uint8_t valor = senoidal[n++];
  write(valor);
  beat = true;

  // print = true;
}

ISR(ADC_vect)
{
   adc.conversion_complete();
}

ISR(USART_UDRE_vect)
{
   usart.udrie();
}

ISR(USART_RX_vect)
{
   uint8_t leido = UDR0;
   if (usart.libre_lectura()){
      usart.buffer_lectura[usart.fin_l] = leido;
      usart.fin_l = (usart.fin_l + 1) % sizeof(usart.buffer_lectura);
   }
}

void setup()
{
   // Serial.begin(115200, SERIAL_8N1);
   adc.begin(1);
   usart.begin(250000);

   // Pin 2-7 como salida:
   DDRD = 0b11111100;
   // Pin 8 y 9 como salida:
   DDRB = 0b00000011;

   timer1.setup();
   timer1.start();


   pinMode(13, OUTPUT);
   digitalWrite(13, HIGH);
}

uint8_t i = 0;
bool enviar = true;
bool primera_vez = true;
uint32_t inicio;

// bool run = true;
void loop()
{
   // usart.escribir(i);
   if (i < 250 && usart.pendiente_lectura() > 0){
      uint8_t valor = usart.leer();
      if (primera_vez){
         inicio = millis();
         primera_vez =false;
      }
      // usart.escribir(0xAB);
      usart.escribir(valor);
      i++;
   }
   else if (i == 250 && enviar) {
      enviar = false;
      // for (size_t i = 0; i < 250; i++)
      //    usart.escribir(buffer[buffer_i]);

      uint32_t tiempo = millis() - inicio;
      usart.escribir(' ');
      usart.escribir(' ');
      usart.escribir('T');
      usart.escribir(tiempo & 255);
      usart.escribir((tiempo >> 8) & 255);
      usart.escribir((tiempo >> 16) & 255);
      usart.escribir((tiempo >> 24) & 255);
   }

   // if (beat){
   //    beat = false;

   //    usart.escribir(senoidal[++counter]);

   //    if (usart.pendiente_lectura()){
   //       valor = usart.leer();
   //    }
   //    else
   //       valor = 255 - adc.get();
   // }
}
