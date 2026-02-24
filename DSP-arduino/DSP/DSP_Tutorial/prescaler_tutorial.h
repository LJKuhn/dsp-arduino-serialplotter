/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                         ⚙️ PRESCALER TUTORIAL COMENTADO ⚙️                         ║
 * ║                     Tutorial Educativo de División de Frecuencias                  ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 📚 OBJETIVO EDUCATIVO:
 * Este archivo enseña los conceptos fundamentales de los prescalers en
 * microcontroladores, explicando cómo dividir frecuencias de reloj para
 * conseguir timings precisos y eficientes.
 * 
 * 🎯 QUÉ APRENDERÁS:
 * ✅ ¿Qué es un prescaler y por qué es necesario?
 * ✅ ¿Cómo dividir frecuencias de forma eficiente?
 * ✅ ¿Cuáles son los valores estándar de prescaler en AVR?
 * ✅ ¿Cómo calcular frecuencias resultantes?
 * ✅ ¿Cuándo usar cada valor de prescaler?
 * ✅ Trade-offs: resolución vs rango de timing
 * 
 * 💡 NIVEL: Principiante total → Experto en timing
 */

#ifndef PRESCALER_TUTORIAL_H
#define PRESCALER_TUTORIAL_H

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                        🕰️ ¿QUÉ ES UN PRESCALER? 🕰️                                ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 🔄 DEFINICIÓN SIMPLE:
 * Un prescaler es un DIVISOR DE FRECUENCIA que toma una señal de reloj
 * rápida y la convierte en una señal de reloj más lenta.
 * 
 * 🎯 ¿POR QUÉ ES NECESARIO?
 * 
 * 📊 PROBLEMA SIN PRESCALER:
 * • Arduino Mega 2560 funciona a 16 MHz = 16,000,000 Hz
 * • Timer de 16 bits cuenta hasta 65,535 (0xFFFF)
 * • Máximo período = 65,535 / 16,000,000 = 4.1 milisegundos
 * • ¡Solo puedes medir tiempos MUY cortos!
 * 
 * ✅ SOLUCIÓN CON PRESCALER:
 * • Prescaler ÷256: Frecuencia efectiva = 16MHz ÷ 256 = 62,500 Hz
 * • Máximo período = 65,535 / 62,500 = 1.05 segundos
 * • ¡Ahora puedes medir tiempos útiles!
 * 
 * 🏗️ ANALOGÍA FÍSICA:
 * Imagina una rueda grande (prescaler) conectada a una rueda pequeña (CPU):
 * • Rueda pequeña gira muy rápido (16 MHz)
 * • Rueda grande gira lento pero con más fuerza (timing largo)
 * • El prescaler es la relación de tamaños entre las ruedas
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                       ⚡ FRECUENCIAS Y PRESCALERS EN AVR ⚡                         ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 🎛️ PRESCALERS DISPONIBLES EN ATMEGA2560:
 * 
 * Los timers AVR tienen estos valores de prescaler estándar:
 * • 1 (sin división)
 * • 8  
 * • 64
 * • 256
 * • 1024
 * 
 * 📊 TABLA DE FRECUENCIAS RESULTANTES (F_CPU = 16 MHz):
 * 
 * Prescaler | Freq. Timer | Período Timer | Tick mínimo | Máximo período
 * ----------|-------------|---------------|-------------|----------------
 *    1      | 16,000,000  |    62.5 ns   |   62.5 ns   |    4.1 ms
 *    8      |  2,000,000  |   500   ns   |   500   ns   |   32.8 ms  
 *   64      |    250,000  |     4   μs   |     4   μs   |  262.1 ms
 *  256      |     62,500  |    16   μs   |    16   μs   |    1.05 s
 * 1024      |     15,625  |    64   μs   |    64   μs   |    4.19 s
 * 
 * 🤔 ¿CÓMO LEER ESTA TABLA?
 * • Freq. Timer: A qué velocidad cuenta el timer
 * • Período Timer: Tiempo entre cada tick del timer
 * • Tick mínimo: La menor resolución de tiempo medible
 * • Máximo período: El tiempo más largo medible (con timer de 16 bits)
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                          🧮 CÁLCULOS MATEMÁTICOS DETALLADOS 🧮                     ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 📐 FÓRMULAS FUNDAMENTALES:
 * 
 * 1️⃣ Frecuencia del Timer:
 *    f_timer = F_CPU / prescaler
 * 
 * 2️⃣ Período del Timer (tiempo por tick):
 *    T_timer = 1 / f_timer = prescaler / F_CPU
 * 
 * 3️⃣ Tiempo total para N ticks:
 *    Tiempo = N × T_timer = N × prescaler / F_CPU
 * 
 * 4️⃣ Número de ticks para un tiempo deseado:
 *    N = Tiempo_deseado × F_CPU / prescaler
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                        💡 EJEMPLO PRÁCTICO: NUESTRO PROYECTO 💡                    ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 🎯 OBJETIVO: Generar interrupciones a 3840 Hz (cada 260.42 μs)
 * 
 * 🧮 PROCESO DE CÁLCULO:
 * 
 * 1️⃣ Período deseado:
 *    T_deseado = 1 / 3840 = 260.42 μs
 * 
 * 2️⃣ Evaluar cada prescaler:
 * 
 *    📊 PRESCALER = 1:
 *    N = 260.42μs × 16,000,000 / 1 = 4166.67 ticks
 *    ❌ Problema: No cabe en 16 bits (máx 65535) ← ¡Falso! Sí cabe
 *    ✅ Actually: Sí funciona, pero usa prescaler bajo
 * 
 *    📊 PRESCALER = 8:  
 *    N = 260.42μs × 16,000,000 / 8 = 520.83 ticks
 *    ✅ Cabe perfectamente en 16 bits
 *    ✅ Buena resolución (0.5 μs por tick)
 * 
 *    📊 PRESCALER = 64:
 *    N = 260.42μs × 16,000,000 / 64 = 65.10 ticks  
 *    ✅ Cabe pero con menos resolución
 *    ❌ Error de cuantización más grande
 * 
 * 3️⃣ Elección optimal:
 *    🏆 PRESCALER = 8 con N = 521 ticks
 * 
 * 4️⃣ Verificación:
 *    T_real = 521 × 8 / 16,000,000 = 260.5 μs
 *    f_real = 1 / 260.5μs = 3839.85 Hz
 *    Error = |3839.85 - 3840| / 3840 = 0.004% ← ¡Excelente!
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                          🎛️ DEFINICIONES DE PRESCALER 🎛️                          ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * Estas definiciones mapean valores de prescaler a los bits de configuración
 * específicos del hardware AVR.
 */

