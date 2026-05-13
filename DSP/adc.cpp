#include "adc.h"
#include <avr/io.h>

// Constantes de configuración del ADC - Usando bit-shift para claridad
// Estas máscaras de bits configuran el comportamiento del conversor
constexpr uint8_t ACTIVAR = 1 << ADEN;              // Bit 7: Habilita el ADC
constexpr uint8_t EMPEZAR = 1 << ADSC;              // Bit 6: Inicia conversión
constexpr uint8_t AUTO_TRIGGER = 1 << ADATE;        // Bit 5: Modo auto-trigger
constexpr uint8_t AJUSTAR_IZQUIERDA = 1 << ADLAR;   // Bit 5: Left-adjust para lectura rápida 8-bit
constexpr uint8_t PRESCALER_64 = 6;                 // ADC clock = 16MHz/64 = 250kHz
constexpr uint8_t PRESCALER_128 = 7;                // ADC clock = 16MHz/128 = 125kHz (más preciso)
constexpr uint8_t MODO_CONTINUO = 0;
constexpr uint8_t AREF = 0;
constexpr uint8_t AVcc = 1 << REFS0;
constexpr uint8_t AV1_1 = 3 << REFS0;
constexpr uint8_t ADC_INTERRUPT = 1 << ADIE;

// VERSIÓN ANTERIOR: Lectura completa de 10 bits (requería leer ADCL primero, luego ADCH)
// void ADCController::conversion_complete()
// {
//   uint8_t low = ADCL;              // Leer primero ADCL (bloquea ADCH)
//   uint8_t high = ADCH;             // Luego ADCH
//   not_get = true;
//   data = high << 8 | low;          // Combinar en 10 bits
// }

// VERSIÓN OPTIMIZADA: Solo 8 bits más significativos (MSB)
// Ventajas: 50% más rápido, suficiente resolución para DSP de audio
// Usando ADLAR=1, los 8 MSB están en ADCH directamente
void ADCController::conversion_complete()
{
  uint8_t high = ADCH;  // Con ADLAR=1, esto contiene bits 9-2 (8 bits MSB)
  not_get = true;       // Marcar que hay datos nuevos disponibles
  data = high;          // Almacenar solo los 8 bits significativos
}

void ADCController::begin(int pin)
{
  // Configurar registro ADCSRA (ADC Control and Status Register A)
  // PRESCALER_128 = 125kHz ADC clock (óptimo para 16MHz: 50-200kHz recomendado)
  // Resultado: ~9600 conversiones/segundo (cada conversión toma 13 ciclos ADC)
  ADCSRA = ACTIVAR | AUTO_TRIGGER | PRESCALER_128 | ADC_INTERRUPT;

  // Configurar registro ADCSRB - Modo auto-trigger continuo
  // MODO_CONTINUO = conversiones automáticas sin necesidad de disparar manualmente
  ADCSRB = MODO_CONTINUO;

  // Configurar registro ADMUX (ADC Multiplexer Selection)
  // AVcc como referencia (5V), AJUSTAR_IZQUIERDA para lectura rápida de 8 bits
  ADMUX = AVcc | AJUSTAR_IZQUIERDA | pin;

  // Iniciar primera conversión (las siguientes serán automáticas)
  ADCSRA |= EMPEZAR;
}

uint8_t ADCController::get()
{
  not_get = false;
  return data;
}

bool ADCController::available(){
  return not_get;
}

void ADCController::start()
{
  ADCSRA |= EMPEZAR | AUTO_TRIGGER;
}

void ADCController::stop()
{
  ADCSRA &= ~(EMPEZAR | AUTO_TRIGGER);
}

uint8_t ADCController::ahora(int pin)
{
  // Utiliza A1 y usa AVcc
  ADMUX = AVcc | AJUSTAR_IZQUIERDA | pin;

  ADCSRA = ACTIVAR | PRESCALER_128 | EMPEZAR;

  while (ADCSRA & EMPEZAR);
  return ADCH;
}
