#pragma once
#include "prescaler.h"

class Timer1 {
  uint16_t prescaler = 256;
  uint8_t bits_prescaler = 0b100; // 256
  uint16_t comparador = 0;

public:
  constexpr Timer1(float frecuencia) {
    prescaler = elegir_prescaler(frecuencia, 65535);
    bits_prescaler = obtener_bits_prescaler(prescaler);

    comparador = 16e6 / (prescaler * frecuencia) - 1;
  }

  void setup(){
    const int modo = 4;
    const uint8_t wgm10 = modo & 0b11;
    const uint8_t wgm32 = modo & 0b1100;

    TCCR1A = wgm10;
    TCCR1B = wgm32 << 1;

    OCR1A = comparador;
  }

  void start(){
    // if (bits_prescaler == 0)
    // Frecuencia fuera de rango

    // Activar interrupción
    TIMSK1 = 1 << OCIE1A;

    TCNT1 = 0;
    TCCR1B |= bits_prescaler;
  }

  void stop(){
    TCCR1B &= 0b11111000;

    // Desactivar interrupción
    TIMSK1 = 0;
  }

  void set_frequency(float frecuencia){
    prescaler = elegir_prescaler(frecuencia, 65535.0);
    bits_prescaler = obtener_bits_prescaler(prescaler);

    comparador = 16e6 / (prescaler * frecuencia) - 1;
    OCR1A = comparador;
    stop();
    start();
}
};