// 📋 Valores de prescaler disponibles (abstractos)
#define PRESCALER_1     1
#define PRESCALER_8     8      // ← El que usamos en nuestro proyecto
#define PRESCALER_64    64
#define PRESCALER_256   256  
#define PRESCALER_1024  1024

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                    ⚙️ CONFIGURACIÓN DE REGISTROS DE TIMER ⚙️                      ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * El hardware AVR usa 3 bits en el registro TCCR1B para configurar el prescaler.
 * Estos bits se llaman CS12, CS11, CS10 (Clock Select).
 */

// 📊 Mapeo de prescaler a bits de configuración para Timer1
#define TIMER1_NO_PRESCALER     ((1 << CS10))                    // CS12=0, CS11=0, CS10=1
#define TIMER1_PRESCALER_8      ((1 << CS11))                    // CS12=0, CS11=1, CS10=0  
#define TIMER1_PRESCALER_64     ((1 << CS11) | (1 << CS10))     // CS12=0, CS11=1, CS10=1
#define TIMER1_PRESCALER_256    ((1 << CS12))                    // CS12=1, CS11=0, CS10=0
#define TIMER1_PRESCALER_1024   ((1 << CS12) | (1 << CS10))     // CS12=1, CS11=0, CS10=1

/*
 * 🔍 EXPLICACIÓN DETALLADA DEL MAPEO:
 * 
 * 📋 TABLA COMPLETA DE CONFIGURACIONES:
 * 
 * CS12 | CS11 | CS10 | Descripción
 * -----|------|------|----------------------------------
 *   0  |   0  |   0  | Timer detenido
 *   0  |   0  |   1  | Sin prescaler (clk/1)
 *   0  |   1  |   0  | clk/8
 *   0  |   1  |   1  | clk/64  
 *   1  |   0  |   0  | clk/256
 *   1  |   0  |   1  | clk/1024
 *   1  |   1  |   0  | Clock externo pin T1 (falling edge)
 *   1  |   1  |   1  | Clock externo pin T1 (rising edge)
 * 
 * 💡 En nuestro proyecto usamos: CS12=0, CS11=1, CS10=0 → clk/8
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                        🧪 CLASE CALCULADORA DE PRESCALER 🧪                        ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * Esta clase ayuda a calcular el prescaler optimal para cualquier frecuencia
 * deseada, considerando la precisión y limitaciones del hardware.
 */

