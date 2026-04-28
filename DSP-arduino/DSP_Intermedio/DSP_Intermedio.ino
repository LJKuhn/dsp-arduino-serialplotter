/**
 * ════════════════════════════════════════════════════════════════════════════════════════
 * 🎯 DSP INTERMEDIO - VERSIÓN LEGIBLE DEL SISTEMA ORIGINAL  
 * ════════════════════════════════════════════════════════════════════════════════════════
 * 
 * Sistema de procesamiento digital de señales bidireccional:
 * ADC → Arduino → PC (SerialPlotter) → Arduino → DAC
 * 
 * OBJETIVO: Mantener la funcionalidad exacta del DSP original pero con código más claro,
 * mejor organizado y fácil de entender para estudiantes y desarrolladores.
 * 
 * HARDWARE: Arduino Mega 2560
 * - Pin A1: Entrada ADC (señal analógica 0-5V)
 * - Pines 22-29: DAC R2R de 8 bits (PORTA completo)
 * - Pin 13: LED indicador de estado
 * - USART0: Comunicación serie con PC
 * 
 * ESPECIFICACIONES TÉCNICAS:
 * - Frecuencia muestreo: 3840 Hz (sincronizado con SerialPlotter)
 * - Baudrate: 38400 bps (10 bits x 3840 muestras/s)
 * - Latencia total: ~0.6-0.8 ms
 * - Resolución: 8 bits (ADC y DAC)
 */

// Librerías básicas del sistema
#include <avr/io.h>
#include <avr/interrupt.h>

// Incluir controladores del proyecto original (ahora en la misma carpeta)
#include "adc_original.h"       // Controlador ADC optimizado  
#include "timer1_original.h"    // Timer1 para interrupciones precisas
#include "usart.h"              // Comunicación serie optimizada

// ════════════════════════════════════════════════════════════════════════════════════════
// ⚙️ PARÁMETROS DEL SISTEMA
// ════════════════════════════════════════════════════════════════════════════════════════

// Configuración de muestreo y comunicación
const float FRECUENCIA_MUESTREO_HZ = 3840.0f;
const uint32_t BAUDRATE_SERIE = 38400UL;

// Configuración del DAC R2R
const uint8_t DAC_VALOR_MEDIO = 128;        // Punto medio del DAC (2.5V aprox)

// ════════════════════════════════════════════════════════════════════════════════════════
// 🎛️ INSTANCIAS DE CONTROLADORES (usando clases DSP originales)
// ════════════════════════════════════════════════════════════════════════════════════════

ADCController controlador_adc;                           // Control del ADC
Timer1 temporizador_muestreo(FRECUENCIA_MUESTREO_HZ);   // Timer para 3840 Hz
// USART se instancia automáticamente en usart.h como 'usart'

// ════════════════════════════════════════════════════════════════════════════════════════
// 🔄 VARIABLES DE ESTADO DEL SISTEMA
// ════════════════════════════════════════════════════════════════════════════════════════

// Variables globales para el sistema DSP (compatibles con DSP original)
volatile bool beat = false; // Flag de sincronización con timer (equivale a momento_procesar)
uint8_t valor = DAC_VALOR_MEDIO; // Valor actual a escribir al DAC (equivale a valor_salida_dac)

// Declaración externa de funciones write() para el DAC
void write(uint8_t value) {
    PORTA = value;  // Escribir directamente al puerto A (DAC R2R)
}

// ════════════════════════════════════════════════════════════════════════════════════════
// ⚡ RUTINAS DE INTERRUPCIÓN (compatible con DSP original)
// ════════════════════════════════════════════════════════════════════════════════════════

/**
 * Interrupción de Timer1 - Corazón del sistema DSP
 * Se ejecuta cada 260 microsegundos (3840 Hz)
 */
ISR(TIMER1_COMPA_vect) {
    // Escribir valor actual al DAC (igual que DSP original)
    write(valor);
    
    // Activar flag para procesamiento en loop principal (igual que DSP original)
    beat = true;
}

/**
 * Interrupción ADC - Conversión analógica completa
 */
ISR(ADC_vect) {
    controlador_adc.conversion_complete();
}

/**
 * Interrupción USART - Buffer de transmisión vacío
 */
ISR(USART0_UDRE_vect) {
    usart.udrie();
}

/**
 * Interrupción USART - Recepción completa
 */
ISR(USART0_RX_vect) {
    uint8_t dato_recibido = UDR0;
    if (usart.libre_lectura()) {
        usart.buffer_lectura[usart.fin_l] = dato_recibido;
        usart.fin_l = (usart.fin_l + 1) % sizeof(usart.buffer_lectura);
    }
}

// ════════════════════════════════════════════════════════════════════════════════════════
// 🏗️ CONFIGURACIÓN INICIAL DEL SISTEMA
// ════════════════════════════════════════════════════════════════════════════════════════

/**
 * Inicialización completa del sistema DSP
 */
