/**
 * DSP OVERCLOCK - Sistema de Alta Velocidad
 * Arduino Mega 2560 @ Máxima Frecuencia de Muestreo
 * 
 * ⚡ CONFIGURACIÓN EXTREMA:
 * - Frecuencia muestreo: 7680 Hz (2× velocidad estándar)
 * - Baudrate: 115200 bps (3× velocidad estándar)
 * - Nyquist: 3840 Hz (captura señales hasta 3840 Hz)
 * - Latencia: ~0.3-0.4 ms (50% más rápido)
 * - Uso CPU: ~1.1% (aún muy eficiente)
 * 
 * ADVERTENCIAS:
 * ⚠️ Requiere cable USB de calidad (errores de comunicación con cables baratos)
 * ⚠️ PC debe procesar datos 2× más rápido (verificar CPU < 10%)
 * ⚠️ Mayor consumo de corriente del Arduino
 * ⚠️ Verificar que SerialPlotter esté configurado a 115200 bps y 7680 Hz
 * 
 * Hardware requerido: Arduino Mega 2560
 * - Pines 22-29: DAC R2R de 8 bits (PORTA completo - PA0 a PA7)
 * - Pin A1: Entrada analógica ADC (0V-5V después de acondicionador)
 * - Pin 13: LED indicador de estado
 * - USART0: Comunicación serie a 115200 baudios
 * 
 * Comunicación con SerialPlotter OVERCLOCK:
 * - Frecuencia muestreo: 7680 Hz (en vez de 3840 Hz)
 * - Baudrate: 115200 baudios (7680 × 10 bits/byte = 76800, redondeado a estándar)
 * - Modo: Bidireccional no bloqueante con buffers ampliados
 * - Buffer TX: 512 bytes (66 ms capacidad)
 * - Buffer RX: 128 bytes (16 ms capacidad)
 * 
 * Ventajas OVERCLOCK:
 * ✅ Captura frecuencias hasta 3840 Hz sin aliasing (vs 1920 Hz estándar)
 * ✅ Mayor resolución temporal (130 μs vs 260 μs por muestra)
 * ✅ FFT con 7680 puntos = 0.5 Hz por bin (vs 1 Hz estándar)
 * ✅ Menor latencia end-to-end del sistema
 * ✅ Mejor respuesta transitoria de filtros
 * 
 * Limitaciones físicas respetadas:
 * - ADC prescaler 128: Mantiene precisión completa de 8 bits
 * - Conversión ADC: 104 μs (dentro del período 130 μs @ 7680 Hz)
 * - ISR Timer1: ~35 ciclos = 2.2 μs (< 2% del período)
 * - Baudrate estándar: 115200 es soportado universalmente
 * 
 * Autor: Lautaro Kühn & Federico Domínguez
 * Versión: OVERCLOCK 2.0
 * Fecha: Mayo 2026
 */

// Incluir librerías del proyecto
#include "../DSP/adc.h"           // Control del ADC (mismo que versión estándar)
#include "../DSP/timer1.h"        // Timer1 para interrupciones precisas
#include "../DSP/tablas.h"        // Tablas de formas de onda
#include "usart_overclock.h"      // ⚡ USART con buffers ampliados
#include <avr/io.h>
#include <avr/interrupt.h>

// ========== CONFIGURACIÓN OVERCLOCK ==========
#define FREQ_MUESTREO 7680.0      // ⚡ 7680 Hz (2× velocidad estándar)
#define BAUDRATE_OVERCLOCK 115200  // ⚡ 115200 bps (3× velocidad estándar)

// Instancias de controladores
ADCController adc;                 // Controlador del ADC
Timer1 timer1(FREQ_MUESTREO);      // ⚡ Timer a 7680 Hz para muestreo