class CalculadoraPrescaler {
public:
    // 🎯 Estructura para guardar resultado de cálculo
    struct ResultadoPrescaler {
        uint16_t prescaler;           // Valor de prescaler a usar
        uint16_t ticks;              // Número de ticks del timer
        float frecuencia_real;        // Frecuencia real conseguida
        float error_porcentual;       // Error respecto a la deseada
        uint8_t config_registros;     // Bits para TCCR1B
        bool es_valido;              // ¿Es un resultado válido?
    };

    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                    🧮 CALCULAR PRESCALER ÓPTIMO 🧮                             ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     */
    static ResultadoPrescaler calcular_optimo(float frecuencia_deseada) {
        /*
         * 🎯 ALGORITMO:
         * 1. Probar cada prescaler disponible
         * 2. Calcular ticks necesarios para cada uno
         * 3. Verificar que cabe en 16 bits (≤65535)
         * 4. Calcular error de precisión  
         * 5. Elegir el que tenga menor error
         */
        
        ResultadoPrescaler mejor_resultado;
        mejor_resultado.es_valido = false;
        mejor_resultado.error_porcentual = 100.0;  // Empezar con error máximo
        
        // 📋 Lista de prescalers a probar
        uint16_t prescalers[] = {1, 8, 64, 256, 1024};
        uint8_t configs[] = {
            TIMER1_NO_PRESCALER,
            TIMER1_PRESCALER_8, 
            TIMER1_PRESCALER_64,
            TIMER1_PRESCALER_256,
            TIMER1_PRESCALER_1024
        };
        
        for (uint8_t i = 0; i < 5; i++) {
            uint16_t prescaler = prescalers[i];
            
            // 🧮 Calcular ticks necesarios
            float ticks_float = F_CPU / (frecuencia_deseada * prescaler);
            uint16_t ticks = (uint16_t)(ticks_float + 0.5);  // Redondear
            
            // 🚨 Verificar que cabe en 16 bits
            if (ticks == 0 || ticks > 65535) {
                continue;  // No válido, probar siguiente
            }
            
            // 🧮 Calcular frecuencia real conseguida
            float freq_real = F_CPU / ((float)prescaler * ticks);
            
            // 📊 Calcular error porcentual
            float error = fabs(freq_real - frecuencia_deseada) / frecuencia_deseada * 100.0;
            
            // 🏆 ¿Es mejor que el resultado anterior?
            if (error < mejor_resultado.error_porcentual) {
                mejor_resultado.prescaler = prescaler;
                mejor_resultado.ticks = ticks;
                mejor_resultado.frecuencia_real = freq_real;
                mejor_resultado.error_porcentual = error;
                mejor_resultado.config_registros = configs[i];
                mejor_resultado.es_valido = true;
            }
        }
        
        return mejor_resultado;
    }

    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                      📊 MOSTRAR ANÁLISIS COMPLETO 📊                           ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     */
    static void analizar_todas_opciones(float frecuencia_deseada) {
        /*
         * Esta función es puramente educativa para entender cómo
         * cada prescaler afecta la precisión y viabilidad.
         * En un sistema real, enviarías esta info por serie o debug.
         */
        
        uint16_t prescalers[] = {1, 8, 64, 256, 1024};
        
        // 📋 Analizar cada opción
        for (uint8_t i = 0; i < 5; i++) {
            uint16_t prescaler = prescalers[i];
            
            // 🧮 Calcular parámetros
            float ticks_float = F_CPU / (frecuencia_deseada * prescaler);
            uint16_t ticks = (uint16_t)(ticks_float + 0.5);
            
            bool cabe_en_16bits = (ticks > 0 && ticks <= 65535);
            
            if (cabe_en_16bits) {
                float freq_real = F_CPU / ((float)prescaler * ticks);
                float error = fabs(freq_real - frecuencia_deseada) / frecuencia_deseada * 100.0;
                float resolucion = (float)prescaler / F_CPU * 1000000.0;  // μs
                
                // 🎯 Los resultados se pueden mostrar via debug
                // Por ahora almacenamos en variables para análisis
            }
        }
    }

    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                        🎯 FUNCIONES DE UTILIDAD 🎯                             ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     */
    
    // 🔢 Calcular frecuencia real dados prescaler y ticks
    static float calcular_frecuencia_real(uint16_t prescaler, uint16_t ticks) {
        return F_CPU / ((float)prescaler * ticks);
    }
    
    // ⏱️ Calcular período real en microsegundos
    static float calcular_periodo_us(uint16_t prescaler, uint16_t ticks) {
        return ((float)prescaler * ticks) / F_CPU * 1000000.0;
    }
    
    // 📏 Calcular resolución temporal en nanosegundos
    static float calcular_resolucion_ns(uint16_t prescaler) {
        return (float)prescaler / F_CPU * 1000000000.0;
    }
    
