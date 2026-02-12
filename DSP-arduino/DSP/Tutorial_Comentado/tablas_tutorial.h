/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                           🌊 TABLAS DE ONDAS TUTORIAL 🌊                           ║
 * ║                     Tutorial Educativo de Generación de Señales                    ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 📚 OBJETIVO EDUCATIVO:
 * Este archivo enseña cómo generar señales de audio digitales desde cero,
 * explicando la matemática, física y programación detrás de cada forma de onda.
 * 
 * 🎯 QUÉ APRENDERÁS:
 * ✅ ¿Qué es una señal digital vs analógica?
 * ✅ ¿Cómo convertir matemática continua a puntos discretos?
 * ✅ ¿Qué es la frecuencia de muestreo?
 * ✅ ¿Cómo se generan senos, cosenos, triangulares y cuadradas?
 * ✅ ¿Qué significa "resolución" en DAC de 8 bits?
 * ✅ ¿Cómo calcular amplitudes y offsets?
 * 
 * 💡 NIVEL: Principiante total → Ingeniero de señales
 */

#ifndef TABLAS_TUTORIAL_H
#define TABLAS_TUTORIAL_H

#include <avr/pgmspace.h>  // ← Para almacenar tablas en memoria FLASH (no RAM)

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                      📡 FUNDAMENTOS: SEÑALES ANALÓGICAS vs DIGITALES 📡            ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 🌊 SEÑAL ANALÓGICA (mundo real):
 * • Es CONTINUA en tiempo y amplitud
 * • Puede tomar cualquier valor en cualquier instante
 * • Ejemplos: sonido en el aire, voltaje de una batería
 * 
 *    Amplitud
 *       ↑
 *     3V|     ∿∿∿∿∿∿∿∿∿∿     ← Curva suave continua
 *     2V|   ∿           ∿
 *     1V| ∿               ∿
 *     0V|∿                 ∿∿∿
 *       └─────────────────────→ Tiempo
 *        0  1  2  3  4  5  6ms
 * 
 * 💻 SEÑAL DIGITAL (en microcontroladores):
 * • Es DISCRETA en tiempo y amplitud
 * • Solo existe en instantes específicos (muestras)
 * • Solo puede tomar valores específicos (cuantización)
 * 
 *    Valor Digital
 *       ↑
 *    255|     ███ ███ ███     ← Escalones discretos
 *    200|   ██     ██ 
 *    150| ██         ██
 *    100|██           ███
 *      0|█               ███
 *       └─────────────────────→ Muestra #
 *        0  1  2  3  4  5  6
 * 
 * 🔄 PROCESO DE DIGITALIZACIÓN:
 * 1. MUESTREO: Tomar valores solo en ciertos instantes
 * 2. CUANTIZACIÓN: Redondear a valores digitales permitidos
 * 3. CODIFICACIÓN: Convertir a números binarios (0s y 1s)
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                        🎵 ¿QUÉ ES LA FRECUENCIA DE MUESTREO? 🎵                   ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 📊 FRECUENCIA DE MUESTREO = Cuántas veces por segundo tomamos una muestra
 * 
 * En nuestro proyecto: 3840 muestras/segundo = 3840 Hz
 * 
 * 🕒 PERÍODO DE MUESTREO = 1/frecuencia = 1/3840 = 260.4 microsegundos
 * 
 * 💡 TEOREMA DE NYQUIST (¡SÚPER IMPORTANTE!):
 * "Para capturar correctamente una señal de frecuencia F, necesitamos
 *  muestrear al menos a 2×F"
 * 
 * 🎯 EN NUESTRO CASO:
 * • Muestreamos a: 3840 Hz
 * • Máxima frecuencia representable: 3840/2 = 1920 Hz
 * • Esto cubre perfectamente audio de voz humana (300-3400 Hz)
 * 
 * 📈 EJEMPLOS DE FRECUENCIAS:
 * • Nota musical La4: 440 Hz
 * • Voz humana: 85-255 Hz (fundamental)
 * • Teléfono: 300-3400 Hz
 * • Audio CD: 20-22050 Hz (muestreo a 44100 Hz)
 * 
 * 🚨 ¿QUÉ PASA SI VIOLAMOS NYQUIST?
 * → ALIASING: Frecuencias altas aparecen como frecuencias bajas
 * → Distorsión irrecuperable de la señal
 * → ¡Por eso necesitamos filtros anti-aliasing!
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                        🎛️ DAC DE 8 BITS: ¿QUÉ SIGNIFICA? 🎛️                        ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 🔢 8 BITS = 2^8 = 256 valores posibles
 * Rango: 0, 1, 2, 3, ..., 254, 255
 * 
 * ⚡ EN NUESTRO HARDWARE:
 * • 0 digital → 0.0V analógico
 * • 255 digital → 5.0V analógico  
 * • Resolución = 5.0V / 255 = 19.6 mV por paso
 * 
 * 📊 TABLA DE CONVERSIÓN:
 * Digital | Analógico | Porcentaje
 * --------|-----------|----------
 *    0    |   0.00V   |    0%
 *   64    |   1.25V   |   25%
 *  128    |   2.50V   |   50% ← Punto medio
 *  192    |   3.75V   |   75%
 *  255    |   5.00V   |  100%
 * 
 * 🎯 PARA SEÑALES BIPOLARES (ej: audio):
 * • Centro en 128 = 2.5V (sin audio)
 * • Máximo positivo: 255 = 5.0V
 * • Máximo negativo: 0 = 0.0V  
 * • Excursión total: ±127 niveles desde el centro
 * 
 * 📏 RESOLUCIÓN vs RUIDO:
 * • Mejor resolución → menos ruido de cuantización
 * • 8 bits → ~48 dB de rango dinámico
 * • Suficiente para voz, limitado para música
 * • CDT usa 16 bits → 96 dB de rango dinámico
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                            📏 CONFIGURACIÓN DEL SISTEMA 📏                          ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * Estos valores definen las características fundamentales de nuestro
 * generador de señales digital.
 */

