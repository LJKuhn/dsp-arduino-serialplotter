/**
 * ============================================================================
 * IMPLEMENTACIONES ADC TUTORIAL - EXTREMADAMENTE COMENTADO
 * ============================================================================
 * 
 * Este archivo contiene todas las implementaciones de la clase ADCController
 * con comentarios educativos que explican cada línea de código.
 * 
 * ¿POR QUÉ EN UN ARCHIVO .cpp SEPARADO?
 * - ORGANIZACIÓN: Separa interfaz (declaraciones) de implementación
 * - COMPILACIÓN: Permite compilación incremental más rápida
 * - ESTÁNDAR: Sigue las mejores prácticas de C++
 * - REUTILIZACIÓN: Facilita el uso en otros proyectos
 */

#include "adc_tutorial.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// ============================================================================
// VARIABLE GLOBAL PARA MANEJO DE INTERRUPCIONES
// ============================================================================

// ¿POR QUÉ UNA VARIABLE GLOBAL?
// Las interrupciones en C son funciones globales que no pertenecen a ninguna clase
// Necesitamos una manera de conectar la interrupción con nuestra instancia de clase
ADCController* adc_instance = nullptr;

// ============================================================================
// IMPLEMENTACIÓN: begin() - Inicializar ADC
// ============================================================================

/**
 * CONFIGURACIÓN COMPLETA DEL ADC PARA OPERACIÓN CON INTERRUPCIONES
 * 
 * Esta función programa todos los registros del hardware para que el ADC:
 * 1. Use el pin especificado como entrada analógica
 * 2. Use AVcc (5V) como voltaje de referencia
 * 3. Configure prescaler para timing óptimo
 * 4. Habilite interrupciones cuando termine cada conversión
 */
void ADCController::begin(int pin) {
    // PASO 1: Guardar referencia global para uso en ISR
    adc_instance = this;
    
    // PASO 2: Configurar ADMUX (ADC Multiplexer Selection Register)
    //
    // ¿QUÉ HACE ESTE REGISTRO?
    // Controla QUÉ canal leer y CÓMO interpretar el resultado
    //
    // Configuración bit por bit:
    // Bit 7-6 (REFS1:0): Voltaje de referencia
    //   00 = AREF externo
    //   01 = AVcc (5V) ← LO QUE USAMOS
    //   10 = Reservado  
    //   11 = Voltaje interno 1.1V
    //
    // Bit 5 (ADLAR): Alineación del resultado
    //   0 = Justificado a derecha ← LO QUE USAMOS
    //   1 = Justificado a izquierda
    //
    // Bit 4-0 (MUX4:0): Selección de canal (0=A0, 1=A1, etc.)
    ADMUX = (1 << REFS0) | (pin & 0x0F);
    
    // PASO 3: Configurar ADCSRA (ADC Control and Status Register A)
    //
    // ¿QUÉ HACE ESTE REGISTRO?
    // Controla CÓMO opera el ADC: velocidad, interrupciones, estado
    //
    // Configuración bit por bit:
    // Bit 7 (ADEN): ADC Enable
    //   1 = Habilitar ADC ← NECESARIO
    //
    // Bit 6 (ADSC): ADC Start Conversion
    //   1 = Iniciar conversión (lo haremos después en start())
    //
    // Bit 5 (ADATE): ADC Auto Trigger Enable  
    //   0 = Modo manual ← LO QUE USAMOS
    //
    // Bit 4 (ADIF): ADC Interrupt Flag
    //   Se pone en 1 cuando termina conversión (hardware lo maneja)
    //
    // Bit 3 (ADIE): ADC Interrupt Enable
    //   1 = Habilitar interrupción ← NECESARIO PARA NUESTRO SISTEMA
    //
    // Bit 2-0 (ADPS2:0): ADC Prescaler Select
    //   111 = División por 128 ← PARA 125kHz desde 16MHz
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    
    // CÁLCULO DEL PRESCALER:
    // - Reloj del sistema: 16MHz
    // - Prescaler 128: 16MHz ÷ 128 = 125kHz
    // - Tiempo de conversión ADC: ~13 ciclos
    // - Tiempo total por conversión: 13 ÷ 125kHz = 104µs
    // - Frecuencia máxima de muestreo: ~9.6kHz (muy seguro para nuestros 3840Hz)
    
    // PASO 4: Inicializar variables de estado
    data = 0;           // Limpiar dato previo
    not_get = false;    // No hay datos nuevos aún
    
    // NOTA: El ADC está configurado pero aún no convierte
    // Llamar a start() para comenzar las conversiones
}

