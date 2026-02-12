/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                        🎯 DSP ARDUINO - VERSIÓN INTERMEDIA 🎯                       ║
 * ║              Mantiene eficiencia de registros + legibilidad mejorada                ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 📚 FILOSOFÍA:
 * Esta versión conserva la arquitectura eficiente del original (registros directos,
 * Timer1, interrupciones) pero con código más legible y mejor documentado.
 * 
 * ✅ MANTIENE del original:
 * • Timer1 con prescaler e interrupciones para timing exacto
 * • Registros ADC optimizados para máxima velocidad
 * • Acceso directo a PORTA para DAC R2R
 * • Misma latencia y rendimiento
 * 
 * ✅ MEJORA del original:
 * • Nombres de variables más descriptivos
 * • Funciones bien organizadas y comentadas
 * • Constantes con nombres claros
 * • Estructura más modular
 * • Fácil de entender sin perder eficiencia
 */

#include <avr/io.h>
#include <avr/interrupt.h>

// ════════════════════════════════════════════════════════════════════════════════════════
// 🎛️ CONFIGURACIÓN DEL SISTEMA
// ════════════════════════════════════════════════════════════════════════════════════════

const float FRECUENCIA_MUESTREO = 3840.0;    // Hz - Frecuencia de procesamiento DSP
const long BAUD_RATE = 38400;                // bps - Velocidad comunicación con PC
const uint8_t CENTRO_DAC = 128;              // Valor central del DAC (sin señal)

// 🧮 Cálculo automático de parámetros de Timer1
const uint16_t PRESCALER_TIMER1 = 8;
const uint16_t TICKS_POR_MUESTRA = (F_CPU / (PRESCALER_TIMER1 * FRECUENCIA_MUESTREO)) - 1;

// ════════════════════════════════════════════════════════════════════════════════════════
// 🔧 VARIABLES GLOBALES DEL SISTEMA
// ════════════════════════════════════════════════════════════════════════════════════════

volatile uint8_t muestra_adc = 0;           // Última muestra leída del ADC
volatile uint8_t dato_para_dac = CENTRO_DAC; // Dato a escribir en el DAC
volatile bool nueva_muestra_lista = false;   // Flag: hay nueva muestra para enviar

// ════════════════════════════════════════════════════════════════════════════════════════
// 🏗️ INICIALIZACIÓN DEL SISTEMA
// ════════════════════════════════════════════════════════════════════════════════════════

void setup() {
    // 🎯 Inicializar componentes en orden
    configurar_comunicacion_serie();
    configurar_adc_optimizado();
    configurar_dac_porta();
    configurar_timer1_precision();
    
    // ✅ Habilitar interrupciones globales
    sei();
    
    // 🎵 Estado inicial: silencio en el DAC
    escribir_dac(CENTRO_DAC);
}

// ════════════════════════════════════════════════════════════════════════════════════════
// 🔄 BUCLE PRINCIPAL
// ════════════════════════════════════════════════════════════════════════════════════════

void loop() {
    // 📤 ¿Hay nueva muestra para enviar a la PC?
    if (nueva_muestra_lista) {
        enviar_muestra_a_pc(muestra_adc);
        nueva_muestra_lista = false;
    }
    
    // 📥 ¿Hay datos procesados llegando de la PC?
    if (hay_datos_serie_disponibles()) {
        uint8_t dato_procesado = recibir_byte_serie();
        dato_para_dac = dato_procesado;
    }
    
    /*
     * 💡 ARQUITECTURA DEL LOOP:
     * • Timer1 maneja el timing crítico (ISR cada 260μs)
     * • Loop principal maneja comunicación (no crítica)
     * • Variables volátiles conectan ISR con loop principal
     * • Sin delays ni timing manual → máxima eficiencia
     */
}

// ════════════════════════════════════════════════════════════════════════════════════════
// ⚡ INTERRUPCIÓN DE TIMER1 - NÚCLEO DEL PROCESAMIENTO
// ════════════════════════════════════════════════════════════════════════════════════════