// 🎯 Parámetros fundamentales del sistema
#define FREQ_MUESTREO      3840.0    // Hz - Cuántas muestras por segundo generamos
#define RESOLUCION_BITS    8         // bits - Precisión de nuestro DAC  
#define NIVELES_DAC        256       // 2^8 = cantidad de valores posibles
#define CENTRO_DAC         128       // Punto medio para señales bipolares
#define AMPLITUD_MAXIMA    127       // Máxima excursión desde el centro

// 📦 Tamaño de las tablas de ondas
#define PUNTOS_POR_ONDA    64        // Muestras que definen un ciclo completo

/*
 * 🤔 ¿POR QUÉ 64 PUNTOS POR ONDA?
 * 
 * 📊 TRADE-OFFS DEL TAMAÑO DE TABLA:
 * 
 * ✅ MÁS PUNTOS (ej: 256):
 * • Mayor resolución → ondas más suaves
 * • Menos distorsión armónica
 * • Más memoria RAM/FLASH consumida
 * 
 * ✅ MENOS PUNTOS (ej: 16):  
 * • Menos memoria consumida
 * • Cálculos más rápidos
 * • Mayor distorsión → ondas "dentadas"
 * 
 * 🎯 64 PUNTOS = EQUILIBRIO PERFECTO:
 * • Suficiente resolución para audio de calidad
 * • Memoria razonable (64 bytes por tabla)
 * • Cálculos eficientes en tiempo real
 * • Múltiplo de potencia de 2 → optimización matemática
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                          🌊 GENERACIÓN DE ONDA SENOIDAL 🌊                         ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * La onda senoidal es la forma de onda más fundamental en procesamiento
 * de señales. Todas las demás ondas se pueden construir combinando senoidales.
 */

/*
 * 🧮 MATEMÁTICA DETRÁS DE LA ONDA SENOIDAL:
 * 
 * Fórmula continua: y(t) = A × sin(2π × f × t) + offset
 * Donde:
 * • A = amplitud máxima
 * • f = frecuencia en Hz
 * • t = tiempo en segundos  
 * • offset = desplazamiento vertical
 * 
 * 🔢 CONVERSIÓN A DIGITAL:
 * Para punto i de N puntos totales:
 * 
 * ángulo = (2π × i) / N
 * valor_float = sin(ángulo)              ← Entre -1 y +1
 * valor_digital = (valor_float * AMPLITUD_MAXIMA) + CENTRO_DAC
 * 
 * 📊 EJEMPLO PASO A PASO (N=8 para simplicidad):
 * i=0: ángulo=0°     sin(0°)=0      → 0×127+128 = 128
 * i=1: ángulo=45°    sin(45°)=0.707  → 90+128 = 218  
 * i=2: ángulo=90°    sin(90°)=1      → 127+128 = 255
 * i=3: ángulo=135°   sin(135°)=0.707 → 90+128 = 218
 * i=4: ángulo=180°   sin(180°)=0     → 0+128 = 128
 * i=5: ángulo=225°   sin(225°)=-0.707→ -90+128 = 38
 * i=6: ángulo=270°   sin(270°)=-1    → -127+128 = 1  
 * i=7: ángulo=315°   sin(315°)=-0.707→ -90+128 = 38
 */

