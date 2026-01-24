#pragma once
#include "prescaler.h"

/**
 * Clase para configurar y controlar el Timer1 del AVR
 * Genera interrupciones periódicas para DSP en tiempo real
 */
class Timer1 {
  uint16_t prescaler = 256;        // Divisor del clock base
  uint8_t bits_prescaler = 0b100;  // Bits CS12:CS10 para TCCR1B (prescaler 256)
  uint16_t comparador = 0;         // Valor para OCR1A

public:
  /**
   * Constructor: calcula automáticamente prescaler y valor de comparación
   * @param frecuencia Frecuencia deseada de interrupción en Hz
   */
  Timer1(float frecuencia) {
    // Calcular prescaler óptimo para la frecuencia deseada
    prescaler = elegir_prescaler(frecuencia, 65535);
    bits_prescaler = obtener_bits_prescaler(prescaler);
    
    // Calcular valor de comparación: OCR1A = (F_CPU / (prescaler * freq)) - 1
    comparador = 16e6 / (prescaler * frecuencia) - 1;
  }

  /**
   * Configurar registros del Timer1 en modo CTC (Clear Timer on Compare)
   * Configura el timer pero NO lo inicia
   */
  void setup(){
    const int modo = 4;                    // Modo CTC (Clear Timer on Compare)
    const uint8_t wgm10 = modo & 0b11;     // Bits WGM11:WGM10 para TCCR1A
    const uint8_t wgm32 = modo & 0b1100;   // Bits WGM13:WGM12 para TCCR1B

    // Configurar modo CTC
    TCCR1A = wgm10;           // WGM11:WGM10 = 00
    TCCR1B = wgm32 << 1;      // WGM13:WGM12 = 01 (desplazado a posición correcta)

    // Establecer valor de comparación
    OCR1A = comparador;
  }

  /**
   * Iniciar el Timer1 y habilitar interrupciones
   */
  void start(){
    // if (bits_prescaler == 0)
    // Frecuencia fuera de rango

    // Habilitar interrupción de comparación A
    TIMSK1 = 1 << OCIE1A;

    // Reiniciar contador y activar prescaler
    TCNT1 = 0;
    TCCR1B |= bits_prescaler;  // Escribir bits CS12:CS10
  }

  /**
   * Detener el Timer1 y deshabilitar interrupciones
   */
  void stop(){
    TCCR1B &= 0b11111000;  // Limpiar bits CS12:CS10 (parar timer)

    // Deshabilitar interrupción
    TIMSK1 = 0;
  }

  /**
   * Cambiar frecuencia del timer en tiempo real
   * @param frecuencia Nueva frecuencia en Hz
   */
  void set_frequency(float frecuencia){
    // Recalcular prescaler y comparador
    prescaler = elegir_prescaler(frecuencia, 65535.0);
    bits_prescaler = obtener_bits_prescaler(prescaler);

    comparador = 16e6 / (prescaler * frecuencia) - 1;
    OCR1A = comparador;
    
    // Reiniciar timer con nueva configuración
    stop();
    start();
  }
};