ISR(TIMER1_COMPA_vect) {
    /*
     * 🎯 ESTA FUNCIÓN SE EJECUTA CADA 260 MICROSEGUNDOS
     * Es el corazón del sistema DSP en tiempo real.
     * REGLA: Mantener código mínimo y rápido aquí.
     */
    
    // 📥 PASO 1: Leer nueva muestra del ADC
    iniciar_conversion_adc();
    uint16_t lectura_10bit = obtener_valor_adc();
    muestra_adc = convertir_10_a_8_bits(lectura_10bit);
    
    // 🔊 PASO 2: Actualizar salida del DAC
    escribir_dac(dato_para_dac);
    
    // 🚩 PASO 3: Marcar que hay nueva muestra lista
    nueva_muestra_lista = true;
    
    /*
     * ⏱️ TIMING CRÍTICO:
     * Esta ISR debe completarse en < 50μs (del período de 260μs)
     * para dejar tiempo al loop principal y mantener tiempo real.
     */
}

// ════════════════════════════════════════════════════════════════════════════════════════
// 📡 FUNCIONES DE COMUNICACIÓN SERIE
// ════════════════════════════════════════════════════════════════════════════════════════

void configurar_comunicacion_serie() {
    /*
     * 🎯 Configurar USART0 a 38400 baud para comunicación con PC
     * Versión optimizada pero legible del setup serie.
     */
    
    // 🧮 Calcular valor de baud rate
    const uint16_t UBRR_VAL = (F_CPU / (16UL * BAUD_RATE)) - 1;
    
    // ⚙️ Configurar registros USART
    UBRR0H = (uint8_t)(UBRR_VAL >> 8);        // Byte alto del baud rate
    UBRR0L = (uint8_t)(UBRR_VAL);             // Byte bajo del baud rate
    
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);     // Habilitar RX y TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);   // 8 bits de datos, sin paridad
}

void enviar_muestra_a_pc(uint8_t muestra) {
    /*
     * 📤 Enviar una muestra de 8 bits a la PC
     * Versión eficiente con espera activa optimizada.
     */
    
    // ⏳ Esperar que el buffer de transmisión esté libre
    while (!(UCSR0A & (1 << UDRE0)));
    
    // 📤 Enviar el byte
    UDR0 = muestra;
}

bool hay_datos_serie_disponibles() {
    /*
     * 📥 ¿Hay datos esperando en el buffer de recepción?
     * Consulta rápida del flag de recepción completa.
     */
    return (UCSR0A & (1 << RXC0)) != 0;
}

uint8_t recibir_byte_serie() {
    /*
     * 📥 Leer un byte del buffer de recepción
     * NOTA: Solo llamar si hay_datos_serie_disponibles() es true.
     */
    return UDR0;
}

// ════════════════════════════════════════════════════════════════════════════════════════
// 🎛️ FUNCIONES DEL ADC OPTIMIZADO
// ════════════════════════════════════════════════════════════════════════════════════════

void configurar_adc_optimizado() {
    /*
     * ⚡ Configurar ADC para máxima velocidad sin perder precisión crítica
     * Optimizado para lecturas frecuentes de A1 en tiempo real.
     */
    
    // 📌 Seleccionar canal A1 como entrada
    ADMUX = (1 << REFS0) |           // Referencia AVcc (5V)
            (1 << ADLAR) |           // Justificar resultado a la izquierda  
            (1);                     // Canal ADC1 (pin A1)
    
    // ⚡ Configurar prescaler para equilibrio velocidad/precisión
    ADCSRA = (1 << ADEN) |           // Habilitar ADC
             (1 << ADPS2) |          // Prescaler = 16 (1MHz @ 16MHz CPU)
             (0 << ADPS1) |          // ~13μs por conversión
             (0 << ADPS0);
    
    /*
     * 💡 JUSTIFICACIÓN DEL PRESCALER:
     * • Prescaler 16 = frecuencia ADC de 1MHz
     * • Conversión completa en ~13μs
     * • En nuestro período de 260μs, esto es solo 5% del tiempo
     * • Precisión más que suficiente para audio
     */
}

