/**
 * ============================================================================
 * TUTORIAL COMPLETO: CONTROLADOR ADC (Analog-to-Digital Converter)
 * ============================================================================
 * 
 * Este archivo contiene una versión educativa EXTREMADAMENTE comentada del
 * controlador para el ADC del Arduino Mega 2560.
 * 
 * ¿QUÉ ES EL ADC?
 * El ADC (Convertidor Analógico-Digital) es un circuito que convierte
 * voltajes del mundo real (señales analógicas) en números digitales que
 * la computadora puede procesar.
 * 
 * CONCEPTOS FUNDAMENTALES:
 * - RESOLUCIÓN: Arduino Mega tiene ADC de 10 bits (0-1023), pero usamos 8 bits (0-255)
 * - VOLTAJE DE REFERENCIA: 5V (0V=0, 5V=255 en nuestro caso)
 * - FRECUENCIA DE MUESTREO: Controlada por interrupciones del Timer1
 * - CANALES: Arduino Mega tiene 16 canales ADC (A0-A15), usamos A1
 */

#pragma once        // ¿QUÉ HACE #pragma once?
                   // Evita que este archivo se incluya múltiples veces
                   // Es como decir "si ya me incluiste, no lo hagas otra vez"

#include <stdint.h>  // Biblioteca estándar para tipos de datos como uint8_t, uint16_t

// ============================================================================
// DECLARACIÓN DE FUNCIÓN EXTERNA DE INTERRUPCIÓN
// ============================================================================

// ¿QUÉ ES extern "C"?
// Le dice al compilador C++ "esta función está escrita en C, no en C++"
// Las interrupciones de AVR usan convenciones de C puro
extern "C" void ADC_vect (void);

// ============================================================================
// CLASE ADCController: INTERFAZ ORIENTADA A OBJETOS PARA EL ADC
// ============================================================================

/**
 * ¿POR QUÉ UNA CLASE?
 * - ENCAPSULACIÓN: Agrupa datos y funciones relacionadas
 * - ABSTRACCIÓN: Oculta la complejidad de los registros del hardware
 * - REUTILIZACIÓN: Se puede usar fácilmente en otros proyectos
 * - MANTENIMIENTO: Es más fácil de entender y modificar
 */
class ADCController {
private:
    // ========================================================================
    // VARIABLES PRIVADAS (SOLO LA CLASE PUEDE ACCEDER A ELLAS)
    // ========================================================================
    
    // ¿QUÉ ES uint16_t?
    // Un entero sin signo de 16 bits (0 a 65535)
    // Perfecto para almacenar valores del ADC de 10 bits (0 a 1023)
    uint16_t data = -1;         // Último valor leído del ADC
                               // -1 indica "no hay datos válidos aún"
    
    // ¿QUÉ ES bool?
    // Un tipo de dato que solo puede ser true (verdadero) o false (falso)
    // Ocupa 1 byte en memoria
    bool not_get = false;       // Bandera: ¿hay datos nuevos disponibles?
                               // true = hay datos sin leer
                               // false = no hay datos nuevos

    // ========================================================================
    // FUNCIÓN PRIVADA LLAMADA POR LA INTERRUPCIÓN
    // ========================================================================
    
    /**
     * ¿POR QUÉ ES PRIVADA?
     * Solo debe ser llamada por la interrupción del ADC, no por el usuario
     * Mantiene la integridad de los datos y evita uso incorrecto
     */
    void conversion_complete();

    // ========================================================================
    // DECLARACIÓN DE AMISTAD CON LA FUNCIÓN DE INTERRUPCIÓN
    // ========================================================================
    
    // ¿QUÉ SIGNIFICA friend?
    // Le da a la función ADC_vect() acceso a las partes privadas de esta clase
    // Es necesario porque la interrupción necesita llamar conversion_complete()
    friend void ADC_vect();

public:
    // ========================================================================
    // MÉTODOS PÚBLICOS (EL USUARIO PUEDE LLAMAR A ESTAS FUNCIONES)
    // ========================================================================

    /**
     * INICIALIZAR EL ADC EN UN PIN ESPECÍFICO
     * 
     * ¿QUÉ HACE ESTA FUNCIÓN?
     * Configura todos los registros del hardware para que el ADC:
     * 1. Use el pin especificado como entrada
     * 2. Use 5V como voltaje de referencia
     * 3. Active interrupciones cuando termine una conversión
     * 4. Configure la velocidad de conversión apropiada
     * 
     * @param pin: Número del pin analógico (0=A0, 1=A1, etc.)
     */
    void begin(int pin);

    /**
     * OBTENER EL ÚLTIMO VALOR CONVERTIDO (MODO BLOQUEANTE)
     * 
     * ¿QUÉ SIGNIFICA "BLOQUEANTE"?
     * Esta función espera hasta que hay un nuevo valor disponible
     * Si no hay datos nuevos, se queda "bloqueada" hasta que lleguen
     * 
     * ¿CUÁNDO USAR?
     * Cuando necesitas garantía de que el valor es fresco/nuevo
     * 
     * @return: Valor del ADC escalado a 8 bits (0-255)
     */
    uint8_t get();

