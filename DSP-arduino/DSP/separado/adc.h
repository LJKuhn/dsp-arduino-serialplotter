#pragma once
#include <stdint.h>

class ADCController
{
   uint16_t data = -1;
   bool not_get = false;

   void conversion_complete();

   friend void ADC_vect();

public:

   void begin(int pin);

   uint16_t get();

   bool available();

   void start();

   void stop();
};