/**
 * DSP (Procesamiento Digital de Señales) - Arduino Mega 2560
 * 
 * Sistema de procesamiento bidireccional ADC ↔ PC ↔ DAC usando SerialPlotter.
 * Lee señales analógicas, las procesa con filtros digitales en PC y genera
 * salida analógica en tiempo real.
 * 
 * Hardware requerido: Arduino Mega 2560
 * - Pines 22-29: DAC R2R de 8 bits (PORTA completo - PA0 a PA7)
 * - Pin A1: Entrada analógica ADC (0V-5V después de acondicionador LM324)
 * - Pin 13: LED indicador de estado
 * - USART0: Comunicación serie a 38400 baudios
 * 
 * Comunicación con SerialPlotter:
 * - Frecuencia muestreo: 3840 Hz
 * - Baudrate: 38400 baudios (3840 × 10 bits/byte)
 * - Modo: Bidireccional no bloqueante
 * - Latencia: ~0.6-0.8 ms
 * 
 * Ventajas sobre Arduino Uno:
 * - Un solo puerto (PORTA) para los 8 bits = mayor eficiencia
 * - Menos jitter en la señal de salida
 * - Operación atómica de escritura al DAC
 */

// Incluir librerías del proyecto
#include "adc.h"       // Control del ADC
#include "timer1.h"    // Timer1 para interrupciones precisas  
#include "tablas.h"    // Tablas de formas de onda
#include "usart.h"     // Comunicación serie
#include <avr/io.h>
#include <avr/interrupt.h>

// Instancias de controladores
ADCController adc;           // Controlador del ADC
Timer1 timer1(3840.0);       // Timer a 3840 Hz para muestreo

/**
 * Escribe un valor de 8 bits al DAC R2R usando PORTA completo
 * 
 * Arduino Mega 2560 - Configuración optimizada:
 * - Pines 22-29 (PORTA): Los 8 bits completos del DAC
 * - Una sola operación de escritura = máxima eficiencia
 * - Escritura atómica = menor jitter en la señal
 * 
 * @param valor Valor de 0-255 a convertir a voltaje analógico
 * 
 * Mapeo de pines:
 * - Pin 22 (PA0) = Bit 0 (LSB)
 * - Pin 23 (PA1) = Bit 1
 * - Pin 24 (PA2) = Bit 2  
 * - Pin 25 (PA3) = Bit 3
 * - Pin 26 (PA4) = Bit 4
 * - Pin 27 (PA5) = Bit 5
 * - Pin 28 (PA6) = Bit 6
 * - Pin 29 (PA7) = Bit 7 (MSB)
 */
void write(uint8_t valor){
  // MEGA 2560: Escritura atómica en una sola instrucción (62.5 ns @ 16MHz)
  // UNO: Requería 2 escrituras (PORTD + PORTB) = ~125 ns + jitter entre bits
  // Mejora: 2x más rápido, 0% jitter, señal DAC más limpia
  PORTA = valor;  // Los 8 bits se escriben simultáneamente
}

// Variables globales para el sistema DSP
uint8_t valor = 0;          // Valor actual a escribir al DAC
volatile bool beat = false; // Flag de sincronización con timer

// Interrupción del Timer1: Se ejecuta a frecuencia constante (3840 Hz)
// Genera la salida del DAC para crear formas de onda continuas
ISR(TIMER1_COMPA_vect)
{
  write(valor);  // Escribir valor actual al DAC
  beat = true;   // Señalizar que ocurrió una interrupción
}

// Interrupción del ADC: Conversión analógica completa
ISR(ADC_vect)
{
   adc.conversion_complete();
}

// Interrupción: Buffer de transmisión USART vacío
// Se ejecuta cuando el registro UDR0 está listo para recibir más datos
ISR(USART0_UDRE_vect)
{
   usart.udrie();
}

// Interrupción: Recepción USART completa
// Se ejecuta cuando llega un byte por el puerto serie
ISR(USART0_RX_vect)
{
   uint8_t leido = UDR0;
   if (usart.libre_lectura()){
      usart.buffer_lectura[usart.fin_l] = leido;
      usart.fin_l = (usart.fin_l + 1) % sizeof(usart.buffer_lectura);
   }
}

/**
 * Configuración inicial del sistema
 * Inicializa todos los periféricos y configura pines para Arduino Mega 2560
 */
void setup()
{
   // Inicializar periféricos
   adc.begin(1);        // Iniciar ADC en canal 1
   usart.begin(38400);  // Comunicación serie a 38400 baudios (sincronizado con SerialPlotter)

   // Configurar PORTA completo como salida para DAC (pines 22-29)
   // VENTAJA MEGA 2560: Un solo puerto completo vs Uno que necesita 2 puertos parciales
   // Arduino Mega: PORTA = pines 22-29 (PA0-PA7) - 8 bits contiguos
   // Arduino Uno: Requería PORTD (6 bits) + PORTB (2 bits) - glitches por escrituras separadas
   DDRA = 0xFF;  // Todos los pines de PORTA como salida (11111111 binario)

   // Configurar e iniciar Timer1
   timer1.setup();  // Configurar registros del timer
   timer1.start();  // Iniciar generación de interrupciones

   // Configurar LED indicador
   pinMode(13, OUTPUT);
   digitalWrite(13, false);  // LED apagado inicialmente
}

/**
 * Bucle principal del programa
 * 
 * Sistema DSP bidireccional en tiempo real:
 * 1. Lee señal analógica del ADC a 3840 Hz
 * 2. Envía muestra por serie a SerialPlotter para procesamiento
 * 3. Recibe datos procesados desde SerialPlotter
 * 4. Escribe al DAC para generar señal de salida
 */
void loop()
{
   // Sistema DSP bidireccional ADC ↔ PC ↔ DAC
   // 1. Lee señal analógica del ADC a 3840 Hz
   // 2. Envía muestra por serie a SerialPlotter para procesamiento
   // 3. Recibe datos procesados desde SerialPlotter
   // 4. Escribe al DAC para generar señal de salida
   if (beat){
      beat = false;
      
      // Enviar muestra actual por serie a la interfaz C++
      uint8_t muestra_adc = adc.get();           // Leer ADC (0-255)
      usart.escribir(muestra_adc);               // Enviar a PC para análisis/filtrado
      
      // Recibir datos procesados desde la interfaz C++
      if (usart.pendiente_lectura()){
         valor = usart.leer();                   // Usar señal filtrada/procesada de la PC
      }
      else {
         valor = muestra_adc;                    // Usar ADC directo como fallback
      }
      
      // El valor ya se escribirá al DAC en la próxima interrupción del Timer1
   }
}