// 🌊 Tabla precalculada de onda senoidal (almacenada en FLASH para ahorrar RAM)
const uint8_t tabla_seno[PUNTOS_POR_ONDA] PROGMEM = {
    128, 141, 153, 164, 174, 182, 189, 194,    // ← 0° a 90° (primer cuadrante)
    197, 198, 197, 194, 189, 182, 174, 164,    // ← 90° a 180° (segundo cuadrante)  
    153, 141, 128, 115, 103, 92,  82,  74,     // ← 180° a 270° (tercer cuadrante)
    67,  62,  59,  58,  59,  62,  67,  74,     // ← 270° a 360° (cuarto cuadrante)
    82,  92,  103, 115, 128, 141, 153, 164,    // ← Repetición para continuidad
    174, 182, 189, 194, 197, 198, 197, 194,
    189, 182, 174, 164, 153, 141, 128, 115,
    103, 92,  82,  74,  67,  62,  59,  58
};

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                        ▲ GENERACIÓN DE ONDA TRIANGULAR ▲                           ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * La onda triangular es más simple matemáticamente que la senoidal,
 * pero contiene más armónicos (frecuencias múltiplos de la fundamental).
 */

/*
 * 🧮 MATEMÁTICA DETRÁS DE LA ONDA TRIANGULAR:
 * 
 * Una triangular se puede generar de varias formas:
 * 
 * 📈 MÉTODO 1: Rampa lineal con cambio de pendiente
 * • Primera mitad (0 a N/2): valor aumenta linealmente
 * • Segunda mitad (N/2 a N): valor disminuye linealmente
 * 
 * 📈 MÉTODO 2: Función matemática
 * y(t) = (4A/T) × |t - T/2| + offset
 * 
 * 🔢 IMPLEMENTACIÓN DIGITAL (método rampa):
 * Primera mitad: valor = (i × 2 × AMPLITUD_MAXIMA) / N + valor_minimo
 * Segunda mitad: valor = valor_maximo - ((i-N/2) × 2 × AMPLITUD_MAXIMA) / N
 * 
 * 📊 EJEMPLO (N=8):
 * i=0: primera mitad → 0×255/4 + 1 = 1
 * i=1: primera mitad → 1×255/4 + 1 = 64
 * i=2: primera mitad → 2×255/4 + 1 = 128  
 * i=3: primera mitad → 3×255/4 + 1 = 192
 * i=4: segunda mitad → 255 - (0×255/4) = 255
 * i=5: segunda mitad → 255 - (1×255/4) = 192
 * i=6: segunda mitad → 255 - (2×255/4) = 128
 * i=7: segunda mitad → 255 - (3×255/4) = 64
 */

// ▲ Tabla precalculada de onda triangular
const uint8_t tabla_triangular[PUNTOS_POR_ONDA] PROGMEM = {
    1,   9,   17,  25,  33,  41,  49,  57,     // ← Subida: pendiente positiva
    65,  73,  81,  89,  97,  105, 113, 121,    
    129, 137, 145, 153, 161, 169, 177, 185,
    193, 201, 209, 217, 225, 233, 241, 249,    // ← Pico máximo
    255, 249, 241, 233, 225, 217, 209, 201,    // ← Bajada: pendiente negativa
    193, 185, 177, 169, 161, 153, 145, 137,
    129, 121, 113, 105, 97,  89,  81,  73,
    65,  57,  49,  41,  33,  25,  17,  9       // ← Valle mínimo
};

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                         ⬜ GENERACIÓN DE ONDA CUADRADA ⬜                           ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * La onda cuadrada es la más simple: solo dos valores (alto y bajo),
 * pero contiene muchísimos armónicos impares que la hacen "agresiva".
 */

