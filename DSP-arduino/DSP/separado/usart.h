#pragma once
#include <stdint.h>

extern "C" void USART_RX_vect (void);
extern "C" void USART_UDRE_vect (void);
extern "C" void ADC_vect (void);

class USART
{
private:

   char rx_buffer[64];
   int rx_start = 0;
   int rx_end = 0;

   char tx_buffer[64];
   int tx_start = 0;
   int tx_end = 0;

   void data_received();

   void udr_empty();

   friend void USART_RX_vect (void);
   friend void USART_UDRE_vect (void);

public:

   void begin(uint32_t bps = 9600, int bits = 8);

   bool available();

   char get();
   void put(char c);
   void put(uint16_t n);

   void print(const char *n);
   void print(uint16_t n);

   void println(uint16_t n);
   void println(const char *n);

   USART& operator<<(const char n);
   USART& operator<<(const char *n);

   USART& operator<<(const uint8_t n);
   USART& operator<<(const uint16_t n);
};