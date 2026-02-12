/**
 * ============================================================================
 * TUTORIAL COMPLETO: CONTROLADOR TIMER1 - EL CORAZÓN DEL TIMING
 * ============================================================================
 * 
 * Este archivo contiene una explicación educativa EXHAUSTIVA del Timer1,
 * el componente más crítico para el timing preciso del sistema DSP.
 * 
 * ¿QUÉ ES EL TIMER1?
 * Es un contador de hardware de 16 bits que cuenta pulsos de reloj del
 * microcontrolador. Cuando llega a un valor específico, puede generar
 * interrupciones para ejecutar código con timing perfecto.
 * 
 * ¿POR QUÉ ES CRÍTICO?
 * - Garantiza muestreo a frecuencia exacta (3840 Hz)
 * - Sincroniza ADC, comunicación serie y DAC
 * - Independiente de la carga de CPU del programa principal
 * - Precisión de microsegundos
 * 
 * ANALOGÍA:
 * Si el microcontrolador es un músico, el Timer1 es el metrónomo que
 * mantiene el ritmo perfecto, sin importar qué tan complicada sea la música.
 */

#pragma once
#include <stdint.h>

// ============================================================================
// CONCEPTOS FUNDAMENTALES DE TIMERS
// ============================================================================

/**
 * ARQUITECTURA DE UN TIMER DE 16 BITS:
 * 
 * ┌──────────────────────────────────────────────────────────┐
 * │                    PRESCALER                              │
 * │  16MHz ──→ ÷1 ──→ ÷8 ──→ ÷64 ──→ ÷256 ──→ ÷1024 ──→     │
 * └────────────────────────────────┬─────────────────────────┘
 *                                  │
 *                                  ▼
 * ┌──────────────────────────────────────────────────────────┐
 * │              CONTADOR DE 16 BITS                         │
 * │  0x0000 → 0x0001 → 0x0002 → ... → 0xFFFF → 0x0000      │
 * └────────────────────────────────┬─────────────────────────┘
 *                                  │
 *                                  ▼
 * ┌──────────────────────────────────────────────────────────┐
 * │              COMPARADORES                                │
 * │  Si TCNT1 == OCR1A → Interrupción TIMER1_COMPA_vect    │
 * │  Si TCNT1 == OCR1B → Interrupción TIMER1_COMPB_vect    │
 * └──────────────────────────────────────────────────────────┘
 * 
 * EXPLICACIÓN:
 * 1. El reloj principal (16MHz) es demasiado rápido
 * 2. El prescaler lo divide para obtener frecuencias manejables
 * 3. El contador cuenta estos pulsos más lentos
 * 4. Cuando el contador llega a un valor específico, ¡INTERRUPCIÓN!
 */

// ============================================================================
// CLASE TIMER1: INTERFAZ SIMPLIFICADA PARA CONFIGURACIÓN DE TIMING
// ============================================================================

/**
 * ¿POR QUÉ UNA CLASE PARA EL TIMER?
 * Los registros del Timer1 son complejos y fáciles de confundir:
 * - TCCR1A, TCCR1B, TCCR1C: Registros de control
 * - OCR1A, OCR1B: Registros de comparación
 * - TIMSK1: Registro de máscaras de interrupción
 * - ICR1: Registro de captura de entrada
 * 
 * Esta clase oculta esa complejidad y permite configurar con una sola línea.
 */
class Timer1 {
private:
    // ========================================================================
    // VARIABLES PRIVADAS PARA CÁLCULOS DE TIMING
    // ========================================================================
    
    float frecuencia_deseada;   // Frecuencia objetivo en Hz (ej: 3840.0)
    uint16_t prescaler;         // Factor de división del reloj (1, 8, 64, 256, 1024)
    uint16_t valor_comparacion; // Valor para OCR1A que genera la frecuencia deseada

public:
    // ========================================================================
    // CONSTRUCTOR: CALCULA PARÁMETROS DE TIMING
    // ========================================================================
    