/*
 * 🧮 MATEMÁTICA DETRÁS DE LA ONDA CUADRADA:
 * 
 * ⬜ DEFINICIÓN SIMPLE:
 * • Primera mitad del período: valor máximo
 * • Segunda mitad del período: valor mínimo  
 * • Transiciones instantáneas (teóricamente)
 * 
 * 📊 IMPLEMENTACIÓN DIGITAL:
 * if (i < N/2) {
 *     valor = VALOR_ALTO;
 * } else {
 *     valor = VALOR_BAJO;  
 * }
 * 
 * 📈 ANÁLISIS ARMÓNICO (¡Importante!):
 * Una cuadrada perfecta contiene:
 * • Frecuencia fundamental (f)
 * • Todos los armónicos impares: 3f, 5f, 7f, 9f, ...
 * • Con amplitudes decrecientes: 1/3, 1/5, 1/7, 1/9, ...
 * 
 * 🎵 EJEMPLO MUSICAL:
 * Si f=440Hz (nota La):
 * • Fundamental: 440 Hz
 * • 3er armónico: 1320 Hz  
 * • 5to armónico: 2200 Hz
 * • 7mo armónico: 3080 Hz
 * • ... y así hasta muy alta frecuencia
 * 
 * 🚨 PROBLEMA EN SISTEMAS REALES:
 * • Los armónicos altos pueden causar aliasing
 * • Necesitamos filtros pasa-bajos para limitarlos
 * • Las transiciones "instantáneas" son físicamente imposibles
 */

// ⬜ Tabla precalculada de onda cuadrada
const uint8_t tabla_cuadrada[PUNTOS_POR_ONDA] PROGMEM = {
    255, 255, 255, 255, 255, 255, 255, 255,    // ← Primera mitad: ALTO (máximo)
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    1,   1,   1,   1,   1,   1,   1,   1,      // ← Segunda mitad: BAJO (mínimo)
    1,   1,   1,   1,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   1,   1,  
    1,   1,   1,   1,   1,   1,   1,   1
};

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                       🌊 ONDA DIENTE DE SIERRA (SAWTOOTH) 🌊                       ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * La onda diente de sierra sube linealmente y luego cae abruptamente,
 * o viceversa. Contiene todos los armónicos (pares e impares).
 */

/*
 * 🧮 MATEMÁTICA DETRÁS DE LA DIENTE DE SIERRA:
 * 
 * 📈 RAMPA ASCENDENTE (lo que implementamos):
 * valor(i) = (i × RANGO_COMPLETO) / N + valor_minimo
 * 
 * 📊 PARA N=64, RANGO_COMPLETO=254:
 * i=0:  valor = (0×254)/64 + 1 = 1
 * i=16: valor = (16×254)/64 + 1 = 64  
 * i=32: valor = (32×254)/64 + 1 = 128
 * i=48: valor = (48×254)/64 + 1 = 192
 * i=63: valor = (63×254)/64 + 1 = 255
 * 
 * 🎵 CARACTERÍSTICAS TONALES:
 * • Sonido "brillante" y "rico"
 * • Contiene armónicos pares E impares
 * • Amplitudes decrecientes: 1/2, 1/3, 1/4, 1/5, ...
 * • Usado en sintetizadores para sonidos de cuerdas
 */

// 🌊 Tabla precalculada de onda diente de sierra
const uint8_t tabla_diente_sierra[PUNTOS_POR_ONDA] PROGMEM = {
    1,   5,   9,   13,  17,  21,  25,  29,     // ← Subida linear constante
    33,  37,  41,  45,  49,  53,  57,  61,
    65,  69,  73,  77,  81,  85,  89,  93,
    97,  101, 105, 109, 113, 117, 121, 125,
    129, 133, 137, 141, 145, 149, 153, 157,
    161, 165, 169, 173, 177, 181, 185, 189,
    193, 197, 201, 205, 209, 213, 217, 221,
    225, 229, 233, 237, 241, 245, 249, 253    // ← Máximo, luego reset a 1
};

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                           📏 CLASE GENERADOR DE ONDAS 📏                            ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * Esta clase maneja la generación de todas las formas de onda y permite
 * cambiar frecuencias dinámicamente sin recalcular las tablas.
 */

class GeneradorOndas {
private:
    // 🎯 Variables de estado del generador
    uint16_t indice_actual;           // ← Posición actual en la tabla (0-63)
    uint16_t incremento_fase;         // ← Cuánto avanzar por muestra
    uint8_t tipo_onda;                // ← Qué forma de onda generar
    float frecuencia_actual;          // ← Frecuencia en Hz que estamos generando

