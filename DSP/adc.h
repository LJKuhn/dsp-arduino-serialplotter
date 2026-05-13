#pragma once
#include <stdint.h>

extern "C" void ADC_vect (void);

class ADCController
{
   uint16_t data = -1;
   bool not_get = false;

   void conversion_complete();

   friend void ADC_vect();

public:

   void begin(int pin);

   uint8_t get();

   bool available();

   void start();

   void stop();

   uint8_t ahora(int pin);
};