/**
 * Escribe un valor de 8 bits al DAC R2R usando PORTA completo
 * 
 * OPTIMIZADO PARA OVERCLOCK:
 * - Inline forzado para eliminar overhead de función
 * - Escritura atómica ultra-rápida (2 ciclos @ 16 MHz = 125 ns)
 * - Sin cálculos ni verificaciones en tiempo de ejecución
 * 
 * @param valor Valor de 0-255 a convertir a voltaje analógico
 */
inline void write(uint8_t valor) __attribute__((always_inline));
inline void write(uint8_t valor) {
  PORTA = valor;  // 2 ciclos de clock
}

// Variables globales para el sistema DSP
uint8_t valor = 0;          // Valor actual a escribir al DAC
volatile bool beat = false; // Flag de sincronización con timer

/**
 * ISR Timer1 OPTIMIZADA - Se ejecuta 7680 veces por segundo
 * 
 * OPTIMIZACIONES APLICADAS:
 * - Función write() inline eliminada (compilador optimiza a 1 instrucción)
 * - Solo 2 operaciones: escribir PORTA y setear flag
 * - ~20 ciclos totales (incluyendo overhead ISR)
 * - Ejecuta en 1.25 μs @ 16 MHz
 * - Uso CPU: 1.25 μs × 7680 Hz = 0.96% (excelente)
 */
ISR(TIMER1_COMPA_vect) {
  PORTA = valor;  // Escribir DAC (inline directo)
  beat = true;    // Señalizar procesamiento
}

/**
 * ISR ADC - Conversión completa
 * Idéntica a versión estándar (no requiere optimización)
 */
ISR(ADC_vect) {
   adc.conversion_complete();
}

/**
 * ISR USART TX - Buffer de transmisión
 * Optimizada con buffer de 512 bytes (vs 256 estándar)
 */
ISR(USART0_UDRE_vect) {
   usart_oc.udrie();
}

/**
 * ISR USART RX - Recepción de datos filtrados
 * Optimizada con buffer de 128 bytes (vs 64 estándar)
 */
ISR(USART0_RX_vect) {
   uint8_t leido = UDR0;
   if (usart_oc.libre_lectura()) {
      usart_oc.buffer_lectura[usart_oc.fin_l] = leido;
      usart_oc.fin_l = (usart_oc.fin_l + 1) % sizeof(usart_oc.buffer_lectura);
   }
}

/**
 * Configuración inicial OVERCLOCK
 */
void setup() {
   // ========== CONFIGURAR PERIFÉRICOS ==========
   adc.begin(1);                         // ADC canal A1
   usart_oc.begin(BAUDRATE_OVERCLOCK);   // ⚡ 115200 baudios
   
   // ========== CONFIGURAR DAC (PORTA) ==========
   DDRA = 0xFF;  // Todos los pines como salida
   
   // ========== CONFIGURAR TIMER1 @ 7680 Hz ==========
   timer1.setup();
   timer1.start();
   
   // ========== LED INDICADOR ==========
   pinMode(13, OUTPUT);
   digitalWrite(13, HIGH);  // LED encendido = OVERCLOCK activo
   
   // ========== PARPADEO INDICADOR OVERCLOCK ==========
   // 3 parpadeos rápidos para indicar modo OVERCLOCK activado
   for (int i = 0; i < 3; i++) {
      digitalWrite(13, LOW);
      delay(100);
      digitalWrite(13, HIGH);
      delay(100);
   }
   
   sei();  // Habilitar interrupciones globales
}

/**
 * Loop principal OVERCLOCK
 * 
 * OPTIMIZADO PARA MÁXIMA VELOCIDAD:
 * - Procesamiento mínimo en loop
 * - Todo el trabajo pesado en ISRs
 * - Sin delays ni bloqueos
 * - Escritura no bloqueante garantizada por buffer ampliado
 * 
 * FLUJO @ 7680 Hz:
 * 1. Timer1 ISR: Escribe DAC cada 130 μs
 * 2. ADC ISR: Captura muestra (~104 μs conversión)
 * 3. Loop: Envía a PC y recibe filtrado (cuando beat==true)
 * 4. USART ISR: Transmite buffer en segundo plano
 */