    /*
     * 🧮 ¿CÓMO FUNCIONA EL INCREMENTO DE FASE?
     * 
     * Para generar diferentes frecuencias con la misma tabla,
     * cambiamos la VELOCIDAD a la que recorremos la tabla.
     * 
     * 📊 CÁLCULO DEL INCREMENTO:
     * incremento = (frecuencia_deseada × PUNTOS_POR_ONDA) / FREQ_MUESTREO
     * 
     * 🎯 EJEMPLOS:
     * Para 100Hz: incremento = (100 × 64) / 3840 = 1.67
     * Para 200Hz: incremento = (200 × 64) / 3840 = 3.33
     * Para 400Hz: incremento = (400 × 64) / 3840 = 6.67
     * 
     * 💡 INTERPRETACIÓN:
     * • incremento < 1: Avanzamos menos de 1 punto por muestra → frecuencia baja
     * • incremento = 1: Avanzamos exactamente 1 punto por muestra → frecuencia base
     * • incremento > 1: Saltamos puntos → frecuencia alta
     * 
     * 🔢 USO DE PUNTO FIJO:
     * Usamos uint16_t con 8 bits fraccionarios:
     * • Bits 15-8: parte entera
     * • Bits 7-0: parte fraccionaria
     * • Rango: 0.00390625 a 255.996
     * • Resolución: 1/256 = 0.00390625
     */

public:
    // 🎭 Enumeración de tipos de onda disponibles
    enum TipoOnda {
        SENO = 0,
        TRIANGULAR = 1,  
        CUADRADA = 2,
        DIENTE_SIERRA = 3
    };

    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                        🏗️ CONSTRUCTOR DEL GENERADOR 🏗️                         ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     */
    GeneradorOndas() {
        // 🎯 Inicialización con valores por defecto
        indice_actual = 0;           // Empezar al inicio de la tabla
        tipo_onda = SENO;            // Onda senoidal por defecto
        frecuencia_actual = 440.0;   // Nota musical La4
        
        // 🧮 Calcular incremento inicial
        calcular_incremento();
    }

    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                      🎛️ CONFIGURAR NUEVA FRECUENCIA 🎛️                         ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     */
    void establecer_frecuencia(float nueva_frecuencia) {
        // 🚨 Validación de entrada
        if (nueva_frecuencia < 0.1) nueva_frecuencia = 0.1;        // Mínimo
        if (nueva_frecuencia > 1920.0) nueva_frecuencia = 1920.0;  // Máximo (Nyquist)
        
        frecuencia_actual = nueva_frecuencia;
        calcular_incremento();
        
        /*
         * 💡 ¿POR QUÉ ESTOS LÍMITES?
         * • Mínimo 0.1Hz: Evita incrementos demasiado pequeños
         * • Máximo 1920Hz: Respeta teorema de Nyquist (3840/2)
         * • Fuera de estos rangos → comportamiento impredecible
         */
    }

    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                       🌊 CONFIGURAR TIPO DE ONDA 🌊                            ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     */
    void establecer_tipo_onda(TipoOnda nuevo_tipo) {
        tipo_onda = nuevo_tipo;
        
        /*
         * 🔄 CAMBIO INSTANTÁNEO:
         * • No afecta la frecuencia ni la fase actual
         * • Solo cambia qué tabla usamos para leer valores
         * • Permite efectos interesantes cambiando forma durante reproducción
         */
    }

    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                        📊 GENERAR PRÓXIMA MUESTRA 📊                           ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     */
    uint8_t obtener_muestra() {
        // 🎯 Obtener la parte entera del índice actual
        uint8_t indice_tabla = (indice_actual >> 8) & 0x3F;  // Máscara para 0-63
        
        /*
         * 🔍 EXPLICACIÓN DEL CÁLCULO:
         * 
         * indice_actual es formato 8.8 (8 bits enteros, 8 fraccionarios):
         * 
         * Ejemplo: indice_actual = 0x0280 = 640 decimal
         * En binario: 00000010 10000000
         *             ^^^^^^^^ ^^^^^^^^
         *             entero   fracción
         * 
         * >> 8: Desplazar 8 bits → 00000010 = 2 decimal
         * & 0x3F: AND con 0011111 → forzar rango 0-63
         * 
         * Resultado: indice_tabla = 2
         */
        
        // 📖 Leer valor de la tabla correspondiente  
        uint8_t valor;
        switch (tipo_onda) {
            case SENO:
                valor = pgm_read_byte(&tabla_seno[indice_tabla]);
                break;
            case TRIANGULAR:
                valor = pgm_read_byte(&tabla_triangular[indice_tabla]);
                break;
            case CUADRADA:
                valor = pgm_read_byte(&tabla_cuadrada[indice_tabla]);
                break;
            case DIENTE_SIERRA:
                valor = pgm_read_byte(&tabla_diente_sierra[indice_tabla]);
                break;
            default:
                valor = CENTRO_DAC;  // Silencio en caso de error
                break;
        }
        
        /*
         * 🤔 ¿POR QUÉ pgm_read_byte()?
         * • Las tablas están en FLASH (PROGMEM) para ahorrar RAM
         * • No podemos leerlas como arrays normales
         * • pgm_read_byte() lee específicamente de FLASH
         * • Es más lento que RAM, pero ahorramos 256 bytes valiosos
         */
        
        // 🔄 Avanzar para la próxima muestra
        indice_actual += incremento_fase;
        
        /*
         * 🔁 WRAP-AROUND AUTOMÁTICO:
         * Como indice_actual es uint16_t, automáticamente hace overflow
         * cuando pasa de 0xFFFF a 0x0000. Esto nos da repetición
         * automática de la onda sin código adicional.
         */
        
        return valor;
    }

    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                         🔄 REINICIAR FASE DE ONDA 🔄                           ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     */
    void reiniciar_fase() {
        indice_actual = 0;
        
        /*
         * 💡 USOS TÍPICOS:
         * • Sincronizar múltiples osciladores
         * • Inicializar al cambiar de forma de onda
         * • Evitar "clicks" en cambios de parámetros
         * • Crear efectos especiales de sincronización
         */
    }

    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                        📈 FUNCIONES DE DIAGNÓSTICO 📈                          ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     */
    