// ============================================================================
// IMPLEMENTACIÓN: get() - Obtener valor (modo bloqueante)
// ============================================================================

/**
 * LECTURA BLOQUEANTE CON ESPERA ACTIVA
 * 
 * ¿QUÉ SIGNIFICA "BLOQUEANTE"?
 * Esta función NO retorna hasta que hay un dato nuevo disponible
 * Si no hay datos frescos, se queda esperando en un loop infinito
 * 
 * ¿CUÁNDO ES ÚTIL?
 * Cuando necesitas garantía absoluta de datos frescos
 * Perfecto para sincronización con interrupciones de timer
 */
uint8_t ADCController::get() {
    // ESPERA ACTIVA: Loop hasta que haya datos nuevos
    while (!not_get) {
        // Este loop puede ejecutarse 0 veces (datos listos) 
        // o miles de veces (esperando interrupción)
        // CPU queda "bloqueada" aquí hasta que ISR active not_get
    }
    
    // MARCAR DATOS COMO CONSUMIDOS
    not_get = false;    // Resetear bandera para próxima lectura
    
    // CONVERSIÓN DE RESOLUCIÓN 10→8 BITS
    // El ADC produce valores 0-1023 (10 bits)
    // Necesitamos valores 0-255 (8 bits) para el DAC
    // Operación: dividir entre 4 = desplazar 2 bits a derecha
    return data >> 2;
}

// ============================================================================
// IMPLEMENTACIÓN: available() - Verificar datos disponibles
// ============================================================================

/**
 * VERIFICACIÓN NO BLOQUEANTE
 * 
 * ¿QUÉ SIGNIFICA "NO BLOQUEANTE"?
 * Esta función retorna INMEDIATAMENTE sin esperar
 * Solo informa el estado actual: hay/no hay datos
 * 
 * ¿CUÁNDO ES ÚTIL?
 * Cuando quieres verificar sin pausar el programa
 * Perfecto para loops que procesan múltiples tareas
 */
bool ADCController::available() {
    // Retorno directo del estado de la bandera
    // true = hay datos sin leer desde la última conversión
    // false = no hay datos nuevos (o ya se leyeron)
    return not_get;
}

// ============================================================================
// IMPLEMENTACIÓN: start() - Iniciar conversiones automáticas
// ============================================================================

/**
 * INICIAR LA PRIMERA CONVERSIÓN
 * 
 * ¿QUÉ HACE?
 * Dispara la primera conversión ADC escribiendo ADSC=1
 * A partir de ahí, las conversiones se encadenan automáticamente
 * 
 * ¿POR QUÉ SEPARADO DE begin()?
 * - begin() CONFIGURA todo el hardware
 * - start() INICIA el proceso de conversión
 * - Permite configurar completamente antes de empezar
 */
void ADCController::start() {
    // DISPARAR PRIMERA CONVERSIÓN
    // Al escribir ADSC=1, el hardware:
    // 1. Captura el voltaje del pin en sample&hold
    // 2. Inicia conversión analógica→digital (~104µs)  
    // 3. Cuando termina, ejecuta ISR(ADC_vect)
    // 4. En la ISR llamamos ADSC=1 otra vez para continuar
    ADCSRA |= (1 << ADSC);
}

// ============================================================================
// IMPLEMENTACIÓN: stop() - Detener conversiones
// ============================================================================

/**
 * DESHABILITAR ADC PARA AHORRAR ENERGÍA
 * 
 * ¿CUÁNDO USAR?
 * - Cuando no necesitas muestrear temporalmente
 * - Para ahorrar energía en aplicaciones portátiles  
 * - Durante reconfiguraciones del sistema
 */
void ADCController::stop() {
    // DESHABILITAR ADC COMPLETAMENTE
    // Esto para todas las conversiones y reduce consumo
    // Para reiniciar: llamar begin() nuevamente
    ADCSRA &= ~(1 << ADEN);
}

// ============================================================================
// IMPLEMENTACIÓN: ahora() - Conversión inmediata
// ============================================================================