    /**
     * CONSTRUCTOR CON CÁLCULO AUTOMÁTICO DE PARÁMETROS
     * 
     * ¿QUÉ HACE EL CONSTRUCTOR?
     * Recibe la frecuencia deseada y calcula automáticamente:
     * 1. Qué prescaler usar (1, 8, 64, 256, o 1024)
     * 2. Qué valor poner en OCR1A para obtener esa frecuencia
     * 
     * FÓRMULA FUNDAMENTAL:
     * frecuencia = F_CPU / (prescaler * (OCR1A + 1))
     * 
     * Despejando OCR1A:
     * OCR1A = (F_CPU / (prescaler * frecuencia)) - 1
     * 
     * EJEMPLO PARA 3840 Hz:
     * - Si prescaler = 1: OCR1A = (16000000 / (1 * 3840)) - 1 = 4165
     * - Si prescaler = 8: OCR1A = (16000000 / (8 * 3840)) - 1 = 520
     * 
     * ¿POR QUÉ ELEGIR PRESCALER = 8?
     * - Con prescaler = 1: OCR1A = 4165 (valor alto, menos precisión)
     * - Con prescaler = 8: OCR1A = 520 (valor medio, buena precisión)
     * - Con prescaler = 64: OCR1A = 65 (valor bajo, puede ser inestable)
     * 
     * REGLA PRÁCTICA: Buscar OCR1A entre 100 y 10000 para mejor precisión
     */
    Timer1(float frecuencia) {
        frecuencia_deseada = frecuencia;
        
        // ALGORITMO DE SELECCIÓN DE PRESCALER:
        // Probamos cada prescaler hasta encontrar uno que dé un OCR1A razonable
        
        const uint16_t prescalers[] = {1, 8, 64, 256, 1024};
        const uint16_t F_CPU = 16000000;  // 16 MHz
        
        for (int i = 0; i < 5; i++) {
            prescaler = prescalers[i];
            
            // Calcular OCR1A teórico para este prescaler
            float ocr1a_float = (F_CPU / (prescaler * frecuencia)) - 1;
            valor_comparacion = (uint16_t)ocr1a_float;
            
            // ¿Es un valor razonable? (entre 100 y 65000)
            if (valor_comparacion >= 100 && valor_comparacion <= 65000) {
                break;  // ¡Perfecto! Usar este prescaler
            }
        }
        
        // VERIFICACIÓN: Calcular frecuencia real conseguida
        float frecuencia_real = (float)F_CPU / (prescaler * (valor_comparacion + 1));
        float error_porcentual = abs((frecuencia_real - frecuencia) / frecuencia) * 100.0;
        
        // En un sistema real, aquí imprimiríamos los resultados:
        // Serial.print("Frecuencia deseada: "); Serial.println(frecuencia);
        // Serial.print("Prescaler elegido: "); Serial.println(prescaler);
        // Serial.print("OCR1A calculado: "); Serial.println(valor_comparacion);
        // Serial.print("Frecuencia real: "); Serial.println(frecuencia_real);
        // Serial.print("Error: "); Serial.print(error_porcentual); Serial.println("%");
    }

    // ========================================================================
    // CONFIGURAR REGISTROS DEL TIMER1
    // ========================================================================
    
