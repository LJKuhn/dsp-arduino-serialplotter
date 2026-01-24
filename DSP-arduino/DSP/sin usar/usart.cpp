#include "usart.h"

USART usart;

void USART_RX_vect (void)
{
   usart.data_received();
}

void USART_UDRE_vect (void)
{
   usart.udr_empty();
}
