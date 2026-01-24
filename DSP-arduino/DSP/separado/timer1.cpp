#include <avr/io.h>
#include <HardwareSerial.h>
#include "timer1.h"

void Timer1::setup(){
  const int modo = 4;
  const uint8_t wgm10 = modo & 0b11;
  const uint8_t wgm32 = modo & 0b1100;

  TCCR1A = wgm10;
  TCCR1B = wgm32 << 1;

  OCR1A = comparador;
}

void Timer1::start(){
  if (bits_prescaler == 0)
    Serial.println("Frecuencia fuera de rango");
    
  // Activar interrupción
  TIMSK1 = 1 << OCIE1A;

  TCNT1 = 0;
  TCCR1B |= bits_prescaler;
}

void Timer1::stop(){
  TCCR1B &= 0b11111000;

  // Desactivar interrupción
  TIMSK1 = 0;
}

void Timer1::set_frequency(float frecuencia){
    prescaler = elegir_prescaler(frecuencia, 65535);
    bits_prescaler = obtener_bits_prescaler(prescaler);

    comparador = 16e6 / (prescaler * frecuencia) - 1;
    stop();
    start();
}