    /**
     * VERIFICAR SI HAY DATOS NUEVOS DISPONIBLES (MODO NO BLOQUEANTE)
     * 
     * ¿QUÉ SIGNIFICA "NO BLOQUEANTE"?
     * Esta función retorna inmediatamente, sin esperar
     * true = hay datos nuevos, false = no hay datos nuevos
     * 
     * ¿CUÁNDO USAR?
     * Cuando quieres verificar sin pausar el programa
     * Útil en loops que hacen múltiples tareas
     * 
     * @return: true si hay datos disponibles, false en caso contrario
     */
    bool available();

    /**
     * INICIAR CONVERSIONES AUTOMÁTICAS
     * 
     * ¿QUÉ HACE?
     * Habilita el ADC para que empiece a convertir automáticamente
     * Las conversiones se dispararán por interrupciones del Timer
     * 
     * ¿CUÁNDO USAR?
     * Después de begin(), cuando estés listo para empezar a muestrear
     */
    void start();

    /**
     * DETENER CONVERSIONES AUTOMÁTICAS
     * 
     * ¿QUÉ HACE?
     * Deshabilita el ADC para ahorrar energía
     * Útil cuando no necesitas muestrear temporalmente
     * 
     * ¿CUÁNDO USAR?
     * Cuando quieres pausar el sistema o ahorrar energía
     */
    void stop();

    /**
     * CONVERSIÓN INMEDIATA (MODO SÍNCRONO)
     * 
     * ¿QUÉ HACE?
     * Dispara una conversión inmediata y espera el resultado
     * No usa interrupciones, es completamente bloqueante
     * 
     * ¿CUÁNDO USAR?
     * Para mediciones puntuales o calibración
     * NO usar en el loop principal durante operación normal
     * 
     * @param pin: Pin analógico a leer
     * @return: Valor inmediato del ADC (0-255)
     */
    uint8_t ahora(int pin);
};

// ============================================================================
// EXPLICACIÓN DETALLADA DEL HARDWARE ADC
// ============================================================================

/**
 * ARQUITECTURA INTERNA DEL ADC:
 * 
 * 1. MULTIPLEXOR (MUX):
 *    - Selecciona cuál de los 16 canales (A0-A15) se va a convertir
 *    - Como un interruptor rotativo de 16 posiciones
 *    - Controlado por los bits MUX3:0 en el registro ADMUX
 * 
 * 2. SAMPLE & HOLD:
 *    - Captura el voltaje en un condensador
 *    - "Congela" el valor durante la conversión
 *    - Evita errores si la señal cambia durante la conversión
 * 
 * 3. CONVERTIDOR SUCESIVO:
 *    - Usa un algoritmo de "búsqueda binaria" para encontrar el valor
 *    - Compara contra voltajes de referencia: 5V, 2.5V, 1.25V, etc.
 *    - Genera 10 bits de resultado (0-1023)
 * 
 * 4. REGISTRO DE RESULTADO:
 *    - ADCL (8 bits bajos) y ADCH (8 bits altos)
 *    - IMPORTANTE: Leer ADCL primero, luego ADCH
 *    - El hardware se sincroniza automáticamente
 * 
 * CONVERSIÓN DE 10 BITS A 8 BITS:
 * - El ADC produce valores 0-1023 (10 bits)
 * - Nosotros necesitamos 0-255 (8 bits) para el DAC
 * - Fórmula: valor_8bit = valor_10bit / 4
 * - O más eficiente: valor_8bit = valor_10bit >> 2
 * 
 * VOLTAJES Y RESOLUCIÓN:
 * - 10 bits: 1024 niveles, resolución = 5V/1024 = 4.88 mV por paso
 * - 8 bits: 256 niveles, resolución = 5V/256 = 19.53 mV por paso
 * - Para señales de audio, 8 bits es suficiente para demostraciones
 * 
 * FRECUENCIA DE CONVERSIÓN:
 * - El ADC necesita tiempo para cada conversión (~13 ciclos de reloj ADC)
 * - Con prescaler 128: f_adc = 16MHz/128 = 125kHz
 * - Tiempo de conversión = 13/125kHz = 104 µs
 * - Frecuencia máxima de muestreo ≈ 9600 Hz
 * - Nosotros usamos 3840 Hz, muy seguro y estable
 * 
 * SINCRONIZACIÓN CON TIMER1:
 * - Timer1 dispara conversiones cada 1/3840 segundos = 260 µs
 * - ADC completa conversión en ~104 µs
 * - Sobran 156 µs para procesamiento = sistema muy estable
 * - La interrupción ADC_vect se ejecuta cuando termina la conversión
 */