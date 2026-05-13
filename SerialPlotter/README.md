# SerialPlotter

Aplicación de escritorio para Windows que funciona como interfaz de adquisición, visualización y procesamiento DSP en tiempo real para el sistema basado en Arduino Mega 2560.

SerialPlotter recibe muestras por UART, las transforma a voltaje, muestra la señal en tiempo real, ejecuta análisis FFT, aplica filtros IIR y puede reenviar la señal procesada al microcontrolador para su salida por DAC R2R.

## Qué hace

- Lee muestras de 8 bits enviadas por el Arduino a 38400 baud.
- Convierte esas muestras a voltaje según la calibración configurada.
- Muestra tres vistas simultáneas: entrada, salida filtrada y espectro FFT.
- Permite congelar la visualización sin detener la adquisición.
- Aplica filtros Butterworth IIR de orden 8 en tiempo real.
- Detecta frecuencia dominante, armónicas y THD.

## Tecnologías utilizadas

| Componente | Tecnología |
|---|---|
| Lenguaje | C++20 |
| Build | CMake 3.20+ |
| Generador recomendado | Ninja |
| GUI | Dear ImGui + ImPlot |
| Renderizado | GLFW + OpenGL + GLAD |
| FFT | FFTW3 |
| Filtros | IIR1 |
| Comunicación serial | Win32 API |
| Plataforma objetivo | Windows |

## Arquitectura real de la aplicación

El código actual implementa esta estructura:

- Hilo principal: interfaz gráfica y renderizado.
- Hilo serial: lectura continua del puerto COM, transformación ADC→voltaje y aplicación de filtro.
- Hilo de análisis: cálculo periódico de FFT sobre las últimas muestras.

Los datos se almacenan en buffers circulares separados para:

- eje temporal,
- señal de entrada,
- señal filtrada.

Cuando se activa el modo congelado, se toma un snapshot thread-safe de esos buffers para permitir inspección y zoom sin frenar la adquisición.

## Funcionalidad validada contra código

Esta documentación fue ajustada a la implementación real en `src/`:

- Estándar de compilación: C++20.
- Dos hilos de trabajo: serial y análisis.
- Filtros disponibles: ninguno, pasa-bajos Butterworth orden 8, pasa-altos Butterworth orden 8.
- Detección de hasta 10 armónicas desde la interfaz.
- Cálculo de THD cuando hay suficientes armónicas detectadas.
- Buffers de lectura y escritura de 512 bytes dentro de la aplicación PC.
- Frecuencia y baudrate configurables desde la interfaz, con valores por defecto de 3840 Hz y 38400 baud.
- Comunicación serial implementada exclusivamente con la API de Windows.

## Estructura del código fuente

```
SerialPlotter/
├── src/
│   ├── main.cpp / main.h        # Punto de entrada y loop principal
│   ├── MainWindow.cpp / .h      # UI, adquisición, filtros, freeze, FFT
│   ├── Serial.cpp / .h          # Puerto COM mediante Win32 API
│   ├── FFT.cpp / .h             # Cálculo de FFT y detección de armónicas
│   ├── Settings.cpp / .h        # Configuración del sistema
│   ├── Console.cpp / .h         # Consola auxiliar en Windows
│   ├── Buffers.h                # Buffers circulares
│   └── Widgets.h                # Widgets reutilizables de UI
│
├── include/                     # Headers públicos de dependencias
├── extern/                      # Dependencias externas
├── glad.c                       # Loader OpenGL
└── CMakeLists.txt               # Build principal
```

## Flujo de datos

```text
Arduino -> UART -> SerialWorker -> TransformSample()
        -> buffer de entrada -> filtro IIR opcional -> buffer de salida
        -> FFT worker -> análisis espectral
        -> UI con ImPlot
        -> InverseTransformSample() -> UART -> Arduino
```

## Compilación

Desde la raíz de esta carpeta:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Ejecutable esperado:

```bash
build/SerialPlotter.exe
```

## Uso básico

1. Ejecutar la aplicación.
2. Seleccionar el puerto COM del Arduino.
3. Configurar baudrate y frecuencia de muestreo.
4. Conectar.
5. Ajustar calibración, filtro y visualización según necesidad.

## Limitaciones y alcance

- La aplicación está orientada a Windows.
- La calibración ADC→voltaje depende del hardware real conectado.
- La resolución que recibe desde el Arduino es de 8 bits.
- El análisis de armónicas está limitado por la frecuencia de Nyquist configurada.

## Estado de la documentación

Antes había tres archivos Markdown con contenido duplicado y parcialmente inconsistente. Se unificó todo en este README para dejar una única fuente de verdad alineada con la implementación actual.

Para contexto general del sistema completo Arduino + PC, ver el README de la raíz del repositorio.
