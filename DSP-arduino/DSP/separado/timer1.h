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

  void setup();
  void start();
  void stop();
  void set_frequency(float frecuencia);
}; 