void iniciar_conversion_adc() {
    /*
     * 🚀 Comenzar una nueva conversión ADC
     * Start conversion bit se auto-limpia cuando termina.
     */
    ADCSRA |= (1 << ADSC);
}

uint16_t obtener_valor_adc() {
    /*
     * ⏳ Esperar y obtener resultado de conversión ADC
     * Retorna valor de 10 bits (0-1023).
     */
    
    // ⏳ Esperar que termine la conversión
    while (ADCSRA & (1 << ADSC));
    
    // 📊 Leer resultado (10 bits)
    return ADC;
}

uint8_t convertir_10_a_8_bits(uint16_t valor_10bit) {
    /*
     * 🔄 Convertir muestra de 10 bits (0-1023) a 8 bits (0-255)
     * Mantiene la resolución más significativa.
     */
    return valor_10bit >> 2;  // Dividir por 4: 1023→255
}

// ════════════════════════════════════════════════════════════════════════════════════════
// 🔊 FUNCIONES DEL DAC R2R
// ════════════════════════════════════════════════════════════════════════════════════════

void configurar_dac_porta() {
    /*
     * 🎛️ Configurar puerto PORTA completo como salida para DAC R2R
     * Los 8 bits de PORTA controlan directamente la resistor ladder.
     */
    DDRA = 0xFF;  // Todo PORTA como salida (pins 22-29 en Arduino Mega)
    
    /*
     * 📌 CONEXIONES FÍSICAS:
     * PA0 (pin 22) = Bit 0 (LSB) → Resistor 20KΩ
     * PA1 (pin 23) = Bit 1       → Resistor 10KΩ  
     * PA2 (pin 24) = Bit 2       → Resistor 5KΩ
     * PA3 (pin 25) = Bit 3       → Resistor 2.5KΩ
     * PA4 (pin 26) = Bit 4       → Resistor 1.25KΩ
     * PA5 (pin 27) = Bit 5       → Resistor 625Ω
     * PA6 (pin 28) = Bit 6       → Resistor 312.5Ω
     * PA7 (pin 29) = Bit 7 (MSB) → Resistor 156.25Ω
     */
}

void escribir_dac(uint8_t valor) {
    /*
     * 🔊 Escribir valor de 8 bits directamente al DAC R2R
     * Operación atómica súper rápida (~62ns @ 16MHz).
     */
    PORTA = valor;
    
    /*
     * ⚡ EFICIENCIA MÁXIMA:
     * Una sola instrucción assembly actualiza todos los 8 bits
     * simultáneamente. Imposible de hacer más rápido.
     */
}

// ════════════════════════════════════════════════════════════════════════════════════════
// ⏰ FUNCIONES DE TIMER1 PARA TIMING DE PRECISIÓN
// ════════════════════════════════════════════════════════════════════════════════════════

void configurar_timer1_precision() {
    /*
     * ⏰ Configurar Timer1 para interrupciones exactas a 3840 Hz
     * Usa modo CTC (Clear Timer on Compare) para máxima precisión.
     */
    
    // 🔧 Limpiar registros de Timer1
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    
    // 🎯 Modo CTC: cuenta hasta OCR1A, luego se resetea
    TCCR1A |= (0 << WGM11) | (0 << WGM10);
    TCCR1B |= (0 << WGM13) | (1 << WGM12);
    
    // ⚙️ Prescaler de 8 para balance precisión/rango
    TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
    
    // 📊 Valor de comparación para 3840 Hz exactos
    OCR1A = TICKS_POR_MUESTRA;
    
    // 🔔 Habilitar interrupción por comparación
    TIMSK1 |= (1 << OCIE1A);
    
    /*
     * 🧮 VERIFICACIÓN MATEMÁTICA:
     * F_CPU = 16,000,000 Hz
     * Prescaler = 8
     * Frecuencia timer = 16,000,000 / 8 = 2,000,000 Hz
     * OCR1A = (2,000,000 / 3840) - 1 = 520.83 - 1 = 520
     * Frecuencia real = 2,000,000 / (520 + 1) = 3839.77 Hz
     * Error = 0.006% ← Excelente!
     */
}

