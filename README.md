# Sistema DSP: Arduino Mega 2560 + SerialPlotter

Sistema completo de procesamiento digital de señales en tiempo real que combina hardware (Arduino Mega 2560) con software (SerialPlotter en C++) para análisis y filtrado de señales.

## 🎯 **Características Principales**

- **🔄 Procesamiento bidireccional**: ADC → PC → DAC en tiempo real
- **📊 Análisis espectral**: FFT en tiempo real con FFTW3
- **🔧 Filtros digitales**: Butterworth IIR de 8º orden (pasa-bajos, pasa-altos)
- **📈 Visualización**: Gráficas de entrada y salida simultáneas
- **⚙️ Configuración**: Frecuencias y parámetros ajustables por interfaz
- **🎛️ Generador de pruebas**: 6 tipos de señales automáticas para testing

## 🏗️ **Arquitectura del Sistema**

```
[Generador] → [Acondicionador LM324] → [Arduino ADC] → [UART 38400] → 
[SerialPlotter] → [Filtros IIR] → [FFT] → [UART] → [Arduino DAC] → [Salida]
```

## 📂 **Estructura del Proyecto**

### **DSP-arduino/**
- **DSP/**: Sistema principal para Arduino Mega 2560
  - Frecuencia: 3840 Hz, Baudrate: 38400
  - ADC: Pin A1, DAC: Pines 22-29 (PORTA)
  - Comunicación bidireccional no bloqueante
- **Arduino_Uno_Auto_Waveforms/**: Generador de señales de prueba
  - 12 estados: Triangular/Cuadrada/Senoidal × (2Hz/10Hz/80Hz/300Hz) × (1V-4V/0V-5V)
  - Cambio automático cada 15 segundos

### **SerialPlotter/**
- Interfaz gráfica en C++ con ImGui
- Filtros Butterworth de 8º orden
- Análisis FFT en tiempo real
- Configuración de mapeo ADC y calibración

### **Documentación/**
- **ANALISIS_COMPATIBILIDAD.md**: Problemas identificados y soluciones
- **EVALUACION_TP3.md**: Cumplimiento de objetivos del TP3
- **GUIA_CONFIGURACION.md**: Instrucciones paso a paso
- **MEJORAS_Y_TEORIA_TP3.md**: Fundamentos teóricos y mejoras

## 🚀 **Inicio Rápido**

### **1. Arduino Mega 2560**
```cpp
// Compilar y subir DSP-arduino/DSP/DSP.ino
// Configuración: 3840 Hz, 38400 baudios, Pin A1 entrada
```

### **2. SerialPlotter**
```bash
# IMPORTANTE: Instalar dependencias primero (ver DEPENDENCIES.md)
# Las bibliotecas externas no están incluidas en el repositorio

cd SerialPlotter
cmake -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
./build-release/SerialPlotter.exe
```

### **3. Configuración**
- Conectar Arduino al puerto USB
- En SerialPlotter: seleccionar puerto COM
- Calibrar mapeo ADC según hardware
- ¡Listo para procesar señales!

## 🔧 **Especificaciones Técnicas**

| Aspecto | Valor |
|---------|-------|
| **Frecuencia de muestreo** | 3840 Hz |
| **Comunicación** | 38400 baudios |
| **Resolución ADC/DAC** | 8 bits (0-255) |
| **Latencia del sistema** | ~0.6-0.8 ms |
| **Filtros** | Butterworth 8º orden |
| **Entrada** | Pin A1 (0V-5V) |
| **Salida** | Pines 22-29 PORTA |

## 🎓 **Aplicaciones Educativas**

- **Conceptos de DSP**: Muestreo, filtrado, análisis espectral
- **Sistemas embebidos**: Comunicación serie, interrupciones, timers
- **Procesamiento en tiempo real**: Latencia, buffering, sincronización
- **Análisis de señales**: FFT, filtros IIR, respuesta en frecuencia

## 📋 **Requisitos**

### **Hardware:**
- Arduino Mega 2560
- Circuito acondicionador (LM324)
- DAC R2R de 8 bits
- Fuente de señales o generador

### **Software:**
- Arduino IDE 1.8+
- CMake 3.20+
- Visual Studio 2019+ (Windows)
- Git

## 🏆 **Estado del Proyecto**

- ✅ **Compilación**: Sin errores
- ✅ **Comunicación**: 38400 baudios sincronizados
- ✅ **Filtros**: Butterworth funcionando
- ✅ **FFT**: Análisis espectral implementado
- ✅ **Documentación**: Completa y actualizada
- ✅ **Testing**: Generador automático de señales

---

**Desarrollado para**: Procesamiento Digital de Señales - Universidad  
**Versión**: 1.0  
**Licencia**: Educacional