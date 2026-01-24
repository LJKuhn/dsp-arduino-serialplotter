// #include "usart.h"
#include "adc.h"
#include "timer1.h"
#include "tablas.h"
#include <avr/io.h>
#include <avr/interrupt.h>


ADCController adc;
Timer1 timer1(11520.0);

void write(uint8_t valor){
  uint8_t lsb = valor << 2;
  uint8_t msb = valor >> 6;
  // Escribir los 6 LSB a los pines 2-7
  PORTD = lsb;
  // Escribir los 2 MSB a los pines 8 y 9
  PORTB = msb;
}

uint8_t counter = 0;
uint8_t forma = 0;
uint8_t n = 0;
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

void setup()
{
   Serial.begin(115200, SERIAL_8N1);
  //  adc.begin(1);

   // Pin 2-7 como salida:
   DDRD = 0b11111100;
   // Pin 8 y 9 como salida:
   DDRB = 0b00000011;

   timer1.setup();
   timer1.start();
}

// bool run = true;
void loop()
{
   // static uint16_t end = millis() + 1000, count = 0;
   // uint16_t now = millis();
   // if (run && now >= end){
   //    end = now + 1000;
   //    if (count > 0){
   //       run = false;
   //       Serial.write(0);
   //       Serial.write(0);
   //       Serial.write(0);
   //       Serial.write(0);
   //       Serial.print(count);
   //       Serial.println(" bytes received");
   //    }
   // }

   if (beat){
      beat = false;

      Serial.write(senoidal[++counter]);
      // uint8_t valor_adc = adc.ahora(1);
      // Serial.write(valor_adc);

      if (Serial.available())
         valor = Serial.read();
   }

   // if (adc.available())
   //    Serial.write(adc.get());
   // Serial.write(senoidal[++counter]);

}