    // 🎚️ Calcular rango máximo de timing en segundos
    static float calcular_rango_maximo_s(uint16_t prescaler) {
        return (65535.0 * prescaler) / F_CPU;
    }
};

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                      🎓 CONCEPTOS AVANZADOS DE PRESCALER 🎓                        ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 🔧 PRESCALERS DINÁMICOS:
 * • Cambiar prescaler en tiempo real según necesidades
 * • Precisión alta para medidas cortas, rango largo para esperas
 * • Cuidado: cambiar prescaler resetea el contador interno
 * 
 * ⚡ PRESCALERS EN DIFERENTES TIMERS:
 * • Timer0 (8-bit): prescalers 1, 8, 64, 256, 1024
 * • Timer1 (16-bit): prescalers 1, 8, 64, 256, 1024  
 * • Timer2 (8-bit): prescalers 1, 8, 32, 64, 128, 256, 1024
 * • Cada timer puede usar prescaler diferente simultáneamente
 * 
 * 🌊 JITTER Y PRECISIÓN:
 * • Prescalers bajos → menor jitter, mayor precisión
 * • Prescalers altos → mayor jitter, pero timing más largos
 * • Overhead de interrupciones afecta más con prescalers altos
 * 
 * 🔄 SINCRONIZACIÓN ENTRE TIMERS:
 * • Usar mismo prescaler para sincronización perfecta
 * • Reset simultáneo de múltiples timers
 * • Fase-lock entre diferentes frecuencias
 * 
 * ⚙️ PRESCALERS FRACCIONARIOS:
 * • AVR solo tiene prescalers enteros fijos
 * • Para prescalers fraccionarios: usar PLL o técnicas avanzadas
 * • Algunos MCUs modernos sí tienen prescalers programables
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                        🚀 EJEMPLO DE USO PRÁCTICO 🚀                              ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * // Calcular prescaler para 1000 Hz (1 ms)
 * auto resultado = CalculadoraPrescaler::calcular_optimo(1000.0);
 * 
 * if (resultado.es_valido) {
 *     // Configurar Timer1 con el resultado
 *     TCCR1A = 0;  // Modo CTC
 *     TCCR1B = (1 << WGM12) | resultado.config_registros;
 *     OCR1A = resultado.ticks - 1;
 *     TIMSK1 |= (1 << OCIE1A);  // Habilitar interrupción
 *     
 *     // Frecuencia real conseguida: resultado.frecuencia_real
 *     // Error: resultado.error_porcentual %
 * }
 * 
 * // Análisis detallado de todas las opciones
 * CalculadoraPrescaler::analizar_todas_opciones(3840.0);
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                      📊 TABLA DE REFERENCIA RÁPIDA 📊                              ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * PRESCALERS TÍPICOS PARA APLICACIONES COMUNES:
 * 
 * 🎯 ALTA PRECISIÓN, TIMING CORTO:
 * • PWM de alta frecuencia → Prescaler 1 o 8
 * • Medición de pulsos rápidos → Prescaler 1
 * • Control de motores paso a paso → Prescaler 1 o 8
 * 
 * 🎯 EQUILIBRIO PRECISION/RANGO:
 * • Audio sampling (nuestro caso) → Prescaler 8  
 * • Comunicación serie → Prescaler 8 o 64
 * • Control PID de temperatura → Prescaler 64
 * 
 * 🎯 LARGO ALCANCE, BAJA PRECISIÓN:
 * • Timeout de comunicaciones → Prescaler 256 o 1024
 * • Medición de RPM lentas → Prescaler 1024
 * • Watchdog timer → Prescaler 1024
 * • Parpadeo de LEDs → Prescaler 1024
 * 
 * 💡 REGLA GENERAL:
 * "Usa el prescaler MÁS BAJO que permita medir tu tiempo objetivo"
 */

#endif // PRESCALER_TUTORIAL_H

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                              🎉 ¡FELICITACIONES! 🎉                                ║
 * ║                                                                                      ║
 * ║    Has dominado los prescalers desde los fundamentos hasta técnicas avanzadas.     ║
 * ║    Ahora puedes calcular timings precisos para cualquier aplicación embedded.      ║
 * ║                                                                                      ║
 * ║    🎯 Tu siguiente nivel:                                                           ║
 * ║    • Implementar prescalers dinámicos                                               ║
 * ║    • Sincronizar múltiples timers                                                   ║
 * ║    • Optimizar jitter en aplicaciones críticas                                     ║
 * ║    • Diseñar sistemas de timing distribuidos                                        ║
 * ║                                                                                      ║
 * ║                       ¡El timing perfecto te espera! ⏰                             ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 */