/**
 * CONVERSIÓN SÍNCRONA SIN INTERRUPCIONES
 * 
 * ¿CUÁNDO USAR?
 * - Para mediciones puntuales de calibración
 * - Cuando no necesitas el sistema de interrupciones
 * - Para debug o caracterización de señales
 * 
 * ¡ADVERTENCIA!
 * NO usar en el loop principal durante operación normal
 * porque bloquea el sistema ~104µs por cada llamada
 */
uint8_t ADCController::ahora(int pin) {
    // PASO 1: Configurar canal (puede ser diferente al configurado en begin)
    ADMUX = (ADMUX & 0xF0) | (pin & 0x0F);
    
    // PASO 2: Disparar conversión inmediata
    ADCSRA |= (1 << ADSC);
    
    // PASO 3: Esperar activamente hasta que termine
    while (ADCSRA & (1 << ADSC)) {
        // ADSC se pone en 0 automáticamente cuando termina
        // Este loop dura ~104µs a 16MHz con prescaler 128
    }
    
    // PASO 4: Leer resultado directamente de registros
    // ¡CRÍTICO! Leer ADCL primero, luego ADCH
    // El hardware sincroniza automáticamente esta secuencia
    uint16_t result = ADCL;     // Byte bajo primero
    result |= (ADCH << 8);      // Byte alto después
    
    // PASO 5: Convertir a 8 bits y retornar
    return result >> 2;
}

// ============================================================================
// IMPLEMENTACIÓN: conversion_complete() - Llamada por interrupción
// ============================================================================

/**
 * MANEJADOR INTERNO DE INTERRUPCIÓN
 * 
 * ¿CUÁNDO SE EJECUTA?
 * Automáticamente cuando el hardware ADC completa una conversión
 * Llamada desde ISR(ADC_vect) en el archivo principal .ino
 * 
 * ¿QUÉ HACE?
 * 1. Lee el resultado de la conversión
 * 2. Marca datos como disponibles
 * 3. Dispara siguiente conversión para continuidad
 */
void ADCController::conversion_complete() {
    // PASO 1: Leer resultado de conversión
    // ¡ORDEN CRÍTICO! ADCL primero, ADCH segundo
    // El hardware AVR requiere esta secuencia para sincronización
    data = ADCL;                // Lee byte bajo y bloquea registros
    data |= (ADCH << 8);        // Lee byte alto y desbloquea
    
    // PASO 2: Señalizar datos disponibles
    not_get = true;             // Activar bandera para get() y available()
    
    // PASO 3: Encadenar siguiente conversión
    // Para mantener flujo continuo de muestras
    // Sin esto, solo obtendríamos una muestra y se detendría
    ADCSRA |= (1 << ADSC);
    
    // NOTA: Esta función debe ser MUY rápida (<10µs)
    // Se ejecuta 3840 veces por segundo en nuestro sistema
    // Cualquier demora aquí afecta el timing de todo el DSP
}

// ============================================================================
// EXPLICACIONES ADICIONALES PARA EL ESTUDIANTE
// ============================================================================

/**
 * ¿POR QUÉ ESTE DISEÑO DE CLASE?
 * 
 * VENTAJAS DE LA ORIENTACIÓN A OBJETOS AQUÍ:
 * 1. ENCAPSULACIÓN: Los registros del hardware están "ocultos"
 * 2. ABSTRACCIÓN: Interface simple (begin, get, start) vs registros complejos
 * 3. REUTILIZACIÓN: Se puede instanciar para múltiples canales ADC
 * 4. MANTENIMIENTO: Cambios en hardware solo afectan este archivo
 * 
 * PATRÓN DE DISEÑO USADO:
 * - SINGLETON-like: Una instancia global para interrupciones
 * - OBSERVER: La clase "observa" interrupciones del hardware
 * - FACADE: Interface simple sobre registros complejos
 * 
 * OPTIMIZACIONES APLICADAS:
 * - Variables uint16_t para valores ADC (eficiencia)
 * - Bit shifting en lugar de división (>> vs /)
 * - Inline en header para funciones pequeñas
 * - Acceso directo a registros vs digitalWrite()
 * 
 * TRADE-OFFS CONSIDERADOS:
 * - Precisión vs Velocidad: 8 bits suficiente para audio demo
 * - Memoria vs CPU: Variables uint16 vs cálculos repetidos  
 * - Simplicidad vs Features: API básica vs configuración avanzada
 * 
 * ¡Este código es un excelente ejemplo de programación de sistemas
 * embebidos con balance entre eficiencia y legibilidad!
 */