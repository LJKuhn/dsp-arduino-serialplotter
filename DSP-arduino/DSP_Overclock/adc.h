#pragma once
#include <stdint.h>

class ADCController
{
   uint16_t data = -1;
   bool not_get = false;

public:
   void conversion_complete();

   void begin(int pin);

   uint8_t get();

   bool available();

   void start();

   void stop();

   uint8_t ahora(int pin);
};