    /**
     * CONFIGURACIÓN COMPLETA DEL TIMER1 EN MODO CTC
     * 
     * ¿QUÉ ES EL MODO CTC?
     * CTC = "Clear Timer on Compare Match"
     * Significa: "Cuando el contador llegue a OCR1A, reiniciarlo a 0"
     * 
     * VENTAJAS DEL MODO CTC:
     * - Frecuencia exacta y predecible
     * - El contador nunca desborda (no llega a 65535)
     * - Fácil de calcular y configurar
     * - Ideal para generación de frecuencias precisas
     * 
     * REGISTROS INVOLUCRADOS:
     */
    void setup() {
        // ====================================================================
        // PASO 1: LIMPIAR CONFIGURACIÓN PREVIA
        // ====================================================================
        
        // ¿POR QUÉ LIMPIAR PRIMERO?
        // Arduino puede haber usado el Timer1 para otras funciones
        // Necesitamos empezar desde un estado conocido
        
        TCCR1A = 0;  // Timer/Counter Control Register 1A = 0
                     // Esto deshabilita todas las salidas PWM
                     // Bits relevantes:
                     // - COM1A1:0 = 00 (desconectar pin OC1A)
                     // - COM1B1:0 = 00 (desconectar pin OC1B)
                     // - WGM11:10 = 00 (parte del modo de operación)
        
        TCCR1B = 0;  // Timer/Counter Control Register 1B = 0
                     // Esto detiene el timer (sin fuente de reloj)
                     // Bits relevantes:
                     // - ICNC1 = 0 (sin cancelación de ruido)
                     // - ICES1 = 0 (flanco de bajada)
                     // - WGM13:12 = 00 (parte del modo de operación)
                     // - CS12:10 = 000 (sin fuente de reloj = timer parado)
        
        TCNT1 = 0;   // Timer/Counter 1 = 0
                     // Resetear el contador a 0
                     // Empezar la cuenta desde cero
        
        // ====================================================================
        // PASO 2: CONFIGURAR MODO CTC
        // ====================================================================
        
        // ¿CÓMO SE CONFIGURA EL MODO CTC?
        // El modo se controla con 4 bits: WGM13, WGM12, WGM11, WGM10
        // Para modo CTC con TOP = OCR1A: WGM13:10 = 0100
        // 
        // DISTRIBUCIÓN DE BITS:
        // - WGM11:10 están en TCCR1A bits 1:0
        // - WGM13:12 están en TCCR1B bits 4:3
        
        TCCR1B |= (1 << WGM12);  // Poner WGM12 = 1, los demás quedan en 0
                                // WGM13:10 = 0100 = Modo CTC con TOP = OCR1A
        
        // ====================================================================
        // PASO 3: CONFIGURAR VALOR DE COMPARACIÓN
        // ====================================================================
        
        OCR1A = valor_comparacion;  // Output Compare Register 1A
                                   // Cuando TCNT1 llegue a este valor:
                                   // 1. Se genera interrupción TIMER1_COMPA_vect
                                   // 2. TCNT1 se resetea automáticamente a 0
                                   // 3. Empieza a contar desde 0 otra vez
        
        // ====================================================================
        // PASO 4: HABILITAR INTERRUPCIÓN
        // ====================================================================
        
        // ¿QUÉ ES TIMSK1?
        // Timer/Counter Interrupt Mask Register 1
        // Controla qué interrupciones del Timer1 están habilitadas
        
        TIMSK1 |= (1 << OCIE1A);  // Output Compare Match A Interrupt Enable
                                 // Habilita interrupción cuando TCNT1 = OCR1A
                                 // La función ISR(TIMER1_COMPA_vect) se ejecutará
        
        // NOTA: El prescaler NO se configura aquí
        // Se configura en start() para mantener el timer detenido por ahora
    }

    // ========================================================================
    // INICIAR EL TIMER (APLICAR PRESCALER)
    // ========================================================================
    
    /**
     * INICIAR EL CONTEO CON EL PRESCALER CALCULADO
     * 
     * ¿POR QUÉ start() SEPARADO DE setup()?
     * - setup() configura TODO excepto el prescaler
     * - start() aplica el prescaler para iniciar el conteo
     * - Esto permite configurar todo y luego iniciar en el momento exacto
     * - Útil para sincronización con otros componentes
     */
    void start() {
        // CONFIGURACIÓN DEL PRESCALER EN TCCR1B:
        // Los bits CS12:10 controlan la fuente de reloj del timer
        // 
        // TABLA DE PRESCALERS:
        // CS12 CS11 CS10 | Prescaler | Descripción
        // 0    0    0    |    OFF    | Timer detenido
        // 0    0    1    |     1     | clk/1 (sin prescaler)
        // 0    1    0    |     8     | clk/8
        // 0    1    1    |    64     | clk/64
        // 1    0    0    |   256     | clk/256
        // 1    0    1    |  1024     | clk/1024
        // 1    1    0    |   EXTL    | Reloj externo en pin T1 (flanco bajada)
        // 1    1    1    |   EXTH    | Reloj externo en pin T1 (flanco subida)
        
        // Limpiar bits de prescaler
        TCCR1B &= ~((1<<CS12) | (1<<CS11) | (1<<CS10));
        
        // Aplicar prescaler según el valor calculado en el constructor
        if (prescaler == 1) {
            TCCR1B |= (1 << CS10);           // 001 = clk/1
        } else if (prescaler == 8) {
            TCCR1B |= (1 << CS11);           // 010 = clk/8
        } else if (prescaler == 64) {
            TCCR1B |= (1 << CS11) | (1 << CS10);  // 011 = clk/64
        } else if (prescaler == 256) {
            TCCR1B |= (1 << CS12);           // 100 = clk/256
        } else if (prescaler == 1024) {
            TCCR1B |= (1 << CS12) | (1 << CS10); // 101 = clk/1024
        }
        
        // ¡MOMENTO CRÍTICO!
        // En el momento que se escriben estos bits, el timer EMPIEZA A CONTAR
        // La primera interrupción ocurrirá después de (OCR1A + 1) pulsos
        // de reloj dividido por el prescaler
    }

