#include "adc.h"
#include <avr/io.h>

constexpr uint8_t ACTIVAR = 1 << ADEN;
constexpr uint8_t EMPEZAR = 1 << ADSC;
constexpr uint8_t AUTO_TRIGGER = 1 << ADATE;
constexpr uint8_t PRESCALER_128 = 7;
constexpr uint8_t MODO_CONTINUO = 0;
constexpr uint8_t AREF = 0;
constexpr uint8_t AVcc = 1 << REFS0;
constexpr uint8_t AV1_1 = 3 << REFS0;
constexpr uint8_t ADC_INTERRUPT = 1 << ADIE;

void ADCController::conversion_complete()
{
  uint8_t low = ADCL;
  uint8_t high = ADCH;
  not_get = true;
  data = high << 8 | low;
}

void ADCController::begin(int pin)
{
  // Activar, Auto Trigger, ADC Interrupt y factor 128
  ADCSRA = ACTIVAR | AUTO_TRIGGER | PRESCALER_128 | ADC_INTERRUPT;

  ADCSRB = MODO_CONTINUO;

  // Utiliza A1 y usa AVcc
  ADMUX = AVcc | pin;

  // Comenzar la lectura
  ADCSRA |= EMPEZAR;
}

uint16_t ADCController::get()
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