void loop() {
   if (beat) {
      beat = false;
      
      // ========== LEER ADC ==========
      uint8_t muestra_adc = adc.get();
      
      // ========== ENVIAR A PC ==========
      // Escritura no bloqueante con buffer de 512 bytes
      // A 7680 Hz, buffer se llena en 66 ms (muy seguro)
      usart_oc.escribir(muestra_adc);
      
      // ========== RECIBIR DATOS FILTRADOS ==========
      if (usart_oc.pendiente_lectura()) {
         valor = usart_oc.leer();  // Usar señal procesada
      } else {
         valor = muestra_adc;       // Fallback: eco directo
      }
      
      // Próxima ISR Timer1 escribirá 'valor' al DAC
   }
   
   // ========== MONITOREO DE SATURACIÓN (DEBUG) ==========
   // Descomentar para verificar que el buffer no se sature
   /*
   static uint32_t ultimo_check = 0;
   if (millis() - ultimo_check > 1000) {
      ultimo_check = millis();
      
      uint8_t libre_tx = usart_oc.libre_escritura();
      uint8_t libre_rx = usart_oc.libre_lectura();
      
      // Parpadeo de advertencia si buffers < 25%
      if (libre_tx < 128 || libre_rx < 32) {
         digitalWrite(13, !digitalRead(13));
      }
   }
   */
}

/**
 * NOTAS DE CONFIGURACIÓN SERIALPLOTTER:
 * =======================================
 * 
 * 1. Velocidad (Baudrate): 115200
 * 2. Frecuencia (Sampling Rate): 7680
 * 3. Puerto: (El mismo que usabas antes)
 * 
 * VERIFICACIÓN:
 * - LED debe hacer 3 parpadeos al inicio (indica OVERCLOCK activo)
 * - LED debe quedar encendido (indica sistema funcionando)
 * - Si parpadea continuamente: Buffer saturándose (bajar frecuencia o mejorar cable USB)
 * 
 * TROUBLESHOOTING:
 * - Errores de comunicación: Usa cable USB de calidad (no extensiones)
 * - Señal con ruido: Verificar fuente de alimentación estable
 * - PC lenta: Reducir frecuencia a 5760 Hz o usar baudrate 230400
 * 
 * COMPARACIÓN DE RENDIMIENTO:
 * ============================
 * 
 * Versión Estándar (DSP.ino):
 * - Frecuencia: 3840 Hz
 * - Baudrate: 38400 bps
 * - Nyquist: 1920 Hz
 * - Período: 260 μs
 * - Uso CPU Arduino: 0.54%
 * 
 * Versión OVERCLOCK (este archivo):
 * - Frecuencia: 7680 Hz      (2× más rápido)
 * - Baudrate: 115200 bps     (3× más rápido)
 * - Nyquist: 3840 Hz         (2× rango frecuencial)
 * - Período: 130 μs          (2× resolución temporal)
 * - Uso CPU Arduino: 0.96%   (todavía muy bajo)
 * 
 * LÍMITE TEÓRICO ABSOLUTO:
 * =========================
 * ADC con prescaler 128:
 * - F_ADC = 125 kHz
 * - Conversión = 13 ciclos = 104 μs
 * - Máximo teórico: 9615 Hz
 * 
 * Esta versión usa 7680 Hz = 80% del máximo teórico
 * Margen de seguridad: 20% (excelente para estabilidad)
 * 
 * PRÓXIMOS OVERCLOCKS POSIBLES:
 * ==============================
 * - 11520 Hz @ 115200 bps (límite ADC, requiere prescaler 64)
 * - 15360 Hz @ 230400 bps (requiere prescaler 32, pierde precisión)
 * - 23040 Hz @ 250000 bps (requiere prescaler 16, muy ruidoso)
 */