void setup() {
    // Configurar cada subsistema por separado (más legible que el original)
    inicializar_comunicacion_serie();
    inicializar_controlador_adc();
    configurar_puerto_dac();
    inicializar_temporizador();
    configurar_led_estado();
    
    // Estado inicial del sistema
    valor = DAC_VALOR_MEDIO;  // DAC en punto medio (2.5V aprox)
    write(valor);             // Aplicar valor inicial
}

/**
 * Bucle principal - Sistema DSP bidireccional
 * 
 * FUNCIONAMIENTO:
 * 1. Timer1 ISR actualiza DAC cada 260μs (3840 Hz)
 * 2. Loop envía muestras ADC a PC para procesamiento  
 * 3. Loop recibe datos procesados de PC y los aplica al DAC
 * 4. Fallback: si no llegan datos de PC, usa ADC directo
 */
void loop() {
    // ¿Es momento de procesar? (activado por Timer1 ISR)
    if (beat) {
        beat = false;  // Limpiar flag inmediatamente
        
        // PASO 1: Enviar muestra ADC a la PC para análisis
        uint8_t muestra_adc_actual = controlador_adc.get();
        enviar_muestra_a_pc(muestra_adc_actual);
        
        // PASO 2: Recibir datos procesados desde la PC
        if (hay_datos_de_pc_disponibles()) {
            // Usar señal filtrada/procesada de la PC
            valor = recibir_dato_procesado_pc();
        } else {
            // Fallback: usar ADC directo si no hay datos de PC
            valor = muestra_adc_actual;
        }
        
        // El DAC se actualiza automáticamente en la próxima ISR de Timer1
    }
}

// ════════════════════════════════════════════════════════════════════════════════════════
// 🔧 FUNCIONES DE CONFIGURACIÓN
// ════════════════════════════════════════════════════════════════════════════════════════

/**
 * Configurar comunicación serie (equivale a usart.begin() del original)
 */
void inicializar_comunicacion_serie() {
    usart.begin(BAUDRATE_SERIE);
}

/**
 * Configurar controlador ADC (equivale a adc.begin() del original)
 */
void inicializar_controlador_adc() {
    controlador_adc.begin(1);  // Canal A1 (igual que original)
}

/**
 * Configurar puerto para DAC R2R (equivale a DDRA = 0xFF del original)
 */
void configurar_puerto_dac() {
    // Arduino Mega 2560: PORTA = pines 22-29 (PA0-PA7)
    // Configurar todos los pines como salida para DAC R2R de 8 bits
    DDRA = 0xFF;  // 11111111 binario = todos como salida
}

/**
 * Configurar Timer1 para muestreo (equivale a timer1.setup/start del original)
 */
void inicializar_temporizador() {
    temporizador_muestreo.setup();  // Configurar registros
    temporizador_muestreo.start();  // Iniciar interrupciones
}

/**
 * Configurar LED de estado (equivale a pinMode(13, OUTPUT) del original)
 */
void configurar_led_estado() {
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);  // LED apagado inicialmente
}

// ════════════════════════════════════════════════════════════════════════════════════════
// 📡 FUNCIONES DE COMUNICACIÓN
// ════════════════════════════════════════════════════════════════════════════════════════

/**
 * Enviar muestra ADC a la PC para procesamiento
 * (Equivale a usart.escribir() del original)
 */
void enviar_muestra_a_pc(uint8_t muestra) {
    usart.escribir(muestra);
}

/**
 * Verificar si hay datos disponibles desde la PC
 * (Equivale a usart.pendiente_lectura() del original)
 */
bool hay_datos_de_pc_disponibles() {
    return usart.pendiente_lectura();
}

/**
 * Recibir dato procesado desde la PC
 * (Equivale a usart.leer() del original)
 */
uint8_t recibir_dato_procesado_pc() {
    return usart.leer();
}

// ════════════════════════════════════════════════════════════════════════════════════════
// 📝 COMENTARIOS FINALES 
// ════════════════════════════════════════════════════════════════════════════════════════

/*
 * 🎯 RESUMEN DSP_INTERMEDIO:
 * 
 * ✅ FUNCIONALIDAD IDENTICAL AL DSP ORIGINAL:
 * • Mismas librerías (adc.h, timer1.h, usart.h) 
 * • Mismo flujo: ADC → PC → DAC
 * • Mismas variables globales (beat, valor)
 * • Misma ISR de Timer1
 * 
 * ✅ CÓDIGO MÁS LEGIBLE:
 * • Funciones organizadas y bien comentadas
 * • Nombres descriptivos para mayor claridad
 * • Estructura modular fácil de entender
 * • Documentación completa del funcionamiento
 * 
 * 🚀 RENDIMIENTO: Idéntico al original
 * 📚 LEGIBILIDAD: Significativamente mejorada
 * 🔧 MANTENIBILIDAD: Mucho más fácil de modificar
 */