#include "usart.h"
#include <avr/io.h>

constexpr int TRANSMIT = 1 << TXEN0;
constexpr int RECEIVE = 1 << RXEN0;
constexpr int TX_COMPLETE_INTERRUPT = 1 << TXCIE0;
constexpr int RX_COMPLETE_INTERRUPT = 1 << RXCIE0;
constexpr int UDR_EMPTY_INTERRUPT = 1 << UDRIE0;

void USART::data_received(){
  if (rx_end + 1 == rx_start)
      return;

  rx_buffer[rx_end] = UDR0;
  rx_end = (rx_end + 1) % sizeof(rx_buffer);
}

void USART::udr_empty(){
  UDR0 = tx_buffer[tx_start];
  tx_start = (tx_start + 1) % sizeof(tx_buffer);

  if (tx_start == tx_end)
      UCSR0B &= ~UDR_EMPTY_INTERRUPT;
}

void USART::begin(uint32_t bps = 9600, int bits = 8)
{
  if (bits < 5 || bits > 8)
      return;

  const uint8_t interrupts = RX_COMPLETE_INTERRUPT;
  UBRR0 = 16000000ULL / 8 / bps - 1;
  UCSR0A = 1 << U2X0;
  UCSR0B = TRANSMIT | RECEIVE | interrupts;
  UCSR0C = (bits - 5 & 0b11) << UCSZ00;
}

bool USART::available(){
  return rx_start != rx_end;
}

char USART::get(){
  char current = rx_buffer[rx_start];
  rx_start = (rx_start + 1) % sizeof(rx_buffer);
  return current;
}

void USART::put(char c){
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

void USART::write(uint16_t n){
  write(n & 0xFF);
  write(n >> 8);
}

void USART::print(const char *n)
{
  for (int i = 0; n[i]; i++)
  {
      put(n[i]);
  }
}

void USART::print(uint16_t n)
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

void USART::println(uint16_t n)
{
  print(n);
  put('\n');
}

void USART::println(const char *n)
{
  print(n);
  put('\n');
}

USART& USART::operator<<(const char n)
{
  put(n);
  return *this;
}

USART& USART::operator<<(const char *n)
{
  print(n);
  return *this;
}

USART& USART::operator<<(const uint8_t n)
{
  print(n);
  return *this;
}

USART& USART::operator<<(const uint16_t n)
{
  print(n);
  return *this;
}