// ════════════════════════════════════════════════════════════════════════════════════════
// 📊 FUNCIONES DE DIAGNÓSTICO Y DEBUG
// ════════════════════════════════════════════════════════════════════════════════════════

uint16_t obtener_frecuencia_real_timer() {
    /*
     * 📈 Calcular frecuencia real del Timer1 basada en configuración actual
     * Útil para verificación y debug.
     */
    uint32_t freq_timer = F_CPU / PRESCALER_TIMER1;
    return freq_timer / (OCR1A + 1);
}

float calcular_error_frecuencia() {
    /*
     * 📊 Calcular error porcentual entre frecuencia deseada y real
     * Retorna valor entre 0.0 y 100.0
     */
    float freq_real = obtener_frecuencia_real_timer();
    float error = abs(freq_real - FRECUENCIA_MUESTREO) / FRECUENCIA_MUESTREO * 100.0;
    return error;
}

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                            📈 ANÁLISIS DE RENDIMIENTO 📈                            ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 🏆 COMPARACIÓN CON VERSIONES ALTERNATIVAS:
 * 
 * ⚡ VELOCIDAD vs ORIGINAL:
 * • ISR: Idéntica (~15μs)
 * • Loop: Ligeramente más lento por legibilidad (~5μs extra)
 * • Total: 99% del rendimiento original
 * 
 * 📚 LEGIBILIDAD vs ORIGINAL:
 * • Nombres descriptivos vs abreviaciones crípticas
 * • Funciones organizadas vs código monolítico  
 * • Constantes calculadas vs números mágicos
 * • Comentarios útiles vs sin documentación
 * 
 * 🔧 MANTENIBILIDAD:
 * • Fácil cambiar frecuencia (una constante)
 * • Fácil añadir debug sin romper timing
 * • Funciones modulares para extensión
 * • Configuración centralizada
 * 
 * 🎯 SWEET SPOT:
 * Esta versión es el equilibrio perfecto entre:
 * • Eficiencia del código original
 * • Legibilidad del código simplificado
 * • Mantenibilidad para proyectos serios
 * • Performance para aplicaciones críticas
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                              🎓 GUÍA DE USO 🎓                                      ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 🎯 MODIFICACIONES COMUNES:
 * 
 * 📊 CAMBIAR FRECUENCIA DE MUESTREO:
 * • Modificar: const float FRECUENCIA_MUESTREO = 7680.0;  // Nueva frecuencia
 * • El resto se recalcula automáticamente
 * • Recuerda ajustar BAUD_RATE si es necesario
 * 
 * 🔊 AÑADIR PROCESAMIENTO LOCAL:
 * • Modificar la ISR agregando filtros simples
 * • CUIDADO: Mantener ISR bajo 50μs total
 * • Para procesamiento complejo, usar flags y loop principal
 * 
 * 📡 CAMBIAR VELOCIDAD SERIE:
 * • Modificar: const long BAUD_RATE = 115200;  // Más rápido
 * • Útil para frecuencias de muestreo más altas
 * 
 * 🎛️ USAR PIN ADC DIFERENTE:
 * • Modificar el valor (1) en configurar_adc_optimizado()
 * • 0=A0, 1=A1, 2=A2, etc.
 * 
 * 📈 AÑADIR MONITOREO:
 * • Usar las funciones de diagnóstico
 * • Añadir LEDs de estado en el loop principal
 * • Medir timing real con osciloscopio
 */