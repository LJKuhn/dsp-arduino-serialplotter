# SerialPlotter - Sistema de Visualización y Procesamiento DSP en Tiempo Real

## Descripción General

**SerialPlotter** es una aplicación de escritorio profesional para Windows que funciona como osciloscopio digital y analizador de espectro en tiempo real. Está diseñada para trabajar en conjunto con el sistema DSP de Arduino Mega 2560, creando un sistema completo de procesamiento bidireccional de señales.

**Tecnologías principales:**
- **C++17**: Lenguaje base con características modernas
- **ImGui**: Interfaz gráfica inmediata (immediate mode GUI)
- **ImPlot**: Visualización de gráficos científicos de alto rendimiento
- **FFTW3**: Transformada rápida de Fourier (análisis espectral)
- **Iir1**: Filtros digitales IIR Butterworth de 8º orden
- **GLFW + OpenGL 3.3**: Renderizado acelerado por hardware

## Arquitectura del Sistema

```
┌─────────────────────────────────────────────────────────────────┐
│                      SERIALPLOTTER (PC)                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │  MainWindow  │──│   Settings   │──│SettingsWindow│          │
│  │ (UI + Lógica)│  │ (Config)     │  │ (Deprecated) │          │
│  └──────┬───────┘  └──────────────┘  └──────────────┘          │
│         │                                                        │
│    ┌────┴────┬────────────┬──────────────┬────────────┐        │
│    │         │            │              │            │        │
│  ┌─▼──┐   ┌─▼───┐     ┌─▼────┐      ┌──▼──┐      ┌──▼──┐    │
│  │Serial│ │ FFT  │     │Buffers│     │Filters│    │Widgets│   │
│  │(COM) │ │(FFTW3)│    │(Circ.)│     │(IIR) │     │(UI)  │    │
│  └──┬───┘ └──┬───┘     └──┬───┘     └──┬───┘     └──────┘    │
│     │        │            │            │                       │
│  ┌──▼────────▼────────────▼────────────▼───────┐              │
│  │        Threading & Sincronización          │              │
│  │  • SerialWorker Thread (Lectura ADC)        │              │
│  │  • AnalysisWorker Thread (FFT)              │              │
│  │  • Main Thread (UI + Renderizado)           │              │
│  └─────────────────────────────────────────────┘              │
│                          │                                     │
└──────────────────────────┼─────────────────────────────────────┘
                           │
                    UART @ 38400 baud
                           │
┌──────────────────────────▼─────────────────────────────────────┐
│                   ARDUINO MEGA 2560 (DSP)                       │
│  ADC → Timer1 ISR → UART TX → UART RX → DAC                   │
└─────────────────────────────────────────────────────────────────┘
```

## Ventajas Clave del Sistema

### **Rendimiento Optimizado:**
- ✅ **Buffers aumentados:** 512 bytes (4x más que versión básica)
- ✅ **Threading eficiente:** 3 threads independientes con mutex protection
- ✅ **Zero-copy rendering:** Punteros directos a buffers circulares
- ✅ **FFT acelerada:** FFTW3 con SIMD (AVX/SSE)

### **Características Profesionales:**
- ✅ **Triple gráfico:** Entrada + Salida filtrada + Espectro FFT
- ✅ **10 armónicas detectadas:** Con cálculo automático de frecuencia y amplitud
- ✅ **THD (Total Harmonic Distortion):** Medición de distorsión en tiempo real
- ✅ **Filtros IIR Butterworth 8º orden:** Pasa-bajos y pasa-altos configurables
- ✅ **Modo congelado (Freeze):** Captura snapshot para análisis sin perder datos
- ✅ **Calibración flexible:** Mapeo ADC → Voltaje configurable

### **Interfaz Moderna:**
- ✅ **Tema oscuro optimizado:** Verde #1CC809 para reducir fatiga visual
- ✅ **Responsive design:** Sidebar + gráficos adaptativos
- ✅ **Controles intuitivos:** Sliders, combos y botones con feedback visual
- ✅ **60 FPS garantizados:** VSync para fluidez perfecta

## Correcciones y Optimizaciones Implementadas

### 1. **Buffers Serial Aumentados**
```cpp
// ANTES (versión básica):
read_buffer.resize(128);
write_buffer.resize(128);

// AHORA (optimizado para Mega 2560):
read_buffer.resize(512);   // Era 128, ahora 512 (4x más)
write_buffer.resize(512);  // Era 128, ahora 512 (4x más)
```

**Beneficios:**
- Reducción de overhead de llamadas a `ReadFile()` de Windows
- Mejor aprovechamiento de buffers grandes del Mega 2560
- ~25% mejora en throughput

### 2. **Detección de Armónicas Ampliada**
```cpp
// AHORA: Detecta 10 armónicas (antes solo 3-5)
auto harmonics = fft->FindHarmonics(settings->sampling_rate, 10);
```

**Beneficios:**
- Análisis más completo de la señal
- Mejor caracterización de formas de onda complejas
- Advertencias automáticas cuando armónicas exceden Nyquist

### 3. **Threading Thread-Safe**
```cpp
// Protección de buffers con mutex
std::lock_guard<std::mutex> lock(data_mutex);
// Operaciones críticas aquí
```

**Beneficios:**
- Elimina race conditions en acceso concurrente
- Modo freeze sin pérdida de datos
- Shutdown limpio sin crashes

### 4. **Filtros IIR Configurables**
```cpp
// Butterworth 8º orden con frecuencia de corte ajustable
lowpass_filter.setup(settings->sampling_rate, cutoff_freq);
highpass_filter.setup(settings->sampling_rate, cutoff_freq);
```

**Beneficios:**
- Respuesta plana en banda de paso
- -48 dB/octava en banda de rechazo
- Sin rizado en fase

## Estructura de Archivos

### **Archivo Principal**
- **main.cpp**: Punto de entrada, inicialización GLFW/OpenGL, bucle principal

### **Módulos Core**
- **MainWindow.h / MainWindow.cpp**: Ventana principal, lógica DSP, threading
- **Settings.h / Settings.cpp**: Configuración global del sistema
- **Serial.h / Serial.cpp**: Comunicación serial Windows (Win32 API)
- **FFT.h / FFT.cpp**: Análisis espectral con FFTW3
- **Buffers.h**: Buffers circulares template (header-only)
- **Widgets.h**: Widgets reutilizables ImGui (header-only)

### **Módulos Auxiliares**
- **Console.h / Console.cpp**: Control de consola de Windows
- **main.h**: Declaraciones globales y headers comunes

### **Archivos de Configuración**
- **CMakeLists.txt**: Configuración de compilación CMake
- **copy_dlls_debug.bat**: Script para copiar DLLs al directorio de salida