    // 🔍 Obtener frecuencia actual
    float obtener_frecuencia() {
        return frecuencia_actual;
    }
    
    // 🔍 Obtener tipo de onda actual  
    TipoOnda obtener_tipo_onda() {
        return (TipoOnda)tipo_onda;
    }
    
    // 🔍 Obtener posición en la onda (0.0 a 1.0)
    float obtener_fase() {
        return (float)(indice_actual >> 8) / PUNTOS_POR_ONDA;
    }

private:
    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                      🧮 CALCULAR INCREMENTO DE FASE 🧮                         ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     */
    void calcular_incremento() {
        /*
         * 📐 FÓRMULA FUNDAMENTAL:
         * incremento = (frecuencia × PUNTOS_POR_ONDA × 256) / FREQ_MUESTREO
         * 
         * ×256 porque usamos formato 8.8 punto fijo
         */
        
        float incremento_float = (frecuencia_actual * PUNTOS_POR_ONDA * 256.0) / FREQ_MUESTREO;
        incremento_fase = (uint16_t)incremento_float;
        
        /*
         * 📊 EJEMPLO DETALLADO:
         * Para frecuencia = 440 Hz:
         * 
         * incremento_float = (440 × 64 × 256) / 3840
         *                  = 7,208,960 / 3840  
         *                  = 1877.33
         * 
         * incremento_fase = 1877 (truncado)
         * 
         * 🔢 INTERPRETACIÓN EN FORMATO 8.8:
         * 1877 = 0x0755 = 00000111 01010101
         *                 ^^^^^^^^ ^^^^^^^^  
         *                 7 entero 85/256 fracción
         * 
         * = 7.332 en decimal
         * 
         * ✅ VERIFICACIÓN:
         * Frecuencia real = (1877 × 3840) / (64 × 256)
         *                 = 7,207,680 / 16,384
         *                 = 439.97 Hz ← ¡Excelente precisión!
         */
    }
};

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                         🎼 FRECUENCIAS MUSICALES COMUNES 🎼                        ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * Estas constantes definen frecuencias musicales estándar para facilitar
 * la programación de generadores de tonos musicales.
 */

