#include "adc.h"
#include <avr/io.h>

// ════════════════════════════════════════════════════════════════════════════════════════
// 🎯 CONSTANTES DE CONFIGURACIÓN DEL ADC
// ════════════════════════════════════════════════════════════════════════════════════════

// Bits de control ADCSRA
constexpr uint8_t ACTIVAR = 1 << ADEN;              // Habilitar ADC
constexpr uint8_t EMPEZAR = 1 << ADSC;              // Iniciar conversión
constexpr uint8_t AUTO_TRIGGER = 1 << ADATE;        // Activar modo auto-trigger
constexpr uint8_t ADC_INTERRUPT = 1 << ADIE;        // Habilitar interrupción ADC

// Prescalers disponibles (ADPS2:ADPS0)
constexpr uint8_t PRESCALER_64 = 6;                 // /64: F_ADC = 250kHz (rápido)
constexpr uint8_t PRESCALER_128 = 7;                // /128: F_ADC = 125kHz (preciso) ← USADO

// Bits de control ADMUX
constexpr uint8_t AJUSTAR_IZQUIERDA = 1 << ADLAR;   // Justificación izq (8-bit en ADCH)

// Referencias de voltaje (REFS1:REFS0)
constexpr uint8_t AREF = 0;                         // AREF externo
constexpr uint8_t AVcc = 1 << REFS0;                // AVcc (5V) ← USADO
constexpr uint8_t AV1_1 = 3 << REFS0;               // 1.1V interno

// Modos de disparo ADCSRB
constexpr uint8_t MODO_CONTINUO = 0;                // Free running (auto-trigger continuo)

/**
 * ════════════════════════════════════════════════════════════════════════════════════════
 * IMPLEMENTACIÓN: Conversión completa (llamado por ISR)
 * ════════════════════════════════════════════════════════════════════════════════════════
 * 
 * NOTA: Usamos solo ADCH (8 bits) por rendimiento.
 * Si se necesitaran 10 bits completos:
 *   uint8_t low = ADCL;   // Leer primero low (desbloquea registros)
 *   uint8_t high = ADCH;
 *   data = (high << 8) | low;
 */
void ADCController::conversion_complete()
{
  uint8_t high = ADCH;  // Leer solo los 8 bits superiores
  not_get = true;       // Marcar como dato nuevo disponible
/**
 * ════════════════════════════════════════════════════════════════════════════════════════
 * IMPLEMENTACIÓN: Inicializar ADC
 * ════════════════════════════════════════════════════════════════════════════════════════
 * 
 * REGISTROS CONFIGURADOS:
 * 
 * ADCSRA (Control y Estado A):
 *   ADEN = 1: Habilitar ADC
 *   ADATE = 1: Auto-trigger habilitado
 *   ADIE = 1: Interrupciones habilitadas
 *   ADPS = 111: Prescaler /128 → F_ADC = 16MHz/128 = 125kHz
 * 
 * ADCSRB (Control y Estado B):
 *   ADTS = 000: Modo free-running (conversión continua automática)
 * 
 * ADMUX (Multiplexer):
 *   REFS = 01: Referencia AVcc con capacitor en AREF
 *   ADLAR = 1: Justificación izquierda (8-bit en ADCH)
 *   MUX = pin: Canal seleccionado (típicamente A1)
 */
void ADCController::begin(int pin)
{
  // Configurar control: Activar + Auto-trigger + Interrupt + Prescaler 128
  ADCSRA = ACTIVAR | AUTO_TRIGGER | PRESCALER_128 | ADC_INTERRUPT;

  // Configurar modo de disparo: Continuo (free-running)
  ADCSRB = MODO_CONTINUO;

  // Configurar multiplexer: AVcc reference + Left adjust + canal
  ADMUX = AVcc | AJUSTAR_IZQUIERDA | pin;

  // Iniciar primera conversión (las siguientes serán automáticas)cc
  ADMUX = AVcc | AJUSTAR_IZQUIERDA | pin;

  // Comenzar la lectura
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
