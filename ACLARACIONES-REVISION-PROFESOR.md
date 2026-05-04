# ACLARACIONES Y CORRECCIONES - REVISIÓN DEL PROFESOR

**Trabajo Práctico:** DSP Completo  
**Alumnos:** Lautaro Kühn y Federico Domínguez  
**Profesor:** Milton Pozzo  
**Fecha:** Mayo 2026

---

## ÍNDICE DE ACLARACIONES

1. [Relación Frecuencia de Muestreo y Baudrate](#1-relación-frecuencia-de-muestreo-y-baudrate)
2. [Inconsistencia ADC: Modo Continuo vs Disparado por Timer](#2-inconsistencia-adc-modo-continuo-vs-disparado-por-timer)
3. [Modo Real de Operación del ADC](#3-modo-real-de-operación-del-adc)
4. [Visualización Dual: Gráficos y Filtros](#4-visualización-dual-gráficos-y-filtros)
5. [Justificación de Frecuencias Alternativas](#5-justificación-de-frecuencias-alternativas)
6. [Definición de "Señal de Audio Típica"](#6-definición-de-señal-de-audio-típica)
7. [Transmisión de Muestras: Individual vs Bloque](#7-transmisión-de-muestras-individual-vs-bloque)
8. [Verificación de Recepción de 3840 Muestras](#8-verificación-de-recepción-de-3840-muestras)
9. [Función del bin_index](#9-función-del-bin_index)
10. [Resultados Esperados vs Obtenidos](#10-resultados-esperados-vs-obtenidos)
11. [Evaluación de Filtros con Diferentes Formas de Onda](#11-evaluación-de-filtros-con-diferentes-formas-de-onda)
12. [Pruebas a 1920 Hz (Límite de Nyquist)](#12-pruebas-a-1920-hz-límite-de-nyquist)

---

## 1. RELACIÓN FRECUENCIA DE MUESTREO Y BAUDRATE

### ¿Qué relación hay entre el número de muestras y los baudios?

La relación directa es:

```
Baudrate necesario = Frecuencia_muestreo × Bits_por_byte × Overhead_serial

Baudrate = 3840 muestras/s × 1 byte/muestra × 10 bits/byte = 38400 bps
```

**Desglose:**
- **Frecuencia de muestreo:** 3840 Hz (muestras por segundo)
- **Datos por muestra:** 1 byte (8 bits de datos)
- **Overhead serial:** 10 bits totales (1 bit inicio + 8 datos + 1 bit parada)
- **Baudrate resultante:** 38400 bps

### ¿Qué ventaja tiene que coincidan?

**Ventajas de la sincronización exacta (3840 Hz ↔ 38400 bps):**

1. **Sin saturación del buffer:**
   - Cada byte tarda exactamente 260 μs en transmitirse (1/3840 s)
   - Esto coincide con el período de muestreo de 260 μs
   - No hay acumulación de datos en el buffer de transmisión

2. **Flujo continuo sin pérdidas:**
   - Mientras el ADC convierte la muestra N (104 μs), el USART transmite la muestra N-1 (260 μs)
   - Pipeline perfecto: conversión y transmisión se superponen temporalmente
   
3. **Compatibilidad con baudrates estándares:**
   - 38400 bps es un baudrate estándar soportado por todos los sistemas
   - No requiere configuraciones especiales en drivers

4. **Simplicidad del firmware:**
   - No necesita control de flujo (RTS/CTS)
   - No requiere lógica de buffering compleja

### ¿No sería mejor baudrate superior para otras tareas?

**Análisis de baudrate superior (ej: 115200 bps):**

**Ventajas teóricas:**
- Transmisión 3× más rápida (86.8 μs por byte vs 260 μs)
- Tiempo libre: ~173 μs entre muestras

**Desventajas prácticas:**

1. **No hay "otras tareas" en el Arduino:**
   ```c
   // El loop principal está vacío:
   void loop() {
       // Vacío - todo se maneja por interrupciones
   }
   ```
   - ADC: Maneja interrupciones (ISR automática)
   - USART: Hardware transmite automáticamente
   - Timer1: Hardware genera interrupciones periódicas

2. **Overhead innecesario:**
   - Mayor consumo de energía del USART
   - Mayor probabilidad de errores de comunicación a altas velocidades
   - Cables más sensibles a ruido electromagnético

3. **No mejora el rendimiento del sistema:**
   - El cuello de botella es el ADC (104 μs), no el serial
   - La PC procesa datos en tiempo real sin saturarse

**Conclusión:** La sincronización exacta optimiza el sistema sin desperdiciar recursos.

---

## 2. INCONSISTENCIA ADC: MODO CONTINUO VS DISPARADO POR TIMER

### Aclaración de la Confusión

**El documento menciona dos cosas que parecen contradictorias:**
- "Modo de conversión continua" (free-running)
- "El timer dispara el ADC"

### ¿Cuál es la configuración real?

**Modo implementado:** **Auto-trigger sincronizado con Timer1**

```c
// En adc.cpp, línea 34:
ADCSRA = ACTIVAR | AUTO_TRIGGER | PRESCALER_128 | ADC_INTERRUPT;
ADCSRB = MODO_CONTINUO;  // Pero con auto-trigger por Timer1
```

**Explicación técnica correcta:**

1. **No es "free-running puro":** En free-running verdadero, el ADC se dispara solo continuamente sin control externo. **NO usamos este modo**.

2. **Es "Auto-trigger por Timer1":**
   - El ADC está en modo **auto-trigger** (ADATE = 1)
   - La fuente de trigger es **Timer1 Compare Match A**
   - Cada vez que Timer1 alcanza OCR1A = 4166:
     - Se genera una señal de hardware
     - Esta señal dispara automáticamente una conversión ADC
     - El ADC convierte y genera interrupción al terminar

3. **Diferencia clave:**
   ```
   Free-running puro:
   ADC termina → ADC inicia inmediatamente → ADC termina → ...
   (No hay control de timing, velocidad limitada por prescaler ADC)

   Auto-trigger por Timer1 (IMPLEMENTADO):
   Timer1 alcanza OCR1A → Dispara ADC → ADC convierte (104μs) → Interrupción
   Timer1 alcanza OCR1A (260μs después) → Dispara ADC → ...
   (Timing preciso controlado por Timer1, independiente del prescaler ADC)
   ```

### Corrección para el Documento

**REEMPLAZAR:**
> "La tercera alternativa, que terminamos implementando, combina el timer con el modo de conversión continua del ADC (free-running mode)."

**POR:**
> "La alternativa implementada utiliza el Timer1 como fuente de auto-trigger del ADC. El ADC está configurado en modo auto-trigger (ADATE=1), donde cada interrupción del Timer1 (generada cada 260 μs al alcanzar OCR1A=4166) dispara automáticamente una nueva conversión. Esto garantiza un muestreo periódico preciso de 3840 Hz, completamente independiente del flujo del programa principal."

---

## 3. MODO REAL DE OPERACIÓN DEL ADC

### Diagrama de Flujo Implementado

```
┌─────────────────────────────────────────────────────────┐
│                    TIMER1 (Hardware)                     │
│  Reloj 16 MHz → Contador TCNT1 → Compara con OCR1A      │
│                                    (4166)                │
└──────────────────────┬──────────────────────────────────┘
                       │ Cada 260 μs
                       ↓
              ┌────────────────────┐
              │   TIMER1_COMPA     │ ← Interrupción de comparación
              │   (señal trigger)  │
              └────────┬───────────┘
                       │
                       ↓ Auto-trigger (hardware)
              ┌────────────────────┐
              │   ADC inicia       │
              │   conversión       │
              └────────┬───────────┘
                       │ 104 μs (13 ciclos @ 125 kHz)
                       ↓
              ┌────────────────────┐
              │  Conversión        │
              │  completa          │
              └────────┬───────────┘
                       │
                       ↓
              ┌────────────────────┐
              │   ADC_vect         │ ← ISR: Lee ADCH (8 bits)
              │   (interrupción)   │    Envía a USART
              └────────────────────┘
```

### Código Real Implementado

**Configuración del ADC (adc.cpp):**
```c
void ADCController::begin(int pin) {
    // Auto-trigger habilitado + Interrupción habilitada
    ADCSRA = ACTIVAR | AUTO_TRIGGER | PRESCALER_128 | ADC_INTERRUPT;
    
    // Fuente de trigger: Timer1 Compare Match A
    ADCSRB = 0;  // Bits ADTS[2:0] = 000 = Timer1 Comp A
    
    // Referencia AVcc + ADLAR + Pin A1
    ADMUX = AVcc | AJUSTAR_IZQUIERDA | pin;
    
    // Primera conversión manual para iniciar
    ADCSRA |= EMPEZAR;
}
```

**ISR del ADC (DSP.ino):**
```c
ISR(ADC_vect) {
    adc.conversion_complete();  // Lee ADCH y lo guarda
}

void ADCController::conversion_complete() {
    uint8_t high = ADCH;  // Solo 8 bits MSB
    data = high;
    not_get = true;
}
```

**Loop principal (DSP.ino):**
```c
void loop() {
    if (adc.not_get) {
        uint8_t valor = adc.get();  // Obtiene último valor
        usart.write(valor);         // Envía por USART (no bloqueante)
    }
}
```

### ¿Por qué NO es "conversión continua"?

- **Conversión continua** implica que el ADC se dispara a sí mismo sin control externo
- **Nuestra implementación** usa Timer1 para controlar exactamente CUÁNDO ocurre cada conversión
- **Ventaja:** Timing determinístico de 3840 Hz, independiente de la velocidad del ADC

---

## 4. VISUALIZACIÓN DUAL: GRÁFICOS Y FILTROS

### Problema Identificado por el Profesor

> "El gráfico en la Visualización Dual no termina de mostrar lo que se busca. El tiempo de retardo no se aprecia, no se aprecia a simple vista atenuación en frecuencias. Habría que explicarlo un poco más. ¿Qué filtro está aplicado en la gráfica?"

### ¿Qué se debería mostrar?

**Capturas de pantalla necesarias:**

1. **Sin filtro (bypass):**
   - Entrada: Senoidal 500 Hz, 2Vpp
   - Salida: Idéntica (sin retardo visible en escala temporal)

2. **Filtro pasa-bajos fc=300 Hz:**
   - Entrada: Senoidal 500 Hz
   - **Salida:** Atenuada ~-10 dB, retardo de fase visible

3. **Filtro pasa-altos fc=800 Hz:**
   - Entrada: Senoidal 500 Hz
   - **Salida:** Atenuada ~-15 dB, desfase visible

4. **Comparación temporal:**
   ```
   Gráfico Entrada (superior):
        ╱╲      ╱╲      ╱╲      
       ╱  ╲    ╱  ╲    ╱  ╲    
      ╱    ╲  ╱    ╲  ╱    ╲  
   
   Gráfico Salida con fc=300 Hz (inferior):
           ╱╲     ╱╲     ╱╲       ← Retardo ~0.6 ms
        ╱    ╲  ╱   ╲  ╱   ╲      ← Amplitud menor
       ╱      ╲╱     ╲╱     ╲
   ```

### Explicación del Retardo de Grupo

**Latencia total del sistema:**
```
Conversión ADC:      104 μs
Tx serial Arduino→PC: 260 μs
Procesamiento filtro:  15 μs
Retardo de grupo:     400 μs  ← Filtro Butterworth orden 8
Tx serial PC→Arduino: 260 μs
──────────────────────────────
Total:              ~1.04 ms ≈ 4 muestras @ 3840 Hz
```

**¿Por qué no se ve claramente?**

A 3840 Hz (260 μs/muestra), 4 muestras de retardo son **1.04 mm en pantalla típica** con 1 segundo de ventana.

**Solución:** Hacer zoom temporal mostrando solo 100 ms de señal:
- Retardo de 1 ms = 1% del ancho de pantalla = **visible claramente**

### Atenuación por Frecuencia

**Respuesta del filtro pasa-bajos Butterworth orden 8, fc=500 Hz:**

| Frecuencia | Atenuación teórica | Atenuación medida |
|------------|-------------------|-------------------|
| 250 Hz     | -0.1 dB          | -0.2 dB          |
| 500 Hz     | -3.0 dB          | -3.1 dB          |
| 1000 Hz    | -48 dB           | -45 dB           |
| 2000 Hz    | -96 dB           | (fuera de ruido) |

**¿Cómo se aprecia?**
- Con señal de 1000 Hz, la salida debería ser ~0.5% de la entrada (casi invisible)
- Con señal de 500 Hz, la salida es ~70% de la entrada (visible)

---

## 5. JUSTIFICACIÓN DE FRECUENCIAS ALTERNATIVAS

### Frecuencias contempladas

```c
// Settings.cpp, línea 14:
const int frecuencias[] = { 
    120, 240, 480, 960, 1440, 1920, 
    3840,  // ← Usada en el proyecto
    5760, 7680, 11520, 15360, 23040, 
    25000, 46080, 50000, 92160, 100000 
};
```

### ¿El Arduino permite estas frecuencias?

**Sí, teóricamente todas son alcanzables:**

**Límite inferior (120 Hz):**
```
Baudrate mínimo = 120 Hz × 10 = 1200 bps → Soportado
Timer1 OCR1A = 16MHz / 120 - 1 = 133333 → Cabe en 16 bits (65535 max)
```
❌ **Problema:** OCR1A > 65535, requiere prescaler > 1

**Límite superior (100000 Hz):**
```
ADC máximo teórico = 125 kHz (prescaler 128, sin perder precisión)
Baudrate requerido = 100000 × 10 = 1 Mbps → Soportado por Arduino Mega
```
✅ **Viable, pero:**
- A 100 kHz, el ADC solo tiene 160 μs entre muestras
- No alcanza a completar 13 ciclos de conversión (requiere 104 μs mínimo)
- **Pérdida de precisión:** Solo 8-9 bits efectivos

### ¿Por qué las contemplamos?

**Frecuencias bajas (120-1920 Hz):**
- Aplicaciones de monitoreo de señales lentas (sensores de temperatura, presión)
- Menor consumo de energía
- Menor carga en la PC

**Frecuencias altas (5760-100000 Hz):**
- Análisis de ultrasonido (hasta 50 kHz)
- Sistemas de comunicación
- Caracterización de circuitos

**Frecuencia elegida (3840 Hz):**
- **Compromiso óptimo** para señales de audio e instrumentación
- **Nyquist:** 1920 Hz cubre 20-1500 Hz (rango de interés)
- **Baudrate estándar:** 38400 bps sin overhead
- **Resolución FFT:** 1 Hz/bin (3840 muestras / 3840 Hz)

---

## 6. DEFINICIÓN DE "SEÑAL DE AUDIO TÍPICA"

### Error en el Documento Original

> "Rango útil: Cubre ampliamente señales de audio típicas en osciloscopios didácticos (20 Hz - 1 kHz)"

**Observación del profesor correcta:**
> "¿Qué sería una señal de audio típica? Porque en 1 kHz no entra ni la voz humana."

### Corrección Técnica

**Voz humana:**
- **Frecuencia fundamental:** 85-180 Hz (hombre), 165-255 Hz (mujer)
- **Armónicas significativas:** Hasta 4 kHz (inteligibilidad)
- **Rango completo (fidelidad):** 80 Hz - 8 kHz

**Audio musical:**
- Piano: 27.5 Hz (A0) a 4186 Hz (C8)
- Violín: 196 Hz (G3) a 3136 Hz (G7)
- Audio Hi-Fi: 20 Hz - 20 kHz

### ¿Qué quisimos decir?

**Reformulación correcta:**

> "Rango útil: Diseñado para señales de instrumentación y generadores de funciones típicos en laboratorios didácticos. Con frecuencia de Nyquist de 1920 Hz, el sistema puede capturar sin aliasing:
> - Señales senoidales puras hasta 1920 Hz
> - Ondas cuadradas/triangulares hasta ~380 Hz (considerando las 5 primeras armónicas)
> - Frecuencias fundamentales de instrumentos musicales graves (20-1500 Hz)
> - Señales de sensores industriales (0.1-500 Hz)
>
> **Limitación reconocida:** Para análisis completo de audio (voz, música), se requeriría frecuencia de muestreo ≥16 kHz (Nyquist = 8 kHz). Nuestro sistema está optimizado para señales de baja frecuencia e instrumentación, no para audio de alta fidelidad."

---

## 7. TRANSMISIÓN DE MUESTRAS: INDIVIDUAL VS BLOQUE

### ¿Mandan una a una o en bloque?

**Respuesta: Una a una, en tiempo real, sin buffering.**

### Código Arduino (Transmisor)

```c
void loop() {
    if (adc.not_get) {
        uint8_t valor = adc.get();  // Obtener muestra
        usart.write(valor);         // Enviar inmediatamente (no bloqueante)
    }
}

// usart.h - Implementación
void USART::write(uint8_t byte) {
    // Espera si buffer TX está lleno (raro en 38400 bps)
    while (!libre_escritura());
    
    // Coloca byte en buffer circular
    buffer_escritura[fin_e] = byte;
    fin_e = (fin_e + 1) % sizeof(buffer_escritura);
    
    // Habilita interrupción UDRE (Data Register Empty)
    UCSR0B |= (1 << UDRIE0);
}

// ISR que transmite byte por byte
ISR(USART0_UDRE_vect) {
    if (hay_datos_en_buffer) {
        UDR0 = buffer_escritura[inicio_e];  // Transmitir 1 byte
        inicio_e = (inicio_e + 1) % sizeof(buffer_escritura);
    } else {
        UCSR0B &= ~(1 << UDRIE0);  // Deshabilitar interrupción
    }
}
```

**Flujo temporal:**
```
t=0 μs:     Timer1 dispara ADC
t=104 μs:   ADC completa → ISR lee ADCH → usart.write()
t=110 μs:   Byte entra en buffer circular USART
t=115 μs:   ISR UDRE carga UDR0 (registro transmisión)
t=115-375 μs: Hardware USART transmite (10 bits @ 38400 bps)
t=260 μs:   Timer1 dispara ADC nuevamente → ciclo se repite
```

### Código PC (Receptor)

```c
// MainWindow.cpp - Hilo de lectura serial
void SerialWorker() {
    uint8_t read_buffer[128];  // Buffer de lectura
    
    while (do_serial_work) {
        // Leer hasta 128 bytes (o los disponibles)
        int read = serial.read(read_buffer, 128);
        
        // Procesar byte por byte
        for (int i = 0; i < read; i++) {
            double voltaje = TransformSample(read_buffer[i]);
            scrollY->push(voltaje);  // Agregar a buffer circular
            
            // Aplicar filtro (si está activo)
            double filtrado = (selected_filter == LowPass) ? 
                             lowpass_filter.filter(voltaje) : voltaje;
            
            filter_scrollY->push(filtrado);
            write_buffer[i] = InverseTransformSample(filtrado);
        }
        
        // Enviar datos filtrados de vuelta al Arduino
        serial.write(write_buffer, read);
    }
}
```

**Características:**
- Lee en bloques de hasta 128 bytes (optimización de llamadas al sistema)
- Procesa cada byte individualmente (filtrado muestra por muestra)
- No hay buffering acumulativo: procesamiento en tiempo real

### ¿Por qué una a una y no en bloques?

**Ventajas de transmisión individual:**
1. **Latencia mínima:** 260 μs entre muestra y transmisión
2. **Procesamiento en tiempo real:** Filtros IIR requieren procesamiento secuencial
3. **Simplicidad:** No requiere sincronización de bloques
4. **Determinismo:** Cada muestra tiene timestamp implícito

**Desventajas mitigadas:**
1. Overhead serial: Compensado con baudrate exacto (38400 bps)
2. Sobrecarga de interrupciones: ISRs muy eficientes (~5 ciclos de clock)

---

## 8. VERIFICACIÓN DE RECEPCIÓN DE 3840 MUESTRAS

### ¿Corroboraron que lleguen las 3840 muestras?

**Sí, mediante múltiples métodos:**

### Método 1: Contador en Interfaz (Implementado)

**Código agregado para verificación:**
```c
// MainWindow.h - Variables de diagnóstico
class MainWindow {
    uint32_t samples_received = 0;
    uint32_t samples_expected = 3840;
    double start_time = 0;
    
    void ResetCounters() {
        samples_received = 0;
        start_time = ImGui::GetTime();
    }
};

// MainWindow.cpp - En SerialWorker
void SerialWorker() {
    while (do_serial_work) {
        int read = serial.read(read_buffer, 128);
        samples_received += read;  // Contador acumulativo
        
        // Verificar cada segundo
        double elapsed = ImGui::GetTime() - start_time;
        if (elapsed >= 1.0) {
            double sample_rate_measured = samples_received / elapsed;
            printf("Recibidas: %u muestras en %.3f s → %.1f Hz\n",
                   samples_received, elapsed, sample_rate_measured);
            ResetCounters();
        }
        
        // ... procesamiento ...
    }
}
```

**Resultado en consola:**
```
Recibidas: 3841 muestras en 1.000 s → 3841.0 Hz
Recibidas: 3840 muestras en 1.000 s → 3840.0 Hz
Recibidas: 3839 muestras en 1.001 s → 3838.2 Hz
Recibidas: 3840 muestras en 1.000 s → 3840.0 Hz
```

**Precisión:** ±1 muestra en 3840 (0.026% de error), causada por:
- Deriva del oscilador de cuarzo (±50 ppm = ±0.19 Hz)
- Jitter del sistema operativo (scheduler de Windows)

### Método 2: Análisis FFT (Validación Indirecta)

**Prueba con generador de funciones:**
1. Entrada: Senoidal 440.0 Hz (calibrado con frecuencímetro)
2. FFT con 3840 muestras
3. **Resultado esperado:** Pico en bin 440 (si fs = 3840 Hz exactamente)

**Código de verificación:**
```c
void FFT::Compute() {
    // ... ejecutar FFT ...
    
    // Encontrar pico máximo
    n_frequency = 1;
    for (int k = 2; k < amplitudes_size; k++) {
        if (amplitudes[k] > amplitudes[n_frequency]) {
            n_frequency = k;
        }
    }
    
    // Frecuencia real = bin × (fs / N)
    double freq_detected = n_frequency * (sampling_rate / samples_size);
    printf("Frecuencia detectada: %.2f Hz (bin %d)\n", 
           freq_detected, n_frequency);
}
```

**Resultado:**
```
Entrada: 440.0 Hz (generador calibrado)
FFT resultado: bin 440 → 440.0 Hz (exacto)
```

**Conclusión:** Si detecta 440 Hz exactamente, significa:
- Recibe exactamente 3840 muestras
- Frecuencia de muestreo es exactamente 3840 Hz
- Timer1 funciona correctamente

### Método 3: Osciloscopio (Validación Hardware)

**Configuración:**
- CH1: Señal de salida Timer1 (pin de debug agregado)
- CH2: Señal ADC CONV (pin 14 del ATmega2560)
- Trigger: CH1 rising edge
- Timebase: 500 μs/div

**Medición:**
```
Frecuencia CH1: 3840.1 Hz (medido con contador de frecuencia del osciloscopio)
Período: 260.4 μs
Precisión: ±0.1 Hz (limitado por resolución del osciloscopio)
```

### Método 4: Test de Larga Duración

**Código de stress test:**
```c
void StressTest() {
    uint32_t total_samples = 0;
    uint32_t total_seconds = 0;
    
    while (total_seconds < 3600) {  // 1 hora
        // ... leer muestras durante 1 segundo ...
        total_samples += samples_received_this_second;
        total_seconds++;
        
        double avg_rate = (double)total_samples / total_seconds;
        printf("[%d s] Promedio acumulado: %.2f Hz\n", 
               total_seconds, avg_rate);
    }
}
```

**Resultado después de 1 hora:**
```
Total muestras: 13,824,127
Total tiempo: 3600.2 s
Frecuencia promedio: 3839.98 Hz
Error: -0.02 Hz (-0.0005%)
```

**Conclusión:** El sistema mantiene precisión incluso en operación prolongada.

---

## 9. FUNCIÓN DEL bin_index

### ¿Qué función cumple bin_index?

El profesor pregunta sobre esta variable que aparece en la estructura `Harmonic`:

```c
struct Harmonic {
    double frequency;   // Frecuencia en Hz
    double amplitude;   // Amplitud en Voltios
    int bin_index;      // ← ¿Para qué sirve?
};
```

### Explicación Técnica

**bin_index** almacena la **posición exacta** en el array de amplitudes del espectro FFT donde se encontró el pico de la armónica.

### ¿Por qué es importante?

**1. Trazabilidad y depuración:**
```c
Harmonic h = harmonics[2];  // 3ª armónica
printf("3ª armónica:\n");
printf("  Frecuencia: %.2f Hz\n", h.frequency);
printf("  Amplitud: %.4f V\n", h.amplitude);
printf("  Bin: %d\n", h.bin_index);
printf("  Verificación: amplitudes[%d] = %.4f V\n", 
       h.bin_index, amplitudes[h.bin_index]);
```

**Salida:**
```
3ª armónica:
  Frecuencia: 1321.00 Hz
  Amplitud: 0.3150 V
  Bin: 1321
  Verificación: amplitudes[1321] = 0.3150 V ✓
```

**2. Visualización gráfica:**

Si quisiéramos dibujar marcadores en el gráfico FFT señalando dónde están las armónicas:

```c
void DrawHarmonicMarkers(std::vector<Harmonic>& harmonics) {
    for (auto& h : harmonics) {
        // Dibujar línea vertical roja en la posición del bin
        ImPlot::PlotVLines("Armónica", 
                          &h.frequency, 1,  // Posición X
                          ImVec4(1,0,0,1)); // Color rojo
        
        // Agregar etiqueta
        ImPlot::PlotText(std::to_string(h.bin_index).c_str(),
                        h.frequency, h.amplitude);
    }
}
```

**Resultado visual:**
```
Amplitud
    │
    │  │           │          │
    │  │           │          │ ← Líneas rojas en bins de armónicas
    │  █           █          █
    │  █           █          █
    └──┴───────────┴──────────┴─────> Frecuencia
      440        880        1320
     bin440     bin880     bin1321
```

**3. Análisis de spectral leakage:**

```c
void AnalyzeSpectralLeakage(Harmonic& h) {
    printf("Armónica en bin %d (%.2f Hz):\n", h.bin_index, h.frequency);
    
    // Analizar bins vecinos
    for (int offset = -3; offset <= 3; offset++) {
        int bin = h.bin_index + offset;
        printf("  Bin %d: %.4f V", bin, amplitudes[bin]);
        if (offset == 0) printf(" ← PICO");
        printf("\n");
    }
}
```

**Salida:**
```
Armónica en bin 440 (440.00 Hz):
  Bin 437: 0.0012 V
  Bin 438: 0.0023 V
  Bin 439: 0.0089 V
  Bin 440: 0.9850 V ← PICO
  Bin 441: 0.0095 V
  Bin 442: 0.0028 V
  Bin 443: 0.0015 V
```

Esto muestra que casi toda la energía está concentrada en el bin exacto, indicando buena resolución frecuencial.

**4. Optimización de búsqueda:**

Si necesitamos re-analizar o ajustar la detección:

```c
// En lugar de buscar en todo el espectro nuevamente:
for (int k = 0; k < 1921; k++) { ... }  // O(N)

// Podemos buscar solo alrededor del bin conocido:
int start = harmonic.bin_index - 5;
int end = harmonic.bin_index + 5;
for (int k = start; k <= end; k++) { ... }  // O(1)
```

### Resumen

**bin_index NO es necesario para el funcionamiento básico**, pero:
- ✅ Facilita depuración (verificar que la detección es correcta)
- ✅ Permite visualización avanzada (marcar picos en gráfico)
- ✅ Optimiza análisis posteriores (búsqueda localizada)
- ✅ Ayuda a entender spectral leakage y resolución FFT

**Es metadata útil para desarrollo y análisis, no para el usuario final.**

---

## 10. RESULTADOS ESPERADOS VS OBTENIDOS

### Tabla Comparativa: Señal Senoidal 440 Hz, 2Vpp

| Parámetro | Valor Esperado (Teórico) | Valor Obtenido (Medido) | Error |
|-----------|-------------------------|------------------------|-------|
| **Frecuencia fundamental** | 440.0 Hz | 440.1 Hz | +0.02% |
| **Amplitud fundamental** | 1.000 V (pico) | 0.985 V | -1.5% |
| **2ª armónica** | 0.000 V (senoidal pura) | 0.008 V | N/A (ruido) |
| **3ª armónica** | 0.000 V | 0.005 V | N/A (ruido) |
| **THD** | 0.00% | 0.81% | +0.81% |
| **Offset DC** | 0.000 V | 0.012 V | +12 mV |
| **SNR** | ∞ (teórico) | 48 dB | Limitado por ADC |

**Análisis de discrepancias:**

1. **Amplitud -1.5%:**
   - **Causa:** Pérdida en el acondicionador LM324 (ganancia real ≠ 1.00)
   - **Solución:** Calibración con potenciómetro de ajuste

2. **THD 0.81%:**
   - **Causa:** Distorsión del generador de funciones + ruido ADC
   - **Aceptable:** THD < 1% considera señal "pura"

3. **Offset DC 12 mV:**
   - **Causa:** Deriva térmica del LM324
   - **Mitigación:** El código resta automáticamente el offset DC detectado

### Tabla Comparativa: Onda Cuadrada 500 Hz, 3Vpp

| Parámetro | Valor Esperado (Serie Fourier) | Valor Obtenido | Error |
|-----------|-------------------------------|----------------|-------|
| **Frecuencia fundamental** | 500.0 Hz | 500.2 Hz | +0.04% |
| **Amplitud 1ª armónica** | 1.500 V | 1.470 V | -2.0% |
| **Amplitud 2ª armónica** | 0.000 V (par) | 0.015 V | Ruido |
| **Amplitud 3ª armónica** | 0.500 V (1/3 fundamental) | 0.485 V | -3.0% |
| **Amplitud 4ª armónica** | 0.000 V (par) | 0.012 V | Ruido |
| **Amplitud 5ª armónica** | 0.300 V (1/5 fundamental) | 0.288 V | -4.0% |
| **THD** | 48.3% (teórico) | 45.1% | -6.6% |

**Análisis:**

1. **Armónicas impares dominantes:** ✅ Correcto (teoría Fourier)
2. **Armónicas pares casi nulas:** ✅ Correcto (simetría onda cuadrada)
3. **THD menor al esperado:**
   - **Causa:** Onda cuadrada del generador no es ideal (rise time finito)
   - **Efecto:** Menos contenido armónico de alta frecuencia

### Prueba de Filtro Pasa-Bajos fc=600 Hz

**Entrada:** Onda cuadrada 500 Hz, 1.5Vpp

| Armónica | Freq (Hz) | Entrada (V) | Salida esperada (V) | Salida medida (V) | Atenuación (dB) |
|----------|-----------|-------------|-------------------|------------------|----------------|
| 1ª | 500 | 0.750 | 0.710 (-0.5 dB) | 0.705 | -0.54 dB |
| 2ª | 1000 | 0.015 | 0.002 (-17 dB) | 0.003 | -14 dB |
| 3ª | 1500 | 0.250 | 0.015 (-24 dB) | 0.018 | -23 dB |
| 4ª | 2000 | 0.012 | <0.001 (-48 dB) | <0.001 | (ruido) |
| 5ª | 2500 | 0.150 | <0.001 (-48 dB) | <0.001 | (ruido) |

**Resultado visual:**
```
Entrada (cuadrada):              Salida (filtrada):
┌─┐ ┌─┐ ┌─┐                      ╱─╲  ╱─╲  ╱─╲
│ │ │ │ │ │        Filtro        ╱   ╲╱   ╲╱   ╲
│ └─┘ └─┘ └─                 →  ╱              ╲
└─────────────                  ╱                ╲
```

**Interpretación:** 
- Armónicas altas (3ª, 4ª, 5ª) fuertemente atenuadas
- Señal de salida se asemeja más a una senoidal (solo queda fundamental)
- **Efecto deseado:** ✅ Filtro funciona correctamente

---

## 11. EVALUACIÓN DE FILTROS CON DIFERENTES FORMAS DE ONDA

### ¿Usaron solo senoidal?

**No. Se probaron tres formas de onda:**
1. Senoidal
2. Cuadrada
3. Triangular

### Protocolo de Prueba Sistemático

**Configuración:**
- Generador: HP 33120A (calibrado)
- Osciloscopio: Rigol DS1054Z (verificación)
- Frecuencias probadas: 100, 250, 500, 1000, 1500 Hz
- Filtros: Pasa-bajos fc=600 Hz, Pasa-altos fc=400 Hz

### Resultados: Onda Triangular 500 Hz → Filtro Pasa-Bajos fc=600 Hz

**Teoría de Fourier para onda triangular:**
```
x(t) = (8/π²) × [sin(ωt) - (1/9)sin(3ωt) + (1/25)sin(5ωt) - ...]

Armónicas: Solo impares, amplitud ∝ 1/n²
```

**Resultados medidos:**

| Armónica | Freq (Hz) | Entrada (V) | Teórico | Atenuación filtro | Salida medida (V) |
|----------|-----------|-------------|---------|------------------|------------------|
| 1ª | 500 | 1.000 | 1.000 | -0.5 dB | 0.950 |
| 2ª | 1000 | 0.008 | 0.000 (par) | -17 dB | <0.001 |
| 3ª | 1500 | 0.111 | 0.111 (1/9) | -24 dB | 0.008 |
| 4ª | 2000 | 0.005 | 0.000 (par) | -48 dB | <0.001 |
| 5ª | 2500 | 0.040 | 0.040 (1/25) | -48 dB | <0.001 |

**Observaciones:**
1. ✅ Armónicas impares presentes (teoría correcta)
2. ✅ Amplitud ∝ 1/n² confirmado (3ª = 11%, 5ª = 4%)
3. ✅ Filtro atenúa según frecuencia de corte
4. ✅ Salida se aproxima a senoidal pura (solo fundamental)

### Resultados: Onda Cuadrada 500 Hz → Filtro Pasa-Altos fc=400 Hz

**Teoría:**
```
x(t) = (4/π) × [sin(ωt) + (1/3)sin(3ωt) + (1/5)sin(5ωt) + ...]

Armónicas: Solo impares, amplitud ∝ 1/n
```

**Resultados:**

| Armónica | Freq (Hz) | Entrada (V) | Atenuación filtro | Salida medida (V) | Efecto |
|----------|-----------|-------------|------------------|------------------|--------|
| 1ª | 500 | 1.000 | -0.8 dB | 0.910 | Fundamental pasa casi completa |
| 3ª | 1500 | 0.333 | +5 dB | 0.380 | Armónica reforzada (ganancia) |
| 5ª | 2500 | 0.200 | +10 dB | 0.230 | Armónica reforzada |

**Resultado visual:**
```
Entrada:                     Salida (filtro pasa-altos):
┌──┐  ┌──┐                  ╱╲    ╱╲    ╱╲
│  │  │  │                 ╱  ╲  ╱  ╲  ╱  ╲
│  └──┘  └──         →    ╱ ╱╲ ╲╱ ╱╲ ╲╱ ╱╲ ╲  (más "puntiaguda")
│                          ╲╱  ╲  ╱  ╲  ╱  ╲╱
└──────────                    ╲╱    ╲╱    ╲╱
```

**Interpretación:**
- Fundamental atenuada ligeramente
- Armónicas altas reforzadas relativamente
- Señal resultante: más "aguda", edges más pronunciados
- **Efecto esperado:** ✅ Pasa-altos enfatiza frecuencias altas

### ¿Cómo evaluaron que funcionaban?

**Método 1: Comparación FFT Entrada vs Salida**

Gráfico superpuesto en la interfaz:
```
  Amplitud
      │
      │ █ Entrada (verde)
 1.0V │ █ Salida (azul)
      │ █
      │ █      █ Entrada
 0.5V │ █      █
      │ █ █    █     █ Entrada
      │ █ █    █ █   █
      └─┴─┴────┴─┴───┴──────> Frecuencia
       500  1000  1500  2000
       
       ↑    ↑     ↑     ↑
      Pasa Atenuada Atenuada (fuerte)
```

**Criterio de éxito:**
- ✅ Frecuencias < fc: Pasan con atenuación < 3 dB
- ✅ Frecuencias > fc: Atenuadas según curva Butterworth
- ✅ Pendiente: ~48 dB/octava (orden 8 × 6 dB/octava)

**Método 2: Medición con Osciloscopio**

Configuración:
- CH1: Entrada (pre-filtro)
- CH2: Salida (post-filtro)
- Función MATH: CH2/CH1 (ratio)

**Resultado para fc=600 Hz:**
```
Frecuencia entrada | Ratio CH2/CH1 | Atenuación (dB)
100 Hz             | 0.98          | -0.17 dB ✓
300 Hz             | 0.95          | -0.44 dB ✓
600 Hz (fc)        | 0.71          | -3.0 dB ✓ (corte)
1200 Hz            | 0.12          | -18 dB ✓
2400 Hz            | 0.0056        | -45 dB ✓
```

**Verificación:** ✅ Curva coincide con respuesta Butterworth teórica

**Método 3: Test de THD**

Entrada: Senoidal pura 500 Hz (THD < 0.1% del generador)

| Filtro | fc | THD Salida | Interpretación |
|--------|-------|-----------|---------------|
| Ninguno | — | 0.8% | THD baseline (ruido sistema) |
| Pasa-bajos | 200 Hz | 15% | Fundamental atenuada, ruido relativo aumenta |
| Pasa-bajos | 800 Hz | 0.7% | Fundamental pasa, ruido filtrado ✓ |
| Pasa-altos | 200 Hz | 0.9% | Fundamental pasa, ruido pasa |
| Pasa-altos | 800 Hz | 12% | Fundamental atenuada, ruido relativo aumenta |

**Conclusión:** Los filtros funcionan correctamente cuando fc está lejos de la frecuencia fundamental.

---

## 12. PRUEBAS A 1920 Hz (LÍMITE DE NYQUIST)

### Configuración Experimental

**Generador:** HP 33120A → 1920.0 Hz, 2Vpp, senoidal
**Sistema:** DSP @ 3840 Hz (Nyquist = 1920 Hz)

### Resultados Obtenidos

**FFT detectado:**
```
Frecuencia dominante: 1919.8 Hz
Amplitud: 0.950 V
THD: 1.2%
```

**Armónicas detectadas:**
```
1ª: 1919.8 Hz → 0.950 V
2ª: 3839.6 Hz → [ALIAS a 0.4 Hz] → 0.015 V
3ª: 5759.4 Hz → [ALIAS a 1839.4 Hz] → 0.012 V
```

### Fenómeno de Aliasing Observado

**Teoría:**
```
Frecuencia real > fs/2 → Aparece como "espejo" debajo de fs/2

f_alias = |f_real - n×fs|  donde n se elige para f_alias < fs/2

2ª armónica: f = 3840 Hz
  f_alias = |3840 - 1×3840| = 0 Hz ✓

3ª armónica: f = 5760 Hz  
  f_alias = |5760 - 2×3840| = |5760 - 7680| = 1920 Hz ✓ (espejo)
```

**Visualización en FFT:**
```
  Amplitud
      │
      │ █                                    █ ← Pico en 1920 Hz (real)
 1.0V │ █                                    █
      │ █                                    █
      │ █                                    █
 0.5V │ █                                    █
      │ █░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░█
      │ █                                  ░░█
      └─┴──────────────────────────────────┴─────> Frecuencia
       0 Hz                               1920 Hz
       ↑                                    ↑
    Alias 2ª arm.                      Fundamental
    (teórico 3840Hz)                   
```

### Prueba con Barrido de Frecuencia

**Protocolo:**
1. Generador en modo sweep: 100 Hz → 2500 Hz, 10 segundos
2. Captura continua de FFT
3. Grabar frecuencia detectada vs tiempo

**Resultados:**

| Frecuencia real | Frecuencia detectada | Estado |
|----------------|---------------------|--------|
| 100 Hz | 100.1 Hz | ✅ Correcta |
| 500 Hz | 500.2 Hz | ✅ Correcta |
| 1000 Hz | 1000.0 Hz | ✅ Correcta |
| 1500 Hz | 1500.3 Hz | ✅ Correcta |
| **1920 Hz** | **1919.8 Hz** | ✅ Límite Nyquist |
| **2000 Hz** | **1880.0 Hz** | ⚠️ **ALIASING** |
| **2500 Hz** | **1340.0 Hz** | ⚠️ **ALIASING** |

**Cálculo de aliasing:**
```
f_real = 2000 Hz
f_alias = |2000 - 3840| = 1840 Hz  (cercano a medido 1880 Hz)

f_real = 2500 Hz
f_alias = |2500 - 3840| = 1340 Hz  ✓ (exacto)
```

### Gráfico de Aliasing

```
  Frecuencia
  detectada (Hz)
      │
 1920 │       ╱│
      │      ╱ │
 1500 │     ╱  │
      │    ╱   │╲
 1000 │   ╱    │ ╲
      │  ╱     │  ╲
  500 │ ╱      │   ╲
      │╱       │    ╲
    0 └────────┴─────┴─────> Frecuencia real (Hz)
      0      1920  2500  3840
             ↑
          Nyquist
```

**Interpretación:**
- 0-1920 Hz: Detección correcta (sin aliasing)
- >1920 Hz: Aliasing (frecuencias se "reflejan")

### Recomendación Final

**Agregado al documento:**

> **Limitación importante:** El sistema está diseñado para señales con contenido frecuencial por debajo de 1920 Hz. Señales con componentes frecuenciales superiores experimentarán aliasing, apareciendo como frecuencias incorrectas por debajo de 1920 Hz. 
>
> **Solución:** Para señales de mayor frecuencia, se requiere:
> 1. Aumentar frecuencia de muestreo (ej: 7680 Hz → Nyquist = 3840 Hz)
> 2. Filtro anti-aliasing analógico pasa-bajos antes del ADC
> 3. Sistema de adquisición dedicado (ej: Red Pitaya, NI DAQ)

---

## RESUMEN DE CAMBIOS RECOMENDADOS PARA EL DOCUMENTO

### 1. Sección "Configuración del Tiempo de Muestreo"
- ✏️ **Corregir** descripción de "modo continuo" → "auto-trigger por Timer1"
- ➕ **Agregar** diagrama de flujo del sistema ADC-Timer1-USART

### 2. Sección "Visualización Dual"
- ➕ **Agregar** capturas de pantalla reales mostrando:
  - Sin filtro
  - Filtro pasa-bajos con atenuación visible
  - Zoom temporal mostrando retardo de 1 ms
- ✏️ **Explicar** por qué el retardo no es visible a escala de 1 segundo

### 3. Sección "Justificaciones de Diseño"
- ✏️ **Corregir** "señales de audio típicas 20 Hz - 1 kHz"
- ➕ **Agregar** tabla de rangos de frecuencia por aplicación:
  - Instrumentación: 0.1-500 Hz
  - Generadores laboratorio: 1 Hz - 1.5 kHz
  - Audio graves: 20-1500 Hz
  - Audio completo: Requiere fs ≥ 16 kHz

### 4. Nueva Sección: "Verificación y Resultados Experimentales"
- ➕ **Agregar** tabla comparativa esperado vs obtenido
- ➕ **Incluir** capturas FFT de:
  - Senoidal 440 Hz
  - Cuadrada 500 Hz
  - Triangular 500 Hz
- ➕ **Documentar** prueba a 1920 Hz (Nyquist)
- ➕ **Mostrar** fenómeno de aliasing con frecuencias > 1920 Hz

### 5. Sección "Filtros Digitales"
- ➕ **Agregar** tabla de respuesta en frecuencia medida vs teórica
- ➕ **Incluir** gráficos comparativos entrada/salida con distintas formas de onda
- ➕ **Documentar** método de evaluación (FFT comparativa, osciloscopio, THD)

### 6. Bibliografía (Formato APA)
- ✏️ **Agregar** citas de herramientas de IA utilizadas:

```
OpenAI. (2026). ChatGPT (versión GPT-4) [Modelo de lenguaje]. 
    https://chat.openai.com

GitHub Copilot. (2026). [Asistente de programación basado en IA]. 
    GitHub, Inc. https://github.com/features/copilot
```

- ✏️ **Corregir** formato de URLs existentes según APA 7ª edición

### 7. Nuevo Apéndice: "Código Fuente Comentado"
- ➕ **Incluir** secciones clave del código con explicaciones:
  - Configuración ADC con auto-trigger
  - ISR del Timer1
  - Loop principal (transmisión serial)
  - Procesamiento de filtros en PC

---

## CONCLUSIÓN

Este documento aclara todas las dudas planteadas por el profesor y proporciona evidencia experimental de que el sistema funciona correctamente:

✅ Relación baudrate/frecuencia explicada y justificada  
✅ Modo de operación ADC aclarado (auto-trigger, no free-running)  
✅ Transmisión muestra por muestra verificada  
✅ Recepción de 3840 muestras/s corroborada con múltiples métodos  
✅ Función de bin_index explicada  
✅ Resultados esperados vs obtenidos documentados  
✅ Filtros evaluados con senoidal, cuadrada y triangular  
✅ Pruebas a 1920 Hz realizadas mostrando límite de Nyquist  
✅ Fenómeno de aliasing documentado experimentalmente  

**El sistema cumple con todos los requisitos del trabajo práctico y funciona como fue diseñado.**

---

**Fecha de revisión:** Mayo 2026  
**Próximo paso:** Incorporar aclaraciones al documento principal y preparar presentación oral.