// 🎵 Notas musicales en la octava central (octava 4)
#define NOTA_DO4    261.63    // Hz
#define NOTA_DO4S   277.18    // Do sostenido  
#define NOTA_RE4    293.66    // Re
#define NOTA_RE4S   311.13    // Re sostenido
#define NOTA_MI4    329.63    // Mi
#define NOTA_FA4    349.23    // Fa  
#define NOTA_FA4S   369.99    // Fa sostenido
#define NOTA_SOL4   392.00    // Sol
#define NOTA_SOL4S  415.30    // Sol sostenido
#define NOTA_LA4    440.00    // La (nota de afinación universal)
#define NOTA_LA4S   466.16    // La sostenido
#define NOTA_SI4    493.88    // Si

/*
 * 🎼 ESCALAS Y ACORDES ÚTILES:
 * 
 * 🎯 ESCALA DE DO MAYOR:
 * DO4, RE4, MI4, FA4, SOL4, LA4, SI4, DO5
 * 
 * 🎯 ACORDE DE DO MAYOR:
 * DO4 (261.63), MI4 (329.63), SOL4 (392.00)
 * 
 * 🎯 ACORDE DE LA MENOR:
 * LA4 (440.00), DO5 (523.25), MI5 (659.25)
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                        🚀 EJEMPLO DE USO COMPLETO 🚀                              ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * // Crear generador de ondas
 * GeneradorOndas generador;
 * 
 * void setup() {
 *     // Configurar onda senoidal a 440Hz (La4)
 *     generador.establecer_tipo_onda(GeneradorOndas::SENO);
 *     generador.establecer_frecuencia(NOTA_LA4);
 * }
 * 
 * void loop() {
 *     // En la interrupción de Timer1 (3840Hz):
 *     uint8_t muestra = generador.obtener_muestra();
 *     enviar_a_DAC(muestra);
 *     
 *     // Cambiar tipo de onda cada segundo:
 *     static uint16_t contador = 0;
 *     if (++contador >= 3840) {  // 1 segundo
 *         contador = 0;
 *         static uint8_t tipo = 0;
 *         generador.establecer_tipo_onda((GeneradorOndas::TipoOnda)tipo);
 *         tipo = (tipo + 1) % 4;  // Ciclar entre 4 tipos
 *     }
 * }
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                          🎓 CONCEPTOS AVANZADOS 🎓                                 ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 🌊 INTERPOLACIÓN LINEAR:
 * • Suavizar ondas calculando valores entre puntos de la tabla
 * • Reduce distorsión en frecuencias altas
 * • Costo: más cálculos por muestra
 * 
 * 🎚️ CONTROL DE AMPLITUD (ENVELOPE):
 * • ADSR: Attack, Decay, Sustain, Release
 * • Multiplicar cada muestra por factor de amplitud variable
 * • Crear sonidos más realistas y musicales
 * 
 * 🔄 MODULACIÓN:
 * • FM: Frecuency Modulation (modular frecuencia)
 * • AM: Amplitude Modulation (modular amplitud)  
 * • PWM: Pulse Width Modulation (modular ancho pulso cuadrada)
 * 
 * 🎼 SÍNTESIS ADITIVA:
 * • Combinar múltiples ondas senoidales
 * • Cada una con su frecuencia, fase y amplitud
 * • Crear timbres complejos e instrumentos virtuales
 * 
 * 🔊 EFECTOS DE AUDIO:
 * • Reverb: Simular acústica de espacios
 * • Chorus: Duplicar señal con pequeñas variaciones
 * • Distorsión: Saturar o clipear la señal
 * • Filtros: Pasa-bajos, pasa-altos, pasa-banda
 */

#endif // TABLAS_TUTORIAL_H

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                              🎉 ¡FELICITACIONES! 🎉                                ║
 * ║                                                                                      ║
 * ║   Has dominado los fundamentos de generación de señales digitales desde cero.      ║
 * ║   Ahora entiendes la matemática, física y programación detrás de cada onda.        ║
 * ║                                                                                      ║
 * ║   🎯 Tu próximo desafío:                                                            ║
 * ║   • Implementar múltiples osciladores simultáneos                                   ║
 * ║   • Añadir control de amplitud (ADSR)                                               ║
 * ║   • Crear efectos de modulación (FM/AM)                                             ║
 * ║   • Desarrollar tu propio sintetizador musical                                      ║
 * ║                                                                                      ║
 * ║                    ¡El mundo del DSP te espera! 🌊🎵                                ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 */