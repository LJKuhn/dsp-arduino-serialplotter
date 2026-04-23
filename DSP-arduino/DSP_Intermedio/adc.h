/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                       🎛️ ADC CONTROLLER - VERSIÓN DOCUMENTADA 🎛️                   ║
 * ║            Controlador ADC optimizado con explicaciones detalladas                  ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 📚 FILOSOFÍA:
 * Mantiene la eficiencia del ADC usando registros directos, ahora con
 * comentarios extensos para entender cada configuración.
 * 
 * CONFIGURACIONES CLAVE:
 * - Prescaler: 128 (F_ADC = 16MHz/128 = 125kHz) - Máxima precisión
 * - Modo: Auto-trigger continuo
 * - Resolución: 8 bits (ADCH, justificación izquierda)
 * - Referencia: AVcc (5V)
 */

#pragma once
#include <stdint.h>

extern "C" void ADC_vect (void);

/**
 * ════════════════════════════════════════════════════════════════════════════════════════
 * 🏗️ CLASE ADC CONTROLLER
 * ════════════════════════════════════════════════════════════════════════════════════════
 * 
 * Controlador del ADC de 10 bits del ATmega (usado en modo 8 bits por rendimiento).
 * 
 * FUNCIONAMIENTO:
 * 1. ADC funciona en modo continuo (auto-trigger)
 * 2. Cada conversión completa genera interrupción ADC_vect
 * 3. ISR llama a conversion_complete() que guarda el dato
 * 4. Loop principal lee con get() el último valor disponible
 * 
 * VENTAJAS DE AUTO-TRIGGER:
 * - No requiere iniciar cada conversión manualmente (ADSC)
 * - Frecuencia constante de muestreo
 * - Menor overhead de CPU
 */
class ADCController
{
   uint16_t data = -1;      // Último valor capturado (inicializado a -1)
   bool not_get = false;    // Flag: true = dato nuevo disponible

   /**
    * Llamado por ISR(ADC_vect) cuando termina una conversión
    * Guarda el valor y marca como disponible
    */
   void conversion_complete();

   friend void ADC_vect();  // Permite que ISR acceda a conversion_complete()

public:

   /**
    * Inicializar ADC en modo continuo
    * @param pin Canal ADC (0-15, típicamente A1 = 1)
    * 
    * CONFIGURACIÓN APLICADA:
    * - ADCSRA: Activar ADC + Auto-trigger + Interrupt + Prescaler 128
    * - ADCSRB: Modo continuo (free running)
    * - ADMUX: AVcc reference + Left adjust (8-bit) + canal
    */
   void begin(int pin);

   /**
    * Obtener último valor capturado (8 bits)
    * @return Valor ADC 0-255 (marca el dato como leído)
    */
   uint8_t get();

   /**
    * Verificar si hay un nuevo dato disponible
    * @return true si hay un valor nuevo sin leer
    */
   bool available();

   /**
    * Iniciar conversiones continuas (reanudar si estaba pausado)
    */
   void start();

   /**
    * Detener conversiones continuas
    */
   void stop();

   /**
    * Realizar UNA lectura bloqueante (modo single-shot)
    * @param pin Canal a leer
    * @return Valor ADC inmediato (8 bits)
    * 
    * NOTA: No usa auto-trigger, ideal para lecturas aisladas
    */
   uint8_t ahora(int pin);
};
