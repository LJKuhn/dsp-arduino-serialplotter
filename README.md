# DSP System — Arduino Mega 2560 + SerialPlotter

> Trabajo Práctico Final — Procesamiento Digital de Señales  
> Ingeniería en Computación · Universidad Nacional de Rafaela  
> Lautaro Kühn & Federico Domínguez · Prof. Milton Pozzo · 2026

Sistema completo de procesamiento digital de señales en tiempo real. Combina un circuito de acondicionamiento analógico, firmware embebido en C++ para Arduino Mega 2560, y una aplicación de escritorio desarrollada en C++17 con interfaz gráfica, análisis espectral (FFT) y filtros digitales IIR.

---

## ¿Qué hace este proyecto?

Una señal analógica senoidal de hasta ±6 V es acondicionada, digitalizada, enviada a la PC, procesada y reconstruida en tiempo real:

```
Señal analógica (±6V)
       │
       ▼
┌─────────────────┐
│  Acondicionador │  Divisor resistivo + LM324
│  de señal       │  → Adapta la señal a 0.8V–3.8V
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Arduino Mega   │  ADC free-running @ 9.6 kHz (prescaler 128)
│  2560           │  Timer1 @ 3840 Hz → muestreo + UART TX
│                 │  DAC R2R 8 bits (PORTA, pines 22–29)
└────────┬────────┘
         │ UART 38400 baud (bidireccional)
         ▼
┌─────────────────┐
│  SerialPlotter  │  Aplicación C++17 con OpenGL + ImGui
│  (PC)           │  · FFT en tiempo real (FFTW3)
│                 │  · Filtros Butterworth IIR orden 8
│                 │  · Detección de armónicas + THD
│                 │  · Visualización temporal y espectral
└─────────────────┘
```

---

## Tecnologías utilizadas

| Capa | Tecnología |
|------|-----------|
| **Microcontrolador** | Arduino Mega 2560 (ATmega2560), C++ bare-metal |
| **Interfaz PC** | C++17, CMake, Ninja |
| **GUI** | ImGui + ImPlot + GLFW + OpenGL (GLAD) |
| **FFT** | FFTW3 |
| **Filtros digitales** | IIR1 (Butterworth orden 8) |
| **Comunicación** | UART asincrónico, buffers circulares con ISR |
| **Hardware analógico** | LM324, red R2R, divisor resistivo |

---

## Características destacadas

- **Muestreo determinístico**: Timer1 de 16 bits genera interrupciones exactas a 3840 Hz sin jitter dependiente del software.
- **ADC en modo Free Running**: El conversor opera a ~9.6 kHz de forma autónoma (prescaler 128, 13 ciclos/conversión), proveyendo oversampling 2.5× respecto al rate de muestreo.
- **Reducción 10→8 bits con ADLAR**: Activando el bit ADLAR los 8 bits más significativos quedan directamente en ADCH, eliminando la necesidad de combinar registros y reduciendo el tráfico serie a la mitad.
- **UART sin bloqueo**: Buffer circular de 256 bytes con ISR `USART0_UDRE_vect` que se auto-desactiva cuando no hay datos pendientes, minimizando el overhead de CPU.
- **Sincronización perfecta baudrate/muestreo**: 3840 muestras/s × 10 bits de trama UART = 38400 baud exactos, sin pérdida de datos.
- **Análisis FFT en tiempo real**: Detección automática de frecuencia fundamental, las primeras 5 armónicas y cálculo de THD (Distorsión Armónica Total).
- **Filtros Butterworth IIR orden 8**: Pasa-bajos y pasa-altos configurables desde la interfaz, aplicados muestra a muestra y retornados al DAC del Arduino.

---

## Especificaciones técnicas

| Parámetro | Valor |
|-----------|-------|
| Frecuencia de muestreo | 3840 Hz |
| Baudrate UART | 38400 bps |
| Resolución transmitida | 8 bits (256 niveles) |
| Latencia extremo a extremo | ~0.6–0.8 ms |
| Rango efectivo de entrada | 0.8 V – 3.8 V |
| Resolución efectiva ADC | ~11.7 mV/LSB |
| Resolución DAC R2R | ~19.5 mV/LSB |
| Filtros digitales | Butterworth IIR orden 8 |
| Frecuencia máxima analizable | 1920 Hz (Nyquist) |

---

## Estructura del repositorio

```
├── DSP/                        # Firmware principal (Arduino Mega 2560)
│   ├── adc.cpp / adc.h         # Control del ADC en modo Free Running
│   ├── timer1.h                # Timer1 a 3840 Hz para muestreo preciso
│   ├── usart.h                 # UART asincrónico con buffer circular
│   ├── DSP.ino                 # Sketch principal
│   └── tablas.h, prescaler.h   # Tablas de configuración
│
├── SerialPlotter/
│   ├── src/                    # Código fuente C++ de la aplicación PC
│   ├── extern/                 # Dependencias (FFTW3, GLFW, ImGui, IIR1)
│   ├── CMakeLists.txt
│   └── README.md               # Documentación detallada de la interfaz
│
├── img-wwpp/                   # Imágenes del proyecto (circuito físico)
├── DSP_Completo_UNIFICADO.md   # Informe técnico completo
├── DEPENDENCIES.md             # Instrucciones de dependencias
└── README.md                   # Este archivo
```

---

## Compilar y ejecutar

### Firmware (Arduino Mega 2560)
Abrir `DSP/DSP.ino` con Arduino IDE y subir al microcontrolador.

### Aplicación PC (Windows)
```bash
# Instalar dependencias (ver DEPENDENCIES.md)
cd SerialPlotter
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/SerialPlotter.exe
```

Conectar el Arduino, seleccionar el puerto COM en la interfaz y comenzar a adquirir señales.

---

## Hardware requerido

- Arduino Mega 2560
- Amplificador operacional LM324 + resistencias para acondicionamiento
- Red resistiva R2R (8 bits) para DAC
- Generador de señales o fuente de señal analógica

---

*Proyecto académico desarrollado como Trabajo Práctico Final de la materia Procesamiento Digital de Señales, Ingeniería en Computación, Universidad Nacional de Rafaela.*