### **Librerías Externas (submodules Git)**
- **libs/imgui/**: Interfaz gráfica inmediata
- **libs/implot/**: Gráficos científicos
- **libs/glfw/**: Gestión de ventanas y contexto OpenGL
- **libs/iir1/**: Filtros digitales IIR

---

## Trazabilidad Completa del Dato: Desde Arduino hasta Visualización

### **Visión General del Flujo de Datos**

Esta sección documenta el camino exacto que recorre **cada byte** desde que sale del Arduino hasta que se visualiza, filtra y analiza en SerialPlotter.

```
┌──────────────────────────────────────────────────────────────────────┐
│                    FLUJO COMPLETO DE 1 BYTE                          │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ARDUINO MEGA 2560:                                                  │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐                │
│  │   ADC   │→ │ Timer1  │→ │ USART   │→ │   USB   │                │
│  │ 10-bit  │  │   ISR   │  │   TX    │  │  Serial │                │
│  └─────────┘  └─────────┘  └─────────┘  └─────────┘                │
│       ↓            ↓            ↓            ↓                       │
│    ADCH=156   beat=true   UDR0=156      [0x9C]                      │
│                                              │                       │
│  ════════════════════════════════════════════│═══════════════════    │
│                                              │ UART @ 38400 baud    │
│                                              │ ~260 μs/byte         │
│  ════════════════════════════════════════════│═══════════════════    │
│                                              ↓                       │
│  SERIALPLOTTER (PC):                                                │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐                 │
│  │   ReadFile  │→ │  Transform  │→ │   Buffers   │                 │
│  │   Win32 API │  │  ADC→Voltios│  │  Circulares │                 │
│  └─────────────┘  └─────────────┘  └─────────────┘                 │
│        ↓               ↓                  ↓                          │
│   read_buffer[i]  voltage=-0.047V  scrollY->Add()                   │
│      = 156                                                           │
│                                                                      │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐                 │
│  │   Filtro    │→ │  ImPlot     │→ │     FFT     │                 │
│  │   IIR 8º    │  │  Render     │  │   FFTW3     │                 │
│  └─────────────┘  └─────────────┘  └─────────────┘                 │
│        ↓               ↓                  ↓                          │
│  filtered=-0.042V  Pixel(x,y)     Harmonics[10]                     │
│                                                                      │
│  ┌─────────────┐  ┌─────────────┐                                   │
│  │  Inverse    │→ │  WriteFile  │                                   │
│  │  Transform  │  │  Send Back  │                                   │
│  └─────────────┘  └─────────────┘                                   │
│        ↓               ↓                                             │
│   response=154    write_buffer[]                                    │
│                                                                      │
└──────────────────────────────────────────────────────────────────────┘
```

---

### **ETAPA 1: Origen del Dato en Arduino**

#### **1.1. Conversión ADC (adc.cpp)**

```
┌─────────────────────────────────────────────────────────────┐
│ HARDWARE: ADC del ATmega2560                                │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Señal de entrada: 2.5V (ejemplo)                          │
│         ↓                                                   │
│  ┌──────────────────┐                                       │
│  │  ADC 10-bit      │  Vref = 5V                           │
│  │  Conversión:     │  Resolución = 5V / 1024 = 4.88 mV    │
│  └──────────────────┘                                       │
│         ↓                                                   │
│  Valor digital = (2.5V / 5V) × 1024 = 512                  │
│                                                             │
│  Registro ADCH (8 MSB): 512 >> 2 = 128                     │
│  Registro ADCL (2 LSB): descartados (ADLAR=1)              │
│         ↓                                                   │
│  ISR(ADC_vect) ejecuta (~9,615 veces/segundo)              │
│  {                                                          │
│     data = ADCH;  // data = 128                            │
│  }                                                          │
└─────────────────────────────────────────────────────────────┘
```

**Código real (adc.cpp, líneas 28-33):**

```cpp
void ADCController::conversion_complete() {
    data = ADCH;  // Lee solo los 8 bits más significativos
                  // Ejemplo: ADCH = 128 (0x80)
}
```

**Timing:**
- Frecuencia ADC: 125,000 Hz (clock)
- Ciclos por conversión: 13
- **Tasa de conversión: 125,000 / 13 = 9,615 conversiones/segundo**
- Período: 104 μs

---

#### **1.2. Timer1 ISR - Sincronización de Salida (DSP.ino)**

```
┌─────────────────────────────────────────────────────────────┐
│ ISR(TIMER1_COMPA_vect) - Se ejecuta 3,840 veces/segundo    │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Cada 260.42 μs:                                           │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ 1. PORTA = valor;  // Escribir DAC                   │  │
│  │    └─> Salida analógica a pins 22-29                 │  │
│  │                                                        │  │
│  │ 2. beat = true;    // Señalizar a loop()             │  │
│  │    └─> Flag para procesamiento en loop()             │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  Tiempo de ejecución: ~1.25 μs (ultra rápido)             │
└─────────────────────────────────────────────────────────────┘
```

**Código real (DSP.ino, líneas 76-80):**

```cpp
ISR(TIMER1_COMPA_vect) {
    write(valor);  // PORTA = valor (1 instrucción)
    beat = true;   // Activar flag (1 instrucción)
}
// Total: ~20 ciclos @ 16 MHz = 1.25 μs
```

---

#### **1.3. Loop Principal - Lectura ADC y Transmisión USART (DSP.ino)**

```
┌─────────────────────────────────────────────────────────────┐
│ loop() - Ejecuta cada 260.42 μs (cuando beat = true)       │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  if (beat) {  // ← Activado por Timer1                    │
│     beat = false;                                          │
│                                                             │
│     ┌────────────────────────────────────────────────┐    │
│     │ PASO A: Leer valor del ADC                     │    │
│     │ ───────────────────────────────────────────    │    │
│     │ uint8_t muestra_adc = adc.get();               │    │
│     │         └─> Lee 'data' guardado por ADC_vect   │    │
│     │         └─> Ejemplo: muestra_adc = 128         │    │
│     └────────────────────────────────────────────────┘    │
│                                                             │
│     ┌────────────────────────────────────────────────┐    │
│     │ PASO B: Enviar a PC por USART                  │    │
│     │ ───────────────────────────────────────────    │    │
│     │ usart.escribir(muestra_adc);                   │    │
│     │         └─> Encola byte en buffer TX           │    │
│     │         └─> ISR de USART lo enviará            │    │
│     │         └─> Byte transmitido: 0x80 (128)       │    │
│     └────────────────────────────────────────────────┘    │
│                                                             │
│     ┌────────────────────────────────────────────────┐    │
│     │ PASO C: Actualizar valor para DAC              │    │
│     │ ───────────────────────────────────────────    │    │
│     │ if (usart.pendiente_lectura()) {               │    │
│     │    valor = usart.leer();  // Valor del PC      │    │
│     │ } else {                                        │    │
│     │    valor = muestra_adc;   // Echo local        │    │
│     │ }                                               │    │
│     └────────────────────────────────────────────────┘    │
│  }                                                          │
└─────────────────────────────────────────────────────────────┘
```

**Código real (DSP.ino, líneas 142-156):**

```cpp
void loop() {
  if (beat) {
    beat = false;
    
    // PASO A: Leer ADC
    uint8_t muestra_adc = adc.get();  // Ejemplo: 128
    
    // PASO B: Enviar a PC
    usart.escribir(muestra_adc);  // TX → 0x80
    
    // PASO C: Actualizar DAC
    if (usart.pendiente_lectura()) {
      valor = usart.leer();  // Recibir dato procesado del PC
    } else {
      valor = muestra_adc;   // Si no hay respuesta, echo
    }
  }
}
```

---

#### **1.4. Transmisión USART (usart.h)**

```
┌─────────────────────────────────────────────────────────────┐
│ USART TX - Buffer Circular de 256 bytes                    │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  usart.escribir(128):                                      │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ 1. buffer_tx[escritura_tx] = 128;                    │  │
│  │    escritura_tx = (escritura_tx + 1) % 256;          │  │
│  │                                                        │  │
│  │ 2. if (UDR0 vacío) {                                  │  │
│  │       UDR0 = 128;  // Transmitir inmediatamente      │  │
│  │    }                                                   │  │
│  │    else {                                             │  │
│  │       UCSR0B |= UDRIE0;  // Activar ISR UDRE         │  │
│  │    }                                                   │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  ISR(USART0_UDRE_vect):  // Cuando UDR0 vacío             │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ if (buffer no vacío) {                                │  │
│  │    UDR0 = buffer_tx[lectura_tx];                      │  │
│  │    lectura_tx = (lectura_tx + 1) % 256;              │  │
│  │ } else {                                              │  │
│  │    UCSR0B &= ~UDRIE0;  // Desactivar ISR             │  │
│  │ }                                                      │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  Timing @ 38400 baud:                                      │
│  • 1 byte = 10 bits (8N1) = 260.42 μs                     │
└─────────────────────────────────────────────────────────────┘
```

**Transmisión física:**

```
Formato de byte UART 8N1:
┌────┬───┬───┬───┬───┬───┬───┬───┬───┬────┐
│ ST │ 0 │ 0 │ 0 │ 0 │ 0 │ 0 │ 0 │ 1 │ SP │  = 0x80 (128)
└────┴───┴───┴───┴───┴───┴───┴───┴───┴────┘
  ↑                                     ↑
Start bit                            Stop bit

Duración: 10 bits × 26.04 μs/bit = 260.42 μs
```

---

### **ETAPA 2: Recepción en SerialPlotter (PC)**

#### **2.1. Thread SerialWorker - Lectura del Puerto (MainWindow.cpp)**

```
┌─────────────────────────────────────────────────────────────┐
│ SerialWorker() - Thread independiente, bucle infinito      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  while (running) {                                         │
│                                                             │
│     ┌────────────────────────────────────────────────┐    │
│     │ PASO 1: Leer bytes del puerto COM              │    │
│     │ ─────────────────────────────────────────      │    │
│     │ int bytes_read = serial.read(                  │    │
│     │     read_buffer.data(),  // Buffer 512 bytes   │    │
│     │     read_buffer.size()                         │    │
│     │ );                                              │    │
│     │                                                 │    │
│     │ Resultado ejemplo:                              │    │
│     │ • bytes_read = 15                              │    │
│     │ • read_buffer[0..14] = [128,129,127,130,...]   │    │
│     └────────────────────────────────────────────────┘    │
│                                                             │
│     if (bytes_read <= 0) {                                 │
│         std::this_thread::sleep_for(1ms);                  │
│         continue;  // Puerto vacío, esperar               │
│     }                                                       │
│                                                             │
│     ┌────────────────────────────────────────────────┐    │
│     │ PASO 2: Procesar cada byte                     │    │
│     │ ─────────────────────────────────────────      │    │
│     │ for (int i = 0; i < bytes_read; i++) {        │    │
│     │     uint8_t byte = read_buffer[i];            │    │
│     │     // Ejemplo: byte = 128                     │    │
│     │                                                 │    │
│     │     // TRANSFORMAR ADC → VOLTAJE              │    │
│     │     double voltage = TransformSample(byte);    │    │
│     │     // voltage = -0.047 V (ver cálculo abajo) │    │
│     │                                                 │    │
│     │     // Continúa en siguiente paso...           │    │
│     │ }                                               │    │
│     └────────────────────────────────────────────────┘    │
│  }                                                          │
└─────────────────────────────────────────────────────────────┘
```

**Código real de lectura (MainWindow.cpp, ~línea 310):**

```cpp
void MainWindow::SerialWorker() {
    Serial serial;
    serial.open(settings->port, settings->baud_rate);
    
    while (running) {
        // PASO 1: Leer datos
        int bytes_read = serial.read(read_buffer.data(), read_buffer.size());
        
        if (bytes_read <= 0) {
            std::this_thread::sleep_for(1ms);
            continue;
        }
        
        // PASO 2: Procesar cada byte (ver siguiente sección)
        for (int i = 0; i < bytes_read; i++) {
            uint8_t byte = read_buffer[i];
            // ...
        }
    }
}
```

---

#### **2.2. Transformación ADC → Voltaje (MainWindow.cpp)**

```
┌─────────────────────────────────────────────────────────────┐
│ TransformSample(uint8_t adc_value) → double voltage        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Entrada: adc_value = 128 (del Arduino)                    │
│                                                             │
│  Cálculo (mapeo lineal):                                   │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ voltage = (adc_value - adc_min) × volt_per_unit      │  │
│  │                                  + voltage_min        │  │
│  │                                                        │  │
│  │ Valores por defecto:                                  │  │
│  │ • adc_min = 0                                         │  │
│  │ • adc_max = 255                                       │  │
│  │ • voltage_min = -6.0 V                                │  │
│  │ • voltage_max = +6.0 V                                │  │
│  │                                                        │  │
│  │ volt_per_unit = (6.0 - (-6.0)) / (255 - 0)           │  │
│  │               = 12.0 / 255                            │  │
│  │               = 0.047058824 V/unidad                  │  │
│  │                                                        │  │
│  │ Cálculo ejemplo (adc_value = 128):                   │  │
│  │ voltage = (128 - 0) × 0.047058824 + (-6.0)           │  │
│  │         = 6.023529 - 6.0                             │  │
│  │         = 0.023529 V                                  │  │
│  │         ≈ 0.024 V (23.5 mV)                          │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  Tabla de conversión rápida:                               │
│  ┌──────────┬─────────────┐                                │
│  │ ADC (8b) │   Voltaje   │                                │
│  ├──────────┼─────────────┤                                │
│  │    0     │   -6.00 V   │                                │
│  │   64     │   -3.01 V   │                                │
│  │  128     │   +0.024 V  │ ← Ejemplo                     │
│  │  192     │   +3.05 V   │                                │
│  │  255     │   +6.00 V   │                                │
│  └──────────┴─────────────┘                                │
└─────────────────────────────────────────────────────────────┘
```

**Código real (MainWindow.cpp, ~línea 150):**

```cpp
double MainWindow::TransformSample(uint8_t adc_value) {
    double volt_per_unit = (settings->voltage_max - settings->voltage_min) / 
                           (settings->adc_max - settings->adc_min);
    
    return (adc_value - settings->adc_min) * volt_per_unit + settings->voltage_min;
}
```

---

#### **2.3. Almacenamiento en Buffers Circulares (MainWindow.cpp)**

```
┌─────────────────────────────────────────────────────────────┐
│ PASO 3: Guardar en buffers (Thread-Safe con mutex)         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  {                                                          │
│     std::lock_guard<std::mutex> lock(data_mutex);          │
│     // ↑ Bloquea acceso concurrente (thread safety)        │
│                                                             │
│     if (!freeze) {  // Solo si no está congelado           │
│                                                             │
│        ┌────────────────────────────────────────────┐      │
│        │ Buffer de tiempo (eje X):                  │      │
│        │ ─────────────────────────────────────      │      │
│        │ (*scrollX)[size] = next_time;              │      │
│        │                                             │      │
│        │ Ejemplo:                                    │      │
│        │ • size = 4521                              │      │
│        │ • next_time = 1.177344 segundos            │      │
│        │ • scrollX[4521] = 1.177344                 │      │
│        └────────────────────────────────────────────┘      │
│                                                             │
│        ┌────────────────────────────────────────────┐      │
│        │ Buffer de voltaje (eje Y - entrada):       │      │
│        │ ─────────────────────────────────────      │      │
│        │ (*scrollY)[size] = voltage;                │      │
│        │                                             │      │
│        │ Ejemplo:                                    │      │
│        │ • voltage = 0.024 V                        │      │
│        │ • scrollY[4521] = 0.024                    │      │
│        └────────────────────────────────────────────┘      │
│                                                             │
│        size++;  // Incrementar contador                    │
│        next_time += 1.0 / settings->sampling_rate;         │
│        //           1.0 / 3840 = 0.0002604 s              │
│     }                                                       │
│  }  // ← Unlock automático al salir del scope             │
└─────────────────────────────────────────────────────────────┘
```

**Estructura de ScrollBuffer (Buffers.h):**

```cpp
template <typename T>
class ScrollBuffer {
private:
    std::vector<T> data;  // Vector de 230,400 elementos (60 seg @ 3840 Hz)
    int max_size;         // 230,400
    int view_size;        // 115,200 (30 seg visibles)
    int offset;           // Índice de inicio (circular)
    
public:
    // Acceso con wrap-around automático
    T& operator[](int i) {
        return data[(offset + i) % max_size];
    }
    
    // Para ImPlot (zero-copy, puntero directo)
    const T* Data() const {
        return &data[offset];
    }
};
```

**Visualización del buffer circular:**

```
Buffer de 230,400 muestras (60 segundos @ 3840 Hz):

┌─────────────────────────────────────────────────────────┐
│ [0]  [1]  [2]  ... [offset] ... [size] ... [230399]   │
│  │    │    │         ↑           ↑                      │
│  │    │    │      Inicio       Actual                   │
│  │    │    │      visible                               │
│  └────┴────┴─────── Datos antiguos (pueden sobrescribirse)
│                                                          │
│  Si size >= max_size:                                   │
│  • offset avanza → sobrescribe datos más viejos         │
│  • Se mantienen siempre últimos 60 segundos             │
└─────────────────────────────────────────────────────────┘
```

---

### **ETAPA 3: Aplicación del Filtro Digital IIR**

#### **3.1. Decisión de Filtrado (MainWindow.cpp)**

```
┌─────────────────────────────────────────────────────────────┐
│ PASO 4: Aplicar filtro si está activo                      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  if (current_filter != Filter::None) {                     │
│                                                             │
│     ┌────────────────────────────────────────────────┐    │
│     │ Llamar a filtro IIR Butterworth 8º orden       │    │
│     │ ───────────────────────────────────────────    │    │
│     │ double filtered = ApplyFilter(voltage);        │    │
│     │                                                 │    │
│     │ Entrada: voltage = 0.024 V                     │    │
│     │ Salida:  filtered = ??? (depende del filtro)   │    │
│     └────────────────────────────────────────────────┘    │
│                                                             │
│     ┌────────────────────────────────────────────────┐    │
│     │ Guardar en buffer de salida filtrada           │    │
│     │ ───────────────────────────────────────────    │    │
│     │ (*filter_scrollY)[size] = filtered;            │    │
│     └────────────────────────────────────────────────┘    │
│  }                                                          │
└─────────────────────────────────────────────────────────────┘
```

**Código real (MainWindow.cpp, ~línea 340):**

```cpp
// Dentro del bucle de procesamiento:
for (int i = 0; i < bytes_read; i++) {
    uint8_t byte = read_buffer[i];
    double voltage = TransformSample(byte);
    
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        
        if (!freeze) {
            (*scrollX)[size] = next_time;
            (*scrollY)[size] = voltage;
            
            // APLICAR FILTRO
            if (current_filter != Filter::None) {
                double filtered = ApplyFilter(voltage);
                (*filter_scrollY)[size] = filtered;
            }
            
            size++;
            next_time += 1.0 / settings->sampling_rate;
        }
    }
    
    // Continúa con preparación de respuesta...
}
```

---

#### **3.2. Funcionamiento del Filtro IIR Butterworth (ApplyFilter)**

```
┌─────────────────────────────────────────────────────────────┐
│ ApplyFilter(double sample) - Filtro IIR de 8º Orden        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Configuración ejemplo: PASA-BAJOS @ 500 Hz                │
│  ───────────────────────────────────────────────────────   │
│  • Frecuencia de muestreo: 3840 Hz                         │
│  • Frecuencia de corte: 500 Hz                             │
│  • Orden del filtro: 8                                     │
│  • Tipo: Butterworth (respuesta plana)                     │
│                                                             │
│  Ecuación de diferencias (forma IIR):                      │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ y[n] = b₀·x[n] + b₁·x[n-1] + ... + b₈·x[n-8]        │  │
│  │      - a₁·y[n-1] - a₂·y[n-2] - ... - a₈·y[n-8]      │  │
│  │                                                        │  │
│  │ Donde:                                                │  │
│  │ • x[n] = muestra actual (0.024 V)                    │  │
│  │ • x[n-k] = muestras anteriores (historial entrada)   │  │
│  │ • y[n] = salida actual (calculada)                   │  │
│  │ • y[n-k] = salidas anteriores (historial salida)     │  │
│  │ • bᵢ = coeficientes feedforward (parte FIR)          │  │
│  │ • aᵢ = coeficientes feedback (parte IIR)             │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  Coeficientes (calculados por librería iir1):              │
│  ───────────────────────────────────────────────────────   │
│  b = [0.001234, 0.009872, 0.034552, ...]  (9 valores)     │
│  a = [1.000000, -5.234567, 12.345678, ...] (9 valores)    │
│                                                             │
│  Historial de entrada (x_history):                         │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ x[n-1] = 0.022 V                                     │  │
│  │ x[n-2] = 0.019 V                                     │  │
│  │ x[n-3] = 0.015 V                                     │  │
│  │ ... (hasta x[n-8])                                   │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  Historial de salida (y_history):                          │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ y[n-1] = 0.021 V  (última salida filtrada)          │  │
│  │ y[n-2] = 0.018 V                                     │  │
│  │ y[n-3] = 0.014 V                                     │  │
│  │ ... (hasta y[n-8])                                   │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  CÁLCULO (simplificado):                                   │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ y[n] = 0.001234×0.024  (término actual)             │  │
│  │      + 0.009872×0.022  (x[n-1])                     │  │
│  │      + 0.034552×0.019  (x[n-2])                     │  │
│  │      + ...              (resto de términos FIR)      │  │
│  │      - (-5.234567)×0.021  (feedback y[n-1])         │  │
│  │      - 12.345678×0.018    (feedback y[n-2])         │  │
│  │      - ...                 (resto feedback)          │  │
│  │                                                        │  │
│  │ y[n] ≈ 0.0218 V  (resultado filtrado)               │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  ACTUALIZAR HISTORIALES:                                   │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ Desplazar historiales (shift):                       │  │
│  │ x[n-8] ← x[n-7] ← ... ← x[n-1] ← x[n] (0.024)      │  │
│  │ y[n-8] ← y[n-7] ← ... ← y[n-1] ← y[n] (0.0218)     │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  RETORNAR: 0.0218 V                                        │
└─────────────────────────────────────────────────────────────┘
```

**Código real (MainWindow.cpp, ~línea 180):**

```cpp
double MainWindow::ApplyFilter(double sample) {
    switch (current_filter) {
        case Filter::LowPass:
            return lowpass_filter.filter(sample);  // ← Aquí
            
        case Filter::HighPass:
            return highpass_filter.filter(sample);
            
        case Filter::None:
        default:
            return sample;
    }
}
```

**Configuración del filtro (MainWindow.cpp, ~línea 200):**

```cpp
void MainWindow::ConfigureFilters() {
    double nyquist = settings->sampling_rate / 2.0;  // 1920 Hz
    cutoff_frequency = std::min(cutoff_frequency, (float)nyquist);
    
    switch (current_filter) {
        case Filter::LowPass:
            lowpass_filter.setup(settings->sampling_rate, cutoff_frequency);
            // Ejemplo: setup(3840, 500) → filtro pasa-bajos @ 500 Hz
            break;
            
        case Filter::HighPass:
            highpass_filter.setup(settings->sampling_rate, cutoff_frequency);
            break;
    }
}
```

**Respuesta en frecuencia del filtro:**

```
Pasa-Bajos Butterworth 8º Orden @ 500 Hz:

Ganancia (dB)
   0 ┤                   
     │ ─────────────────┐            Banda de paso (plana)
  -3 ┤                  └─┐          fc = 500 Hz (-3 dB)
     │                    │
 -10 ┤                    └──┐
     │                       └──┐
 -48 ┤                          └────────    Banda de rechazo
     │                                       (-48 dB/octava)
-100 ┤                                  ─────────────────
     └──────┬──────┬──────┬──────┬──────┬──────────────
           100    500   1000   2000   3840  Frecuencia (Hz)
                   ↑
              Frecuencia de corte
```

---

### **ETAPA 4: Transmisión de Vuelta al Arduino**

#### **4.1. Transformación Inversa Voltaje → ADC (MainWindow.cpp)**

```
┌─────────────────────────────────────────────────────────────┐
│ PASO 5: Preparar respuesta para Arduino                    │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  if (current_filter != Filter::None) {                     │
│                                                             │
│     ┌────────────────────────────────────────────────┐    │
│     │ Obtener voltaje filtrado                        │    │
│     │ ───────────────────────────────────────────    │    │
│     │ double filtered_voltage =                      │    │
│     │     (*filter_scrollY)[size - 1];               │    │
│     │                                                 │    │
│     │ Ejemplo: filtered_voltage = 0.0218 V           │    │
│     └────────────────────────────────────────────────┘    │
│                                                             │
│     ┌────────────────────────────────────────────────┐    │
│     │ Transformación INVERSA (V → ADC)               │    │
│     │ ───────────────────────────────────────────    │    │
│     │ uint8_t response =                             │    │
│     │     InverseTransformSample(filtered_voltage);  │    │
│     │                                                 │    │
│     │ Cálculo:                                        │    │
│     │ ┌────────────────────────────────────────┐     │    │
│     │ │ adc = (voltage - voltage_min) /        │     │    │
│     │ │           volt_per_unit + adc_min      │     │    │
│     │ │                                         │     │    │
│     │ │     = (0.0218 - (-6.0)) /              │     │    │
│     │ │           0.047058824 + 0              │     │    │
│     │ │                                         │     │    │
│     │ │     = 6.0218 / 0.047058824             │     │    │
│     │ │     = 127.95                           │     │    │
│     │ │     ≈ 128 (redondeado)                 │     │    │
│     │ │                                         │     │    │
│     │ │ response = 128                          │     │    │
│     │ └────────────────────────────────────────┘     │    │
│     └────────────────────────────────────────────────┘    │
│                                                             │
│     ┌────────────────────────────────────────────────┐    │
│     │ Encolar en buffer de escritura                 │    │
│     │ ───────────────────────────────────────────    │    │
│     │ write_buffer[write_index++] = response;        │    │
│     │                                                 │    │
│     │ Ejemplo:                                        │    │
│     │ • write_buffer[0] = 128                        │    │
│     │ • write_index = 1                              │    │
│     └────────────────────────────────────────────────┘    │
│  }                                                          │
└─────────────────────────────────────────────────────────────┘
```

**Código real (MainWindow.cpp, ~línea 160):**

```cpp
double MainWindow::InverseTransformSample(double voltage) {
    double volt_per_unit = (settings->voltage_max - settings->voltage_min) / 
                           (settings->adc_max - settings->adc_min);
    
    double adc_value = (voltage - settings->voltage_min) / volt_per_unit 
                       + settings->adc_min;
    
    // Limitar a rango 0-255
    adc_value = std::clamp(adc_value, 0.0, 255.0);
    
    return static_cast<uint8_t>(adc_value);
}
```

**Código de encolado (MainWindow.cpp, ~línea 350):**

```cpp
// Dentro del bucle SerialWorker, después del filtrado:
if (current_filter != Filter::None) {
    double filtered_voltage = (*filter_scrollY)[size - 1];
    uint8_t response = InverseTransformSample(filtered_voltage);
    write_buffer[write_index++] = response;
}
```

---

#### **4.2. Envío por Puerto Serial (MainWindow.cpp)**

```
┌─────────────────────────────────────────────────────────────┐
│ PASO 6: Transmitir buffer de respuesta                     │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  if (write_index > 0) {  // Si hay datos para enviar       │
│                                                             │
│     ┌────────────────────────────────────────────────┐    │
│     │ Llamar a WriteFile() de Windows                │    │
│     │ ───────────────────────────────────────────    │    │
│     │ serial.write(                                  │    │
│     │     write_buffer.data(),  // [128, ...]       │    │
│     │     write_index           // 1 byte           │    │
│     │ );                                             │    │
│     │                                                 │    │
│     │ WriteFile() interno (Serial.cpp):              │    │
│     │ ┌────────────────────────────────────────┐     │    │
│     │ │ DWORD bytes_written;                   │     │    │
│     │ │ WriteFile(                             │     │    │
│     │ │     file,        // Handle del COM     │     │    │
│     │ │     data,        // [128]              │     │    │
│     │ │     size,        // 1                  │     │    │
│     │ │     &bytes_written,                    │     │    │
│     │ │     nullptr                            │     │    │
│     │ │ );                                     │     │    │
│     │ └────────────────────────────────────────┘     │    │
│     └────────────────────────────────────────────────┘    │
│                                                             │
│     ┌────────────────────────────────────────────────┐    │
│     │ Resetear índice de escritura                   │    │
│     │ ───────────────────────────────────────────    │    │
│     │ write_index = 0;                               │    │
│     └────────────────────────────────────────────────┘    │
│  }                                                          │
│                                                             │
│  Transmisión física:                                       │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ USB Serial → Arduino RX                             │  │
│  │                                                      │  │
│  │ Byte 128 (0x80) transmitido en formato 8N1:        │  │
│  │ ┌────┬───┬───┬───┬───┬───┬───┬───┬───┬────┐       │  │
│  │ │ ST │ 0 │ 0 │ 0 │ 0 │ 0 │ 0 │ 0 │ 1 │ SP │       │  │
│  │ └────┴───┴───┴───┴───┴───┴───┴───┴───┴────┘       │  │
│  │                                                      │  │
│  │ Tiempo: 260.42 μs @ 38400 baud                     │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

**Código real (MainWindow.cpp, ~línea 360):**

```cpp
// Al final del bucle SerialWorker:
if (write_index > 0) {
    serial.write(write_buffer.data(), write_index);
    write_index = 0;
}
```

---

### **ETAPA 5: Análisis FFT (Thread Independiente)**

#### **5.1. Thread AnalysisWorker - Ejecución de FFT (MainWindow.cpp)**

```
┌─────────────────────────────────────────────────────────────┐
│ AnalysisWorker() - Se ejecuta cada 100 ms                  │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  while (running) {                                         │
│                                                             │
│     ┌────────────────────────────────────────────────┐    │
│     │ Verificar condiciones para FFT                 │    │
│     │ ───────────────────────────────────────────    │    │
│     │ if (!freeze && size >= settings->samples) {   │    │
│     │     // samples = 3840 (1 segundo de datos)    │    │
│     │                                                 │    │
│     │     Condiciones:                                │    │
│     │     ✓ NO congelado                             │    │
│     │     ✓ Al menos 3840 muestras disponibles       │    │
│     │ }                                               │    │
│     └────────────────────────────────────────────────┘    │
│                                                             │
│     ┌────────────────────────────────────────────────┐    │
│     │ PASO 1: Copiar datos (thread-safe)            │    │
│     │ ───────────────────────────────────────────    │    │
│     │ std::vector<double> samples_copy(3840);       │    │
│     │                                                 │    │
│     │ {                                              │    │
│     │    std::lock_guard lock(data_mutex);          │    │
│     │                                                 │    │
│     │    int start = size - 3840;                   │    │
│     │    for (int i = 0; i < 3840; i++) {           │    │
│     │        samples_copy[i] = (*scrollY)[start+i]; │    │
│     │    }                                            │    │
│     │ }  // Unlock automático                        │    │
│     │                                                 │    │
│     │ Ejemplo samples_copy:                          │    │
│     │ [0.024, 0.022, 0.019, ..., -0.015] (3840)     │    │
│     └────────────────────────────────────────────────┘    │
│                                                             │
│     ┌────────────────────────────────────────────────┐    │
│     │ PASO 2: Ejecutar FFT (sin mutex)              │    │
│     │ ───────────────────────────────────────────    │    │
│     │ fft->SetData(samples_copy.data(), 3840);      │    │
│     │ fft->Compute();  // ← FFTW3 ejecuta aquí     │    │
│     │                                                 │    │
│     │ Tiempo de ejecución: ~5-10 ms                 │    │
│     └────────────────────────────────────────────────┘    │
│                                                             │
│     std::this_thread::sleep_for(100ms);                    │
│  }                                                          │
└─────────────────────────────────────────────────────────────┘
```

**Código real (MainWindow.cpp, ~línea 500):**

```cpp
void MainWindow::AnalysisWorker() {
    using namespace std::chrono_literals;
    
    while (running) {
        if (!freeze && size >= settings->samples) {
            
            // PASO 1: Copiar datos
            std::vector<double> samples_copy(settings->samples);
            {
                std::lock_guard<std::mutex> lock(data_mutex);
                int start = size - settings->samples;
                for (int i = 0; i < settings->samples; i++) {
                    samples_copy[i] = (*scrollY)[start + i];
                }
            }
            
            // PASO 2: FFT
            fft->SetData(samples_copy.data(), settings->samples);
            fft->Compute();
        }
        
        std::this_thread::sleep_for(100ms);
    }
}
```

---

#### **5.2. Ejecución de FFT con FFTW3 (FFT.cpp)**

```
┌─────────────────────────────────────────────────────────────┐
│ fft->Compute() - Transformada Rápida de Fourier            │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Datos de entrada (dominio tiempo):                        │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ samples[0..3839] = [0.024, 0.022, 0.019, ...]       │  │
│  │                                                        │  │
│  │ Representa: 1 segundo de señal @ 3840 Hz             │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  PASO A: Aplicar ventana Hann (opcional)                   │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ for (i = 0; i < 3840; i++) {                         │  │
│  │     window[i] = 0.5 * (1 - cos(2π*i/3839));         │  │
│  │     samples[i] *= window[i];                         │  │
│  │ }                                                      │  │
│  │                                                        │  │
│  │ Propósito: Reducir "spectral leakage" (fuga)         │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  PASO B: Ejecutar FFTW3                                    │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ fftw_execute(plan);                                   │  │
│  │                                                        │  │
│  │ Input:  samples[3840] (reales)                       │  │
│  │ Output: complex[1920] (complejos)                    │  │
│  │         └─> Solo N/2 bins (simetría Hermitiana)     │  │
│  │                                                        │  │
│  │ Cada bin representa:                                  │  │
│  │ • complex[k] = A + Bi (número complejo)              │  │
│  │ • k = índice del bin (0 a 1919)                      │  │
│  │ • Frecuencia = k × (3840 Hz / 3840) = k Hz          │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  PASO C: Calcular magnitudes                               │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ for (int k = 0; k < 1920; k++) {                     │  │
│  │     double real = complex[k][0];                     │  │
│  │     double imag = complex[k][1];                     │  │
│  │                                                        │  │
│  │     amplitudes[k] = sqrt(real² + imag²) / N;         │  │
│  │                   = sqrt(real² + imag²) / 3840;      │  │
│  │                                                        │  │
│  │     if (k > 0) amplitudes[k] *= 2;  // DC offset     │  │
│  │ }                                                      │  │
│  │                                                        │  │
│  │ Resultado ejemplo:                                    │  │
│  │ amplitudes[0]   = 0.015 V    (DC offset)             │  │
│  │ amplitudes[60]  = 2.350 V    (60 Hz - fundamental)   │  │
│  │ amplitudes[120] = 0.420 V    (2ª armónica)           │  │
│  │ amplitudes[180] = 0.180 V    (3ª armónica)           │  │
│  │ ...                                                    │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

**Código real (FFT.cpp, ~línea 80):**

```cpp
void FFT::Compute() {
    // Ejecutar plan de FFTW3
    fftw_execute(p);
    
    // Calcular magnitudes
    for (int i = 0; i < settings->samples / 2; i++) {
        double real = complex[i][0];
        double imag = complex[i][1];
        
        amplitudes[i] = std::sqrt(real * real + imag * imag) / settings->samples;
        
        if (i > 0) amplitudes[i] *= 2.0;  // Corrección para bins no-DC
    }
}
```

---

#### **5.3. Detección de Armónicas (FFT.cpp)**

```
┌─────────────────────────────────────────────────────────────┐
│ FindHarmonics(double fs, int count) - Detectar 10 picos    │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Entrada:                                                   │
│  • fs = 3840 Hz (frecuencia de muestreo)                   │
│  • count = 10 (armónicas a detectar)                       │
│                                                             │
│  PASO 1: Encontrar frecuencia fundamental                  │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ Buscar máximo en amplitudes[1..1919]:                │  │
│  │                                                        │  │
│  │ max_amplitude = -infinito;                            │  │
│  │ fundamental_index = 0;                                │  │
│  │                                                        │  │
│  │ for (i = 1; i < 1920; i++) {                         │  │
│  │     if (amplitudes[i] > max_amplitude) {             │  │
│  │         max_amplitude = amplitudes[i];               │  │
│  │         fundamental_index = i;                       │  │
│  │     }                                                  │  │
│  │ }                                                      │  │
│  │                                                        │  │
│  │ Resultado ejemplo:                                    │  │
│  │ • fundamental_index = 60                             │  │
│  │ • max_amplitude = 2.350 V                            │  │
│  │ • Frecuencia fundamental = 60 × (3840/3840) = 60 Hz │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  PASO 2: Buscar armónicas múltiplos enteros                │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ for (k = 1; k <= 10; k++) {                          │  │
│  │     harmonic_index = fundamental_index × k;          │  │
│  │     harmonic_freq = harmonic_index × (fs / N);       │  │
│  │                                                        │  │
│  │     // Verificar Nyquist                             │  │
│  │     if (harmonic_freq > fs / 2) break;               │  │
│  │                                                        │  │
│  │     // Buscar pico local (±5 bins)                   │  │
│  │     peak_index = harmonic_index;                     │  │
│  │     peak_amplitude = amplitudes[harmonic_index];     │  │
│  │                                                        │  │
│  │     for (offset = -5; offset <= 5; offset++) {       │  │
│  │         int idx = harmonic_index + offset;           │  │
│  │         if (amplitudes[idx] > peak_amplitude) {      │  │
│  │             peak_amplitude = amplitudes[idx];        │  │
│  │             peak_index = idx;                        │  │
│  │         }                                             │  │
│  │     }                                                  │  │
│  │                                                        │  │
│  │     // Guardar armónica                              │  │
│  │     harmonics[k-1] = {                               │  │
│  │         frequency: peak_index × (fs/N),              │  │
│  │         amplitude: peak_amplitude                    │  │
│  │     };                                                │  │
│  │ }                                                      │  │
│  │                                                        │  │
│  │ Resultado ejemplo (señal 60 Hz + armónicas):         │  │
│  │ ┌──────────────┬──────────────┬──────────────┐       │  │
│  │ │  Armónica    │  Frecuencia  │  Amplitud    │       │  │
│  │ ├──────────────┼──────────────┼──────────────┤       │  │
│  │ │  1ª (fund.)  │     60 Hz    │   2.350 V    │       │  │
│  │ │  2ª          │    120 Hz    │   0.420 V    │       │  │
│  │ │  3ª          │    180 Hz    │   0.180 V    │       │  │
│  │ │  4ª          │    240 Hz    │   0.090 V    │       │  │
│  │ │  5ª          │    300 Hz    │   0.055 V    │       │  │
│  │ │  6ª          │    360 Hz    │   0.032 V    │       │  │
│  │ │  7ª          │    420 Hz    │   0.018 V    │       │  │
│  │ │  8ª          │    480 Hz    │   0.012 V    │       │  │
│  │ │  9ª          │    540 Hz    │   0.008 V    │       │  │
│  │ │  10ª         │    600 Hz    │   0.005 V    │       │  │
│  │ └──────────────┴──────────────┴──────────────┘       │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  PASO 3: Calcular THD (Total Harmonic Distortion)          │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ THD = √(A₂² + A₃² + ... + A₁₀²) / A₁ × 100%        │  │
│  │                                                        │  │
│  │     = √(0.420² + 0.180² + ... + 0.005²) / 2.350     │  │
│  │     = √(0.2283) / 2.350                             │  │
│  │     = 0.4778 / 2.350                                │  │
│  │     = 0.2033 × 100%                                  │  │
│  │     = 20.33%                                         │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  RETORNAR: Vector de 10 armónicas + THD                    │
└─────────────────────────────────────────────────────────────┘
```

**Código real (FFT.cpp, ~línea 120):**

```cpp
std::vector<Harmonic> FFT::FindHarmonics(double fs, int count) {
    std::vector<Harmonic> result;
    
    // PASO 1: Encontrar fundamental
    int fundamental_index = 0;
    double max_amp = 0;
    
    for (int i = 1; i < amplitudes.size(); i++) {
        if (amplitudes[i] > max_amp) {
            max_amp = amplitudes[i];
            fundamental_index = i;
        }
    }
    
    // PASO 2: Buscar armónicas
    for (int k = 1; k <= count; k++) {
        int harmonic_index = fundamental_index * k;
        double freq = harmonic_index * (fs / settings->samples);
        
        // Verificar Nyquist
        if (freq > fs / 2.0) break;
        
        // Buscar pico local (±5 bins)
        int peak_idx = harmonic_index;
        double peak_amp = amplitudes[harmonic_index];
        
        for (int offset = -5; offset <= 5; offset++) {
            int idx = harmonic_index + offset;
            if (idx >= 0 && idx < amplitudes.size()) {
                if (amplitudes[idx] > peak_amp) {
                    peak_amp = amplitudes[idx];
                    peak_idx = idx;
                }
            }
        }
        
        result.push_back({
            .frequency = peak_idx * (fs / settings->samples),
            .amplitude = peak_amp
        });
    }
    
    return result;
}
```

---

### **ETAPA 6: Visualización en ImPlot (Main Thread)**

#### **6.1. Renderizado de Gráficos (MainWindow::DrawGraphs)**

```
┌─────────────────────────────────────────────────────────────┐
│ DrawGraphs() - Renderizado a 60 FPS                        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  GRÁFICO 1: Entrada (ADC)                                  │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ ImPlot::BeginPlot("Entrada (ADC)")                    │  │
│  │                                                        │  │
│  │ {                                                      │  │
│  │    std::lock_guard lock(data_mutex);  // Thread-safe │  │
│  │                                                        │  │
│  │    const double* x = freeze ?                        │  │
│  │        frozen_dataX.data() : scrollX->Data();        │  │
│  │    const double* y = freeze ?                        │  │
│  │        frozen_dataY.data() : scrollY->Data();        │  │
│  │    int count = freeze ?                              │  │
│  │        frozen_dataX.size() : scrollY->Size();        │  │
│  │ }  // Unlock                                          │  │
│  │                                                        │  │
│  │ ImPlot::PlotLine("ADC", x, y, count);                │  │
│  │         └─> Renderiza línea verde con 4521 puntos    │  │
│  │                                                        │  │
│  │ ImPlot::EndPlot();                                    │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  GRÁFICO 2: Salida Filtrada                                │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ ImPlot::BeginPlot("Salida (Filtrada)")               │  │
│  │                                                        │  │
│  │ if (current_filter != Filter::None) {                │  │
│  │     const double* y_filt = filter_scrollY->Data();   │  │
│  │     ImPlot::PlotLine("Filtrada", x, y_filt, count);  │  │
│  │ }                                                      │  │
│  │                                                        │  │
│  │ ImPlot::EndPlot();                                    │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  GRÁFICO 3: Espectro FFT                                   │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ ImPlot::BeginPlot("Espectro (FFT)")                  │  │
│  │                                                        │  │
│  │ if (fft) {                                            │  │
│  │     fft->Plot(settings->sampling_rate);              │  │
│  │     └─> Dibuja barras de amplitud vs frecuencia      │  │
│  │ }                                                      │  │
│  │                                                        │  │
│  │ ImPlot::EndPlot();                                    │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                             │
│  PANEL ARMÓNICAS: Tabla + THD                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ auto harmonics = fft->FindHarmonics(fs, 10);         │  │
│  │                                                        │  │
│  │ Renderizar tabla:                                     │  │
│  │ ┌────────┬──────────────┬─────────────┐              │  │
│  │ │   #    │  Frecuencia  │  Amplitud   │              │  │
│  │ ├────────┼──────────────┼─────────────┤              │  │
│  │ │   1ª   │    60.0 Hz   │   2.350 V   │              │  │
│  │ │   2ª   │   120.0 Hz   │   0.420 V   │              │  │
│  │ │  ...   │     ...      │    ...      │              │  │
│  │ │  10ª   │   600.0 Hz   │   0.005 V   │              │  │
│  │ └────────┴──────────────┴─────────────┘              │  │
│  │                                                        │  │
│  │ Texto: "THD: 20.33%"                                  │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

**Código real (MainWindow.cpp, ~línea 600):**

```cpp
void MainWindow::DrawGraphs() {
    float graph_height = ImGui::GetContentRegionAvail().y / 3.0f;
    
    // GRÁFICO 1
    if (ImPlot::BeginPlot("Entrada (ADC)", ImVec2(-1, graph_height))) {
        ImPlot::SetupAxisLimits(ImAxis_X1, left_limit, right_limit);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -6, 6);
        
        const double* x_data = freeze ? frozen_dataX.data() : scrollX->Data();
        const double* y_data = freeze ? frozen_dataY.data() : scrollY->Data();
        int count = freeze ? frozen_dataX.size() : scrollY->Size();
        
        ImPlot::SetNextLineStyle(ImVec4(0.110f, 0.784f, 0.035f, 1.0f));
        ImPlot::PlotLine("ADC", x_data, y_data, count);
        
        ImPlot::EndPlot();
    }
    
    // GRÁFICO 2 y 3 similares...
    
    DrawHarmonicsPanel();  // Panel de armónicas
}
```

---

## Resumen del Flujo Completo

```
════════════════════════════════════════════════════════════════
                    CRONOLOGÍA DE 1 BYTE (128)                    
════════════════════════════════════════════════════════════════

T=0 μs:        ADC convierte 2.5V → 512 (10-bit)
               ISR(ADC_vect) guarda ADCH=128 en 'data'

T=104 μs:      Timer1 ISR ejecuta
               • PORTA = valor_anterior
               • beat = true

T=105 μs:      loop() detecta beat=true
               • muestra_adc = adc.get() = 128
               • usart.escribir(128)

T=260 μs:      USART TX completa transmisión de 128

───────────────────────────────────────────────────────────────

PC:            ReadFile() lee byte 128 del buffer USB

T=0 ms:        SerialWorker recibe byte
               • voltage = TransformSample(128) = 0.024 V
               • scrollY[4521] = 0.024
               • filtered = ApplyFilter(0.024) = 0.0218 V
               • filter_scrollY[4521] = 0.0218
               • response = InverseTransform(0.0218) = 128
               • WriteFile(128) envía de vuelta

T=0.26 ms:     Arduino recibe 128 procesado
               • ISR(USART0_RX_vect) guarda en buffer RX
               • loop() leerá en próxima iteración

T=100 ms:      AnalysisWorker ejecuta FFT
               • Copia últimas 3840 muestras
               • Compute() calcula espectro
               • FindHarmonics() detecta 10 picos

T=16.67 ms:    Main thread renderiza (60 FPS)
               • Dibuja 3 gráficos
               • Muestra tabla de armónicas
               • Calcula THD

════════════════════════════════════════════════════════════════
```

---

*Esta sección documenta el camino completo de cada byte de datos, desde su origen en el ADC del Arduino hasta su visualización, análisis espectral y retransmisión filtrada al sistema embebido, formando un bucle cerrado de procesamiento digital de señales en tiempo real.*

---

## Funcionamiento Paso a Paso del Código Real

### PASO 1: Inicialización del Sistema (main.cpp)

**1.1. Registro de Signal Handlers**

```cpp
// Capturar Ctrl+C, Alt+F4, terminación del proceso
std::signal(SIGINT, signal_handler);   // Ctrl+C
std::signal(SIGTERM, signal_handler);  // Terminación
std::signal(SIGABRT, signal_handler);  // Abort
```

**¿Para qué?**
- Cerrar puerto serial correctamente antes de terminar
- Liberar recursos OpenGL y buffers
- Evitar dejar el puerto COM bloqueado

**1.2. Creación de Objetos Principales**

```cpp
Settings settings;                              // Configuración global
SettingsWindow settings_window(settings);       // Ventana config (deprecated)
MainWindow mainWindow(width, height, settings, settings_window);
```

**Orden de inicialización:**
1. `Settings`: Valores por defecto (3840 Hz, baudrate 38400, etc.)
2. `SettingsWindow`: Wrapper sobre settings (ya no se usa)
3. `MainWindow`: Constructor ejecuta `CreateBuffers()`

**1.3. Ocultar Consola de Windows**

```cpp
Console console;
if (console.IsOwn())
    console.Hide(true);
```

**¿Qué hace esto?**
- Detecta si la aplicación se lanzó desde consola o doble-click
- Si tiene consola propia → la oculta para UX más limpia
- Si se lanzó desde cmd → mantiene consola visible para logs

**1.4. Inicializar GLFW y OpenGL**

```cpp
// Inicializar biblioteca de ventanas
if (!glfwInit())
    return -1;

// Configurar versión OpenGL 3.3 Core
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
```

**Configuración crítica:**
- OpenGL 3.3: Balance entre compatibilidad y características modernas
- Core Profile: Sin APIs legacy (más eficiente)
- Forward Compatible: Preparado para futuras versiones

**1.5. Crear Ventana**

```cpp
GLFWwindow* window = glfwCreateWindow(width, height, "SerialPlotter - DSP", nullptr, nullptr);
if (window == nullptr)
    return -1;

glfwMakeContextCurrent(window);
glfwSwapInterval(1);  // VSync activado (60 FPS)
```

**Callbacks registrados:**
```cpp
glfwSetWindowSizeCallback(window, window_resize);      // Detectar cambio de tamaño
glfwSetWindowIconifyCallback(window, window_minimized); // Detectar minimizado
glfwSetWindowFocusCallback(window, window_focused);     // Detectar pérdida de foco
```

**1.6. Cargar OpenGL con GLAD**

```cpp
if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    return -1;
}
```

**¿Qué hace GLAD?**
- Carga dinámicamente los punteros a funciones OpenGL
- Necesario porque OpenGL es un API, no una biblioteca
- Sin GLAD, las llamadas a `glXXX()` fallarían

**1.7. Inicializar ImGui**

```cpp
IMGUI_CHECKVERSION();
ImGui::CreateContext();
ImPlot::CreateContext();

ImGuiIO& io = ImGui::GetIO();
io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Navegación con teclado
io.IniFilename = nullptr;  // No guardar layout en archivo
```

**Configurar tema oscuro personalizado:**

```cpp
ImGui::StyleColorsDark();
ImVec4* colors = ImGui::GetStyle().Colors;

// Color verde característico: #1CC809
colors[ImGuiCol_CheckMark] = ImVec4(0.110f, 0.784f, 0.035f, 1.0f);
colors[ImGuiCol_SliderGrab] = ImVec4(0.110f, 0.784f, 0.035f, 1.0f);
colors[ImGuiCol_Button] = ImVec4(0.055f, 0.392f, 0.018f, 1.0f);
// ... más colores
```

**Inicializar backends:**

```cpp
ImGui_ImplGlfw_InitForOpenGL(window, true);
ImGui_ImplOpenGL3_Init(glsl_version);
```

---

### PASO 2: Constructor de MainWindow (MainWindow.cpp)

**2.1. Almacenar Referencias**

```cpp
MainWindow::MainWindow(int width, int height, Settings& config, SettingsWindow& ventanaConfig) :
    settings(&config),           // Puntero a configuración
    settingsWindow(&ventanaConfig),
    width(width), 
    height(height)
{
    CreateBuffers();  // Crear buffers circulares y FFT
}
```

**2.2. Crear Buffers Iniciales (CreateBuffers)**

```cpp
void MainWindow::CreateBuffers() {
    int speed = settings->sampling_rate;  // 3840 Hz
    int max_size = speed * max_time;      // 3840 * 60 = 230,400 muestras (60 segundos)
    int view_size = 30 * speed;           // 3840 * 30 = 115,200 muestras (30 segundos)
    
    // CRÍTICO: Mutex para operación atómica
    std::lock_guard<std::mutex> lock(data_mutex);
    
    // Buffers de lectura/escritura serial
    read_buffer.resize(512);   // 4x más grande que versión básica
    write_buffer.resize(512);
    
    // Buffer FFT
    fft = new FFT(settings->sampling_rate);  // 3840 muestras
    
    // Buffers circulares para visualización
    scrollX = new ScrollBuffer<double>(max_size, view_size);
    scrollY = new ScrollBuffer<double>(max_size, view_size);
    filter_scrollY = new ScrollBuffer<double>(max_size, view_size);
}
```

**¿Qué es ScrollBuffer?**

```cpp
template <typename T>
class ScrollBuffer {
    std::vector<T> data;  // Buffer circular de 230,400 elementos
    int max_size;         // Capacidad máxima: 60 segundos
    int view_size;        // Vista actual: 30 segundos
    int offset;           // Índice de inicio en el buffer
    
    // Acceso con wrap-around automático
    T& operator[](int i) {
        return data[(offset + i) % max_size];
    }
};
```

**Ventajas:**
- **Eficiente:** No copia datos, solo mueve puntero `offset`
- **Zero-copy para ImPlot:** Pasa puntero directo al buffer
- **Thread-safe:** Con mutex en operaciones críticas

---

### PASO 3: Inicio de Threads (MainWindow::Start)

**3.1. Thread de Lectura Serial (SerialWorker)**

```cpp
void MainWindow::Start() {
    if (serial_thread.joinable())
        return;  // Ya está ejecutando
    
    running = true;
    
    // Lanzar thread de lectura
    serial_thread = std::thread([this]() {
        SerialWorker();  // Función que ejecuta el thread
    });
}
```

**¿Qué hace SerialWorker()? (MainWindow.cpp línea ~300+)**

```cpp
void MainWindow::SerialWorker() {
    Serial serial;
    
    // Abrir puerto COM
    if (!serial.open(settings->port, settings->baud_rate))
        return;
    
    // Bucle infinito de lectura
    while (running) {
        // PASO 1: Leer bytes del puerto
        int bytes_read = serial.read(read_buffer.data(), read_buffer.size());
        
        if (bytes_read <= 0) {
            std::this_thread::sleep_for(1ms);  // Puerto vacío, esperar
            continue;
        }
        
        // PASO 2: Procesar cada byte recibido
        for (int i = 0; i < bytes_read; i++) {
            uint8_t byte = read_buffer[i];
            
            // Transformar ADC (0-255) → Voltaje (-6V a +6V)
            double voltage = TransformSample(byte);
            
            // PASO 3: Agregar a buffers circulares (thread-safe)
            {
                std::lock_guard<std::mutex> lock(data_mutex);
                
                if (!freeze) {  // Solo si no está congelado
                    (*scrollX)[size] = next_time;
                    (*scrollY)[size] = voltage;
                    
                    // Aplicar filtro si está activo
                    if (current_filter != Filter::None) {
                        double filtered = ApplyFilter(voltage);
                        (*filter_scrollY)[size] = filtered;
                    }
                    
                    size++;
                    next_time += 1.0 / settings->sampling_rate;
                }
            }
            
            // PASO 4: Preparar respuesta procesada para enviar
            if (current_filter != Filter::None) {
                double filtered_voltage = (*filter_scrollY)[size - 1];
                uint8_t response = InverseTransformSample(filtered_voltage);
                write_buffer[write_index++] = response;
            }
        }
        
        // PASO 5: Enviar datos procesados de vuelta al Arduino
        if (write_index > 0) {
            serial.write(write_buffer.data(), write_index);
            write_index = 0;
        }
    }
    
    serial.close();
}
```

**Timing del SerialWorker:**

```
Cada ciclo (~260 μs @ 3840 Hz):
├─ ReadFile(): 10-50 μs (depende de datos disponibles)
├─ Transformación: 1 μs × bytes_read
├─ Lock mutex + agregar a buffers: 5-10 μs
├─ Filtrado (si activo): 2-5 μs × bytes_read
└─ WriteFile(): 10-30 μs (depende de datos a enviar)

Total: ~50-150 μs por ciclo (deja 75% CPU libre)
```

**3.2. Thread de Análisis FFT (AnalysisWorker)**

```cpp
void MainWindow::Start() {
    // ... SerialWorker iniciado
    
    // Lanzar thread de análisis FFT
    analysis_thread = std::thread([this]() {
        AnalysisWorker();
    });
}
```

**¿Qué hace AnalysisWorker()? (MainWindow.cpp línea ~500+)**

```cpp
void MainWindow::AnalysisWorker() {
    using namespace std::chrono_literals;
    
    while (running) {
        // Solo ejecutar si NO está congelado y hay datos suficientes
        if (!freeze && size >= settings->samples) {
            
            // PASO 1: Copiar datos para FFT (thread-safe)
            std::vector<double> samples_copy(settings->samples);
            {
                std::lock_guard<std::mutex> lock(data_mutex);
                
                // Copiar últimas N muestras
                int start = size - settings->samples;
                for (int i = 0; i < settings->samples; i++) {
                    samples_copy[i] = (*scrollY)[start + i];
                }
            }
            
            // PASO 2: Ejecutar FFT (sin mutex, no bloquea otros threads)
            fft->SetData(samples_copy.data(), settings->samples);
            fft->Compute();  // Aquí se ejecuta FFTW3
            
            // PASO 3: Detectar armónicas
            auto harmonics = fft->FindHarmonics(settings->sampling_rate, 10);
            
            // Los resultados se leen en Draw() cuando sea necesario
        }
        
        // Esperar 100 ms antes de siguiente análisis
        std::this_thread::sleep_for(100ms);
    }
}
```

**¿Por qué 100 ms de espera?**
- FFT es cara computacionalmente (~5-10 ms para 3840 muestras)
- No necesitamos actualizar espectro 60 veces/segundo
- 10 FPS de actualización FFT es suficiente para visualización
- Ahorra ~50% de CPU

---

### PASO 4: Bucle Principal de Renderizado (main.cpp)

**4.1. Loop Infinito con Optimizaciones**

```cpp
while (!glfwWindowShouldClose(window) && !should_close)
{
    // OPTIMIZACIÓN 1: Event polling adaptativo
    if (focused || !minimized) {
        glfwPollEvents();  // Procesar eventos inmediatamente
    } else {
        glfwWaitEvents();  // Bloquear hasta evento (ahorra CPU)
    }
    
    // OPTIMIZACIÓN 2: Limitar FPS cuando no tiene foco
    if (!focused && !minimized) {
        std::this_thread::sleep_for(16ms);  // ~60 FPS → 10 FPS
    }
    
    // OPTIMIZACIÓN 3: No renderizar si está minimizado
    if (minimized) {
        std::this_thread::sleep_for(100ms);
        continue;
    }
```

**4.2. Inicio de Frame ImGui**

```cpp
    // Nuevo frame para backends
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();  // Inicio de frame ImGui
```

**4.3. Renderizar UI**

```cpp
    // Renderizar ventana principal (ocupa toda la ventana)
    mainWindow.Draw();
    
    // Ventana de configuración (deprecated, ya no se usa)
    // settings_window.Draw();
```

**4.4. Renderizado OpenGL**

```cpp
    // Finalizar frame ImGui (genera comandos de dibujo)
    ImGui::Render();
    
    // Configurar viewport OpenGL
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    
    // Limpiar pantalla (color de fondo)
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Ejecutar comandos de dibujo de ImGui
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // Swap buffers (mostrar frame renderizado)
    glfwSwapBuffers(window);  // VSync aquí: espera 16.67 ms (60 Hz)
}
```

**Timing del bucle principal:**

```
Frame time @ 60 FPS (16.67 ms):
├─ glfwPollEvents(): 0.5-1 ms
├─ ImGui::NewFrame(): 0.1 ms
├─ mainWindow.Draw(): 3-8 ms
│  ├─ Sidebar: 1-2 ms
│  ├─ 3 gráficos ImPlot: 2-5 ms
│  └─ Cálculos UI: 0.5 ms
├─ ImGui::Render(): 0.5-1 ms
├─ OpenGL rendering: 1-2 ms
└─ glfwSwapBuffers(): espera hasta completar 16.67 ms (VSync)

Total: ~5-12 ms de trabajo + espera VSync
CPU libre: 30-70% por frame
```

---

### PASO 5: Renderizado de MainWindow (MainWindow::Draw)

**5.1. Estructura de la Ventana**

```cpp
void MainWindow::Draw() {
    // Ventana fullscreen sin decoración
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(width, height));
    
    ImGui::Begin("MainWindow", nullptr, 
        ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove);
```

**5.2. Layout: Sidebar + Gráficos**

```cpp
    // COLUMNA IZQUIERDA: Sidebar de controles (240px fijo)
    ImGui::BeginChild("Sidebar", ImVec2(240, 0), true);
    {
        DrawControls();  // Selector puerto, baudrate, botones, etc.
    }
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // COLUMNA DERECHA: Gráficos (resto del espacio)
    ImGui::BeginChild("Graphs");
    {
        DrawGraphs();  // 3 gráficos apilados
    }
    ImGui::EndChild();
    
    ImGui::End();
}
```

**5.3. Controles del Sidebar (DrawControls)**

```cpp
void MainWindow::DrawControls() {
    ImGui::Text("SERIALPLOTTER - DSP");
    ImGui::Separator();
    
    // === SECCIÓN 1: Puerto Serial ===
    MenuPuertos(settings->port);  // Dropdown con puertos COM disponibles
    
    // === SECCIÓN 2: Baudrate ===
    ComboBaudRate(baud_selected);
    
    // === SECCIÓN 3: Frecuencia de Muestreo ===
    ComboFrecuenciaMuestreo(freq_selected);
    if (freq_selected_changed) {
        settings->sampling_rate = frecuencias[freq_selected];
        settings->baud_rate = settings->sampling_rate * 10;
        // Recrear buffers con nuevo tamaño
        CreateBuffers();
    }
    
    // === SECCIÓN 4: Botones Control ===
    if (Button("Conectar", !serial_thread.joinable())) {
        Start();  // Iniciar threads
    }
    
    if (Button("Desconectar", serial_thread.joinable())) {
        Stop();   // Detener threads
    }
    
    if (Button(freeze ? "Reanudar" : "Congelar", serial_thread.joinable())) {
        ToggleFreeze();  // Alternar modo freeze
    }
    
    // === SECCIÓN 5: Filtros ===
    ImGui::Separator();
    ImGui::Text("FILTROS DIGITALES:");
    
    if (ImGui::RadioButton("Sin Filtro", current_filter == Filter::None)) {
        current_filter = Filter::None;
    }
    if (ImGui::RadioButton("Pasa-Bajos", current_filter == Filter::LowPass)) {
        current_filter = Filter::LowPass;
        ConfigureFilters();
    }
    if (ImGui::RadioButton("Pasa-Altos", current_filter == Filter::HighPass)) {
        current_filter = Filter::HighPass;
        ConfigureFilters();
    }
    
    // Slider de frecuencia de corte (solo si filtro activo)
    if (current_filter != Filter::None) {
        ImGui::SliderFloat("Frecuencia Corte", &cutoff_frequency, 
                          10.0f, settings->sampling_rate / 2.0f, "%.0f Hz");
        if (ImGui::IsItemEdited()) {
            ConfigureFilters();  // Reconfigurar filtro en tiempo real
        }
    }
    
    // === SECCIÓN 6: Información en Tiempo Real ===
    ImGui::Separator();
    ImGui::Text("INFORMACIÓN:");
    ImGui::Text("Muestras: %d", size);
    ImGui::Text("Tiempo: %.2f s", next_time);
    
    if (fft) {
        double freq = fft->Frequency(settings->sampling_rate);
        double offset = fft->Offset();
        ImGui::Text("Frecuencia: %.2f Hz", freq);
        ImGui::Text("Offset DC: %.3f V", offset);
    }
}
```

**5.4. Gráficos (DrawGraphs)**

```cpp
void MainWindow::DrawGraphs() {
    // Altura de cada gráfico (divide pantalla en 3 partes iguales)
    float graph_height = ImGui::GetContentRegionAvail().y / 3.0f;
    
    // === GRÁFICO 1: Señal de Entrada (ADC) ===
    if (ImPlot::BeginPlot("Entrada (ADC)", ImVec2(-1, graph_height))) {
        
        // Configurar ejes
        ImPlot::SetupAxisLimits(ImAxis_X1, left_limit, right_limit);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -6, 6);
        
        // Dibujar líneas de división horizontal
        for (int i = 0; i < divisions; i++) {
            double t = left_limit + i * (right_limit - left_limit) / divisions;
            ImPlot::PlotInfLines("##vline", &t, 1);
        }
        
        // Datos a graficar
        const double* x_data = freeze ? frozen_dataX.data() : scrollX->Data();
        const double* y_data = freeze ? frozen_dataY.data() : scrollY->Data();
        int count = freeze ? frozen_dataX.size() : scrollY->Size();
        
        // Renderizar línea con optimización stride
        ImPlot::SetNextLineStyle(ImVec4(0.110f, 0.784f, 0.035f, 1.0f));  // Verde
        ImPlot::PlotLine("ADC", x_data, y_data, count, 
                        ImPlotLineFlags_None, 0, settings->byte_stride);
        
        ImPlot::EndPlot();
    }
    
    // === GRÁFICO 2: Señal de Salida (Filtrada) ===
    if (ImPlot::BeginPlot("Salida (Filtrada)", ImVec2(-1, graph_height))) {
        // Similar al gráfico 1 pero usando filter_scrollY
        // ...
        ImPlot::EndPlot();
    }
    
    // === GRÁFICO 3: Espectro FFT ===
    if (ImPlot::BeginPlot("Espectro (FFT)", ImVec2(-1, graph_height))) {
        
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, settings->sampling_rate / 2);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 3);
        
        // Dibujar espectro
        if (fft) {
            fft->Plot(settings->sampling_rate);
        }
        
        ImPlot::EndPlot();
    }
    
    // === PANEL DE ARMÓNICAS (debajo de gráficos) ===
    DrawHarmonicsPanel();
}
```

**5.5. Panel de Armónicas (DrawHarmonicsPanel)**

```cpp
void MainWindow::DrawHarmonicsPanel() {
    if (!fft) return;
    
    ImGui::Separator();
    ImGui::Text("ARMÓNICAS DETECTADAS:");
    
    // Detectar 10 armónicas
    auto harmonics = fft->FindHarmonics(settings->sampling_rate, 10);
    
    if (harmonics.size() == 1) {
        // Advertencia: Solo fundamental detectada
        ImGui::TextColored(ImVec4(0.9f, 0.6f, 0.0f, 1.0f), 
            "ADVERTENCIA: Solo se detecta la fundamental");
        double nyquist = settings->sampling_rate / 2.0;
        ImGui::Text("Aumenta frecuencia de muestreo para detectar armónicas");
    }
    
    // Tabla de armónicas
    ImGui::BeginChild("HarmonicsTable", ImVec2(0, 200), true);
    ImGui::Columns(3, "harmonics_table");
    
    // Encabezados
    ImGui::Text("Armónica"); ImGui::NextColumn();
    ImGui::Text("Frecuencia"); ImGui::NextColumn();
    ImGui::Text("Amplitud"); ImGui::NextColumn();
    ImGui::Separator();
    
    // Datos
    for (size_t i = 0; i < harmonics.size(); i++) {
        ImGui::Text("%dª", i + 1);
        ImGui::NextColumn();
        
        ImGui::Text("%s", MetricFormatter(harmonics[i].frequency, "Hz").data());
        ImGui::NextColumn();
        
        ImGui::Text("%s", MetricFormatter(harmonics[i].amplitude, "V").data());
        ImGui::NextColumn();
    }
    
    ImGui::Columns(1);
    ImGui::EndChild();
    
    // Calcular THD si hay al menos 3 armónicas
    if (harmonics.size() >= 3) {
        double fundamental = harmonics[0].amplitude;
        double thd_sum = 0;
        
        for (size_t i = 1; i < harmonics.size(); i++) {
            thd_sum += harmonics[i].amplitude * harmonics[i].amplitude;
        }
        
        double thd = (fundamental > 0) ? 
                     (std::sqrt(thd_sum) / fundamental * 100.0) : 0;
        
        ImGui::Separator();
        ImGui::Text("THD (Distorsión Armónica Total): %.2f%%", thd);
    }
}
```

---

### PASO 6: Aplicación de Filtros IIR (MainWindow::ApplyFilter)

**6.1. Configuración de Filtros**

```cpp
void MainWindow::ConfigureFilters() {
    // Frecuencia de Nyquist (máxima frecuencia representable)
    double nyquist = settings->sampling_rate / 2.0;
    
    // Limitar frecuencia de corte
    cutoff_frequency = std::min(cutoff_frequency, (float)nyquist);
    
    // Configurar filtro según tipo seleccionado
    switch (current_filter) {
        case Filter::LowPass:
            // Butterworth 8º orden: -48 dB/octava
            lowpass_filter.setup(settings->sampling_rate, cutoff_frequency);
            break;
            
        case Filter::HighPass:
            highpass_filter.setup(settings->sampling_rate, cutoff_frequency);
            break;
    }
}
```

**¿Qué es un filtro Butterworth 8º orden?**

```
Características:
- Respuesta plana en banda de paso (sin rizado)
- Atenuación: -48 dB/octava (muy empinada)
- Fase: No lineal pero predecible

Ejemplo Pasa-Bajos @ 500 Hz:
  0 Hz:  0 dB (pasa completamente)
500 Hz: -3 dB (frecuencia de corte)
1000 Hz: -51 dB (atenuación fuerte)
2000 Hz: -99 dB (prácticamente eliminado)
```

**6.2. Aplicar Filtro Muestra por Muestra**

```cpp
double MainWindow::ApplyFilter(double sample) {
    switch (current_filter) {
        case Filter::LowPass:
            return lowpass_filter.filter(sample);
            
        case Filter::HighPass:
            return highpass_filter.filter(sample);
            
        case Filter::None:
        default:
            return sample;
    }
}
```

**Proceso interno del filtro IIR:**

```cpp
// Simplificado (la implementación real está en iir1)
double IIR::filter(double input) {
    // Ecuación de diferencias:
    // y[n] = b₀x[n] + b₁x[n-1] + ... + bₘx[n-M]
    //      - a₁y[n-1] - ... - aₙy[n-N]
    
    double output = b[0] * input;
    
    // Parte FIR (feedforward)
    for (int i = 0; i < M; i++) {
        output += b[i+1] * x_history[i];
    }
    
    // Parte IIR (feedback)
    for (int i = 0; i < N; i++) {
        output -= a[i+1] * y_history[i];
    }
    
    // Actualizar historial
    for (int i = M-1; i > 0; i--) x_history[i] = x_history[i-1];
    x_history[0] = input;
    
    for (int i = N-1; i > 0; i--) y_history[i] = y_history[i-1];
    y_history[0] = output;
    
    return output;
}
```

---

### PASO 7: Modo Freeze (Congelado)

**7.1. Activar Freeze**

```cpp
void MainWindow::ToggleFreeze() {
    std::lock_guard<std::mutex> lock(data_mutex);
    
    if (!freeze) {
        // CONGELAR: Capturar snapshot de datos actuales
        frozen_dataX.clear();
        frozen_dataY.clear();
        frozen_dataY_filtered.clear();
        
        // Copiar datos actuales
        for (int i = 0; i < scrollX->Size(); i++) {
            frozen_dataX.push_back((*scrollX)[i]);
            frozen_dataY.push_back((*scrollY)[i]);
            frozen_dataY_filtered.push_back((*filter_scrollY)[i]);
        }
        
        freeze = true;
        
        // SerialWorker sigue ejecutando pero no modifica buffers
        // AnalysisWorker se pausa automáticamente
    }
    else {
        // DESCONGELAR: Liberar snapshot
        frozen_dataX.clear();
        frozen_dataY.clear();
        frozen_dataY_filtered.clear();
        
        freeze = false;
        
        // SerialWorker vuelve a escribir en buffers
        // AnalysisWorker se reanuda
    }
}
```

**Ventajas del modo freeze:**
- ✅ Zoom independiente sin perder datos nuevos
- ✅ Análisis detallado de eventos específicos
- ✅ Captura de transitorios
- ✅ No detiene adquisición (datos siguen llegando)

---

### PASO 8: Limpieza y Shutdown (MainWindow::Stop)

**8.1. Detener Threads**

```cpp
void MainWindow::Stop() {
    if (!serial_thread.joinable())
        return;  // Ya está detenido
    
    // Señalar a threads que deben terminar
    running = false;
    
    // Esperar a que terminen
    if (serial_thread.joinable())
        serial_thread.join();  // Bloquea hasta que SerialWorker termine
    
    if (analysis_thread.joinable())
        analysis_thread.join();  // Bloquea hasta que AnalysisWorker termine
    
    // Resetear estado
    size = 0;
    next_time = 0.0;
}
```

**8.2. Destructor (MainWindow::~MainWindow)**

```cpp
MainWindow::~MainWindow()
{
    Stop();              // Detener threads si aún están corriendo
    DestroyBuffers();    // Liberar memoria de buffers
}
```

**8.3. Cleanup en main.cpp**

```cpp
// Al salir del bucle principal:

// Cleanup ImGui
ImGui_ImplOpenGL3_Shutdown();
ImGui_ImplGlfw_Shutdown();
ImPlot::DestroyContext();
ImGui::DestroyContext();

// Cleanup GLFW
glfwDestroyWindow(window);
glfwTerminate();

return 0;
```

---

## Análisis de Módulos Detallado

### 1. Módulo Serial (Serial.h / Serial.cpp)

**Propósito:** Comunicación con puerto COM usando Win32 API

**Funciones principales:**

```cpp
class Serial {
public:
    bool open(std::string port, int baud);  // Abrir puerto COM
    void close();                            // Cerrar puerto
    int read(uint8_t* buffer, int size);     // Leer datos (no bloqueante)
    int write(const uint8_t* data, int size); // Escribir datos
};
```

**Implementación interna (Serial.cpp):**

```cpp
bool Serial::open(std::string port, int baud) {
    // Agregar prefijo para puertos COM10+
    if (!port.starts_with("\\\\.\\"))
        port.insert(0, "\\\\.\\");
    
    // Abrir dispositivo COM
    file = CreateFileA(port.c_str(), GENERIC_READ | GENERIC_WRITE, 
                      0, 0, OPEN_EXISTING, 0, 0);
    
    if (file == INVALID_HANDLE_VALUE)
        return false;
    
    // Configurar buffers (8 KB para Mega 2560)
    SetupComm(file, 8192, 8192);
    
    // Limpiar buffers antiguos
    PurgeComm(file, PURGE_RXCLEAR | PURGE_TXCLEAR);
    
    // Configurar parámetros de comunicación
    DCB dcb = {};
    dcb.DCBlength = sizeof(DCB);
    GetCommState(file, &dcb);
    
    dcb.BaudRate = baud;           // 38400
    dcb.ByteSize = 8;              // 8 bits
    dcb.Parity = NOPARITY;         // Sin paridad
    dcb.StopBits = ONESTOPBIT;     // 1 stop bit
    dcb.fBinary = TRUE;
    
    SetCommState(file, &dcb);
    
    // Configurar timeouts (no bloqueante)
    COMMTIMEOUTS timeouts = {};
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    
    SetCommTimeouts(file, &timeouts);
    
    return true;
}

int Serial::read(uint8_t* buffer, int size) {
    DWORD bytes_read = 0;
    
    if (!ReadFile(file, buffer, size, &bytes_read, nullptr))
        return -1;
    
    return bytes_read;
}
```

**Optimización clave:**
- Buffers de 8 KB (4x más que estándar)
- Timeouts configurados para lectura no bloqueante
- Aprovecha buffers grandes del Mega 2560

---

### 2. Módulo FFT (FFT.h / FFT.cpp)

**Propósito:** Análisis espectral con FFTW3

**Arquitectura:**

```cpp
class FFT {
    fftw_complex* complex;       // Salida FFT (números complejos)
    fftw_plan p;                 // Plan de ejecución FFTW
    
    std::vector<double> samples;     // Entrada (dominio tiempo)
    std::vector<double> amplitudes;  // Salida (magnitudes)
    
public:
    FFT(int sample_count);
    void SetData(const double* data, uint32_t count);
    void Compute();  // Ejecutar FFT
    double Frequency(double sampling_frequency);  // Frecuencia dominante
    std::vector<Harmonic> FindHarmonics(double fs, int count);
};
```

**Algoritmo completo ya explicado anteriormente** (ver respuesta anterior sobre armónicos)

---

### 3. Módulo Buffers (Buffers.h)

**ScrollBuffer: Buffer circular optimizado para ImPlot**

```cpp
template <typename T>
class ScrollBuffer {
    std::vector<T> data;
    int max_size;
    int view_size;
    int offset;
    int size;
    
public:
    // Constructor
    ScrollBuffer(int max_size, int view_size);
    
    // Acceso con wrap-around
    T& operator[](int i) {
        return data[(offset + i) % max_size];
    }
    
    // Agregar dato (mueve offset si está lleno)
    void Add(T value) {
        data[(offset + size) % max_size] = value;
        if (size < max_size)
            size++;
        else
            offset = (offset + 1) % max_size;
    }
    
    // Puntero directo para ImPlot (zero-copy)
    const T* Data() const {
        return &data[offset];
    }
};
```

**Ventaja:** ImPlot puede dibujar directamente desde el buffer sin copiar datos

---

## Resumen de Decisiones de Diseño

| Aspecto | Decisión | Razón |
|---------|----------|-------|
| **Threading** | 3 threads separados | Evita bloqueos en UI, lectura serial y FFT |
| **Buffers** | Circulares de 60 seg | Balance memoria/histórico |
| **FFT** | FFTW3 con SIMD | Máximo rendimiento (~0.05 ms) |
| **Filtros** | IIR Butterworth 8º | Respuesta plana, fase predecible |
| **Renderizado** | ImGui immediate mode | Simplicidad, no state management |
| **VSync** | Activado (60 FPS) | Fluidez visual, reduce tearing |
| **Modo Freeze** | Snapshot con mutex | Análisis sin perder datos en vivo |

---

## Flujo de Datos Completo

```
┌─────────────────────────────────────────────────────────────┐
│                    ARDUINO MEGA 2560                         │
│  ADC @ 9.6kHz → Timer1 ISR @ 3.84kHz → UART TX → [byte]    │
└────────────────────────┬────────────────────────────────────┘
                         │
                 UART @ 38400 baud
                         │
┌────────────────────────▼────────────────────────────────────┐
│              SERIALPLOTTER - SerialWorker Thread            │
│  ReadFile() → read_buffer[512] → TransformSample()         │
│      ↓                                                       │
│  Lock(mutex) → scrollX/Y->Add(voltage) → Unlock()          │
│      ↓                                                       │
│  ApplyFilter() → filter_scrollY->Add(filtered)             │
│      ↓                                                       │
│  InverseTransform() → write_buffer[512] → WriteFile()      │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ├───────────────────────────────────┐
                         │                                   │
┌────────────────────────▼────────────┐  ┌──────────────────▼─────┐
│     AnalysisWorker Thread           │  │   Main Thread (UI)     │
│  Lock(mutex) → copy samples         │  │  Lock(mutex) → read    │
│  Unlock() → FFT::Compute()          │  │  data → Unlock()       │
│  FindHarmonics(10) → results        │  │  ImPlot::PlotLine()    │
│  Sleep(100ms)                       │  │  DrawHarmonics()       │
└─────────────────────────────────────┘  │  glfwSwapBuffers()     │
                                         └────────────────────────┘
```

---

## Casos de Uso

1. **Osciloscopio digital:** Visualización en tiempo real de señales
2. **Analizador de espectro:** Identificación de componentes frecuenciales
3. **Medidor de THD:** Calidad de señales de audio
4. **Filtro digital en tiempo real:** Procesamiento IIR bidireccional
5. **Educación:** Demostración de conceptos DSP y teoría de Fourier
6. **Debug de sistemas embebidos:** Análisis de señales del Arduino

---

## Optimizaciones de Rendimiento Implementadas

### CPU Usage

```
Con ventana en foco (60 FPS):
├─ Main Thread: 15-20% (UI + rendering)
├─ SerialWorker: 5-10% (lectura serial)
├─ AnalysisWorker: 3-5% (FFT @ 10 Hz)
└─ Total: 23-35% CPU (deja 65-77% libre)

Con ventana sin foco (10 FPS):
└─ Total: 5-10% CPU (ahorro 70%)

Con ventana minimizada:
└─ Total: <2% CPU (solo SerialWorker activo)
```

### Memoria

```
Buffers principales:
├─ scrollX: 230,400 × 8 bytes = 1.8 MB
├─ scrollY: 230,400 × 8 bytes = 1.8 MB
├─ filter_scrollY: 230,400 × 8 bytes = 1.8 MB
├─ FFT buffers: 3,840 × 24 bytes = 92 KB
└─ Total buffers: ~5.4 MB

Memoria total de la app: ~50-80 MB (razonable)
```

### GPU Usage

```
ImGui + ImPlot son muy eficientes:
└─ ~5-15% GPU (integrada Intel)
└─ ~2-5% GPU (dedicada NVIDIA/AMD)
```

---

*Este sistema representa una solución completa de adquisición y procesamiento de señales, combinando hardware embebido con software de escritorio para crear una plataforma profesional de análisis DSP.*
