#pragma once
#include <stdint.h>
#include <avr/io.h>

extern "C" void USART_RX_vect (void) __attribute__ ((signal,used, externally_visible));
extern "C" void USART_UDRE_vect (void) __attribute__ ((signal,used, externally_visible));

class USART
{
private:
   constexpr static int TRANSMIT = 1 << TXEN0;
   constexpr static int RECEIVE = 1 << RXEN0;
   constexpr static int TX_COMPLETE_INTERRUPT = 1 << TXCIE0;
   constexpr static int RX_COMPLETE_INTERRUPT = 1 << RXCIE0;
   constexpr static int UDR_EMPTY_INTERRUPT = 1 << UDRIE0;

   volatile char rx_buffer[100];
   volatile int rx_start = 0;
   volatile int rx_end = 0;

   volatile char tx_buffer[100];
   volatile int tx_start = 0;
   volatile int tx_end = 0;

   void data_received(){
      if (rx_end + 1 == rx_start)
         return;

      rx_buffer[rx_end] = UDR0;
      rx_end = (rx_end + 1) % sizeof(rx_buffer);
   }

   void udr_empty(){
      UDR0 = tx_buffer[tx_start];
      tx_start = (tx_start + 1) % sizeof(tx_buffer);

      if (tx_start == tx_end)
         UCSR0B &= ~UDR_EMPTY_INTERRUPT;
   }

   friend void USART_RX_vect (void);
   friend void USART_UDRE_vect (void);

public:

   void begin(uint32_t bps = 9600, int bits = 8)
   {
      if (bits < 5 || bits > 8)
         return;

      const uint8_t interrupts = RX_COMPLETE_INTERRUPT;
      UBRR0 = 16000000ULL / 8 / bps - 1;
      UCSR0A = 1 << U2X0;
      UCSR0B = TRANSMIT | RECEIVE | interrupts;
      UCSR0C = (bits - 5 & 0b11) << UCSZ00;
   }

   //  0123456789
   //  0123456701
   //   e   s   e
   //  Disponible: 4
   uint8_t available(){
      uint8_t available = rx_end - rx_start;
      if (rx_end < rx_start)
         available += sizeof(rx_buffer);
      return available;
      // return rx_start != rx_end;
   }

   char peek(){
      return rx_buffer[rx_start];
   }

   char get(){
      char current = rx_buffer[rx_start];
      rx_start = (rx_start + 1) % sizeof(rx_buffer);
      return current;
   }

   uint8_t get_immediate(){
      return UDR0;
   }

   void put(char c){
      bool empty = UCSR0A & 1 << UDRE0;
      if (empty && tx_start == tx_end){
         UDR0 = c;
         return;
      }

      // Buffer lleno, descartar
      if (tx_end + 1 == tx_start)
         return;

      tx_buffer[tx_end] = c;
      tx_end = (tx_end + 1) % sizeof(tx_buffer);
      UCSR0B |= UDR_EMPTY_INTERRUPT;
   }

   void write(uint16_t n){
     char* ptr = (char*)&n;
     put(ptr[0]);
     put(ptr[1]);
   }

   void print(const char *n)
   {
      for (int i = 0; n[i]; i++)
      {
         put(n[i]);
      }
   }

   void print(uint16_t n)
   {
      char buffer[5];

      constexpr int max = sizeof(buffer) - 1;
      int i = 0;
      do
      {
         buffer[i++] = n % 10;
         n /= 10;
      } while (n > 0);
      while (--i >= 0)
         put(buffer[i] | '0');
   }

   void println(uint16_t n)
   {
      print(n);
      put('\n');
   }

   void println(const char *n)
   {
      print(n);
      put('\n');
   }

   USART& operator<<(const char n)
   {
      put(n);
      return *this;
   }

   USART& operator<<(const char *n)
   {
      print(n);
      return *this;
   }

   USART& operator<<(const uint8_t n)
   {
      print(n);
      return *this;
   }

   USART& operator<<(const uint16_t n)
   {
      print(n);
      return *this;
   }
};

extern USART usart;