    // ========================================================================
    // DETENER EL TIMER
    // ========================================================================
    
    /**
     * DETENER EL CONTEO DEL TIMER
     * 
     * ¿CUÁNDO USAR?
     * - Para pausar el sistema temporalmente
     * - Durante reconfiguraciones
     * - Para ahorrar energía
     * - En situaciones de emergencia
     */
    void stop() {
        // Limpiar bits de prescaler = desconectar fuente de reloj
        TCCR1B &= ~((1<<CS12) | (1<<CS11) | (1<<CS10));
        
        // El timer se detiene inmediatamente
        // TCNT1 mantiene su valor actual (no se resetea)
        // Las interrupciones dejan de generarse
    }
    
    // ========================================================================
    // OBTENER INFORMACIÓN DE CONFIGURACIÓN
    // ========================================================================
    
    /**
     * FUNCIONES PARA DEPURACIÓN Y MONITOREO
     * Útiles para verificar que el timer está configurado correctamente
     */
    
    float getFrecuencia() { return frecuencia_deseada; }
    uint16_t getPrescaler() { return prescaler; }
    uint16_t getOCR1A() { return valor_comparacion; }
    
    // Calcular frecuencia real conseguida
    float getFrecuenciaReal() {
        return 16000000.0 / (prescaler * (valor_comparacion + 1));
    }
    
    // Calcular error porcentual
    float getError() {
        float real = getFrecuenciaReal();
        return abs((real - frecuencia_deseada) / frecuencia_deseada) * 100.0;
    }
};

// ============================================================================
// EXPLICACIÓN DETALLADA DEL TIMING DEL SISTEMA
// ============================================================================

/**
 * CRONOLOGÍA DE UNA INTERRUPCIÓN DE TIMER (EJEMPLO 3840 Hz):
 * 
 * 1. CONFIGURACIÓN:
 *    - Prescaler = 8
 *    - OCR1A = 520
 *    - Frecuencia de reloj del timer = 16MHz / 8 = 2MHz
 *    - Período de reloj del timer = 1/2MHz = 0.5 µs
 * 
 * 2. CONTEO:
 *    T=0.0 µs:     TCNT1 = 0, empezar a contar
 *    T=0.5 µs:     TCNT1 = 1
 *    T=1.0 µs:     TCNT1 = 2
 *    ...
 *    T=260.0 µs:   TCNT1 = 520
 *    T=260.0 µs:   ¡COMPARACIÓN! TCNT1 == OCR1A
 *    T=260.1 µs:   TCNT1 = 0 (reset automático)
 *    T=260.1 µs:   Ejecutar ISR(TIMER1_COMPA_vect)
 *    T=260.2 µs:   ISR termina, continuar programa principal
 *    T=260.5 µs:   TCNT1 = 1, ¡empezar nuevo ciclo!
 * 
 * 3. VERIFICACIÓN:
 *    - Tiempo total del ciclo = 520 * 0.5 µs = 260 µs
 *    - Frecuencia real = 1 / 260 µs = 3846.15 Hz
 *    - Error = |3846.15 - 3840| / 3840 = 0.16% ¡EXCELENTE!
 * 
 * LATENCIA DE INTERRUPCIÓN:
 * - Tiempo desde comparación hasta inicio de ISR ≈ 4-5 µs
 * - Tiempo de ejecución de ISR ≈ 1-2 µs
 * - Tiempo total de overhead ≈ 6-7 µs
 * - Porcentaje de CPU usado = 7µs / 260µs = 2.7%
 * - ¡Queda 97.3% de CPU para el programa principal!
 * 
 * ESTABILIDAD A LARGO PLAZO:
 * - El cristal de Arduino tiene precisión ±50 ppm típica
 * - Error máximo = 50/1000000 * 3840 Hz = ±0.19 Hz
 * - Drift típico por temperatura ≈ ±20 ppm/°C
 * - Para DSP de audio, esta precisión es MÁS que suficiente
 * 
 * JITTER (VARIACIÓN DE TIMING):
 * - Timer de hardware: jitter < 1 µs
 * - Sistema basado en delay(): jitter 10-100 µs
 * - ¡El timer de hardware es 100 veces más preciso!
 */