# Documentación Técnica del Proyecto SerialPlotter

## Índice
1. [Visión General](#visión-general)
2. [Arquitectura del Sistema](#arquitectura-del-sistema)
3. [Estructura de Directorios](#estructura-de-directorios)
4. [Bibliotecas Externas](#bibliotecas-externas)
5. [Módulos del Proyecto](#módulos-del-proyecto)
6. [Flujo de Datos](#flujo-de-datos)
7. [Compilación y Configuración](#compilación-y-configuración)
8. [Detalles Técnicos](#detalles-técnicos)

---

## Visión General

**SerialPlotter** es una aplicación de Procesamiento Digital de Señales (DSP) en tiempo real que:

- **Captura** señales desde un microcontrolador vía puerto serie
- **Visualiza** las señales en tiempo real mediante gráficas interactivas
- **Analiza** las señales usando Transformada Rápida de Fourier (FFT)
- **Filtra** las señales con filtros IIR configurables (pasa-bajos, pasa-altos)
- **Congela** la visualización sin detener la adquisición (modo freeze)
- **Retransmite** las señales filtradas de vuelta al microcontrolador

### Tecnologías Principales
- **Lenguaje**: C++20
- **Sistema de Build**: CMake 3.20+
- **Generador**: Ninja
- **Plataforma**: Windows (usa API de Windows para comunicación serial)

---

## Arquitectura del Sistema

```
????????????????????????????????????????????????????
?                Aplicación Principal              ?
?                  (main.cpp)                      ?
????????????????????????????????????????????????????
             ?
      ???????????????
      ?             ?
 ???????????  ??????????????
 ?MainWindow?  ? Settings   ?
 ?  (GUI)   ?  ?  Window    ?
 ????????????  ??????????????
       ?
 ??????????????????????????????????
 ?                                 ?
?????????? ???????? ???????? ????????
? Serial ? ? FFT  ? ?Filter? ?Buffer?
?        ? ?      ? ?(IIR) ? ?      ?
?????????? ???????? ???????? ????????
```

### Hilos de Ejecución

1. **Hilo Principal**: Renderizado de la interfaz gráfica (ImGui + OpenGL)
2. **Hilo Serial**: Lectura/escritura continua del puerto serie
3. **Hilo de Análisis**: Cálculo de FFT en segundo plano

### Modo Freeze (Congelado)

El modo freeze permite **pausar la visualización sin detener la adquisición**:

- **Snapshot**: Al congelar, se copia un snapshot completo de los datos actuales
- **Zoom independiente**: Permite hacer zoom y análisis sin afectar la captura en vivo
- **Adquisición continua**: El SerialWorker sigue capturando datos en segundo plano
- **Thread-safety**: Usa mutex (data_mutex) para proteger la copia del snapshot
- **Reanudación**: Al descongelar, vuelve al modo en vivo sin pérdida de datos

---

## Estructura de Directorios

```
SerialPlotter/
?
??? CMakeLists.txt              # Configuración principal de CMake
??? README.md                   # Descripción básica del proyecto
??? DOCUMENTACION.md            # Este archivo
?
??? src/                        # Código fuente principal
?   ??? main.cpp                # Punto de entrada, bucle principal
?   ??? main.h                  # Declaraciones globales
?   ??? MainWindow.cpp/.h       # Ventana principal y lógica de la app
?   ??? Serial.cpp/.h           # Comunicación con puerto serie
?   ??? FFT.cpp/.h              # Transformada de Fourier
?   ??? Settings.cpp/.h         # Configuración y ventana de ajustes
?   ??? Console.cpp/.h          # Manejo de consola de Windows
?   ??? Buffers.h               # Buffers circulares (ScrollBuffer)
?   ??? Widgets.h               # Widgets personalizados de ImGui
?
??? include/                    # Headers públicos
?   ??? fftw3.h                 # Header de FFTW3
?
??? extern/                     # Bibliotecas externas
?   ??? fftw3/                  # Fast Fourier Transform library
?   ??? glfw/                   # Biblioteca de ventanas/input
?   ??? imgui/                  # Interfaz gráfica inmediata
?   ??? implot/                 # Gráficas para ImGui
?   ??? iir1/                   # Filtros IIR digitales
?
??? build-debug/                # Directorio de compilación Debug
??? build-release/              # Directorio de compilación Release
```

---

## Bibliotecas Externas

SerialPlotter depende de varias bibliotecas externas para su funcionamiento:

### 1. **GLFW** (`extern/glfw/`)
- **Propósito**: Gestión de ventanas, contextos OpenGL y entrada de usuario
- **Versión**: 3.x
- **Uso**: Crea la ventana principal y maneja eventos del sistema operativo
- **Licencia**: zlib/libpng

### 2. **glad** (`glad.c`)
- **Propósito**: Cargador de funciones OpenGL
- **Uso**: Inicializa las funciones OpenGL necesarias para el renderizado
- **Licencia**: MIT

### 3. **Dear ImGui** (`extern/imgui/`)
- **Propósito**: Interfaz gráfica de usuario en modo inmediato
- **Características**:
  - Sistema de ventanas, menús, botones
  - Sin dependencias de frameworks pesados
  - Ideal para herramientas y aplicaciones técnicas
- **Licencia**: MIT

### 4. **ImPlot** (`extern/implot/`)
- **Propósito**: Biblioteca de gráficas para ImGui
- **Características**:
  - Gráficas en tiempo real
  - Zoom, paneo, leyendas
  - Optimizado para grandes cantidades de datos
- **Uso en el proyecto**: Visualización de señales y espectros
- **Licencia**: MIT

### 5. **FFTW3** (`extern/fftw3/`)
- **Nombre completo**: Fastest Fourier Transform in the West
- **Propósito**: Cálculo eficiente de transformadas de Fourier
- **Características**:
  - Altamente optimizado (SSE2, AVX, AVX2)
  - Soporta transformadas reales y complejas
  - Planes precomputados para máxima eficiencia
- **Uso en el proyecto**: Análisis espectral de señales
- **Licencia**: GPL

### 6. **iir1** (`extern/iir1/`)
- **Propósito**: Filtros digitales IIR (Infinite Impulse Response)
- **Tipos de filtros incluidos**:
  - Butterworth (pasa-bajos, pasa-altos, pasa-banda)
  - Chebyshev tipo I y II
  - Filtros RBJ (Robert Bristow-Johnson)
- **Uso en el proyecto**: Filtrado de señales en tiempo real
- **Licencia**: Apache 2.0

---

## Módulos del Proyecto

SerialPlotter se compone de los siguientes módulos principales:

1. **Módulo de Configuración**: Maneja la configuración de la aplicación y las preferencias del usuario.
2. **Módulo de Interfaz Gráfica**: Responsable de renderizar la UI usando ImGui y OpenGL.
3. **Módulo Serial**: Encargado de la comunicación entre el PC y el microcontrolador vía puerto serie.
4. **Módulo FFT**: Realiza el cálculo de la Transformada Rápida de Fourier sobre los datos de entrada.
5. **Módulo de Filtros**: Aplica filtros IIR a las señales para su análisis.
6. **Módulo de Buffers**: Maneja los buffers circulares para el almacenamiento temporal de datos.

Cada módulo se implementa en archivos `.cpp` y sus correspondientes headers `.h` en el directorio `src/`. Los detalles de implementación se describen en las secciones siguientes.

---

## Flujo de Datos

El flujo de datos en SerialPlotter se describe a través de los siguientes pasos:

1. **Adquisición**: Los datos son adquiridos desde el puerto serie por el `SerialWorker`.
2. **Almacenamiento**: Los datos adquiridos se almacenan en buffers circulares gestionados por el `BufferManager`.
3. **Procesamiento**: Los datos son procesados en bloques por el `FFTAnalyzer` y el `IIRFilter`.
4. **Visualización**: Los resultados son enviados a la UI para su visualización en tiempo real.
5. **Retransmisión**: Opcionalmente, los datos filtrados pueden ser retransmitidos al microcontrolador.

Este ciclo se repite continuamente durante la operación normal de la aplicación.

### Recepción de Señal
```
Microcontrolador ? Puerto COM ? Serial::read()
                                     ?
                              TransformSample()
                              (ADC ? Voltaje)
                                     ?
                    ????????????????????????????????????
                    ?                                  ?
             scrollY (original)                 Filtro IIR
                    ?                                  ?
              Gráfica "Entrada"          filter_scrollY (filtrada)
                    ?                                  ?
                    ?                      Gráfica "Salida"
                    ?                                  ?
                    ?                    InverseTransformSample()
                    ?                      (Voltaje ? ADC)
                    ?                                  ?
                    ?                         Serial::write()
                    ?                                  ?
                    ?                         Microcontrolador
                    ?
              Modo Freeze:
              ? Snapshot
         frozen_dataX/Y
         (análisis sin perder datos)
```

### Análisis FFT
```
scrollY (últimas N muestras)
         ?
    FFT::SetData()
         ?
    FFT::Compute()
         ?
   Identifica:
   - Offset DC
   - Frecuencia dominante
   - Espectro completo
         ?
    FFT::Plot()
         ?
   Gráfica "Espectro"
   
   (Solo activo en modo en vivo)
```

---

## Compilación y Configuración

Para compilar y configurar SerialPlotter, siga los siguientes pasos:

1. Asegúrese de tener instaladas todas las dependencias y bibliotecas externas requeridas.
2. Clone el repositorio del proyecto desde GitHub.
3. Cree un directorio de compilación (por ejemplo, `build/`) dentro del directorio raíz del proyecto.
4. Desde el directorio de compilación, ejecute CMake para configurar el proyecto:
   ```bash
   cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
   ```
5. Compile el proyecto usando el comando:
   ```bash
   ninja
   ```
6. Ejecute la aplicación generada:
   ```bash
   ./SerialPlotter
   ```

Consulte la documentación de CMake y Ninja para más detalles sobre la configuración y compilación del proyecto.

---

## Detalles Técnicos

### Comunicación Serial

La comunicación serial se realiza a través de la clase `SerialPort`, que encapsula la API de Windows para el manejo de puertos serie. La clase maneja la configuración de la conexión (baud rate, paridad, bits de datos) y proporciona métodos para leer y escribir datos del puerto.

### `MainWindow.cpp` / `MainWindow.h`
**Responsabilidad**: Ventana principal y lógica central de la aplicación.

**Componentes**:

#### **Gestión de Datos**
- `ScrollBuffer<double>* scrollX, *scrollY, *filter_scrollY`: Buffers circulares para las muestras
- `FFT* fft`: Motor de análisis espectral
- `Serial serial`: Interfaz de comunicación serie

#### **Modo Freeze (Congelado)**
El modo freeze permite pausar la visualización sin detener la adquisición:

**Variables de estado**:
- `bool frozen`: Indica si está en modo congelado
- `frozen_left_limit, frozen_right_limit`: Límites de zoom independientes para modo congelado
- `frozen_down_limit, frozen_up_limit`: Límites verticales independientes
- `frozen_size`: Cantidad de muestras en el snapshot
- `frozen_dataX, frozen_dataY, frozen_dataY_filtered`: Vectores con copia de datos congelados

**Funcionamiento**:
1. Al presionar "Congelar", se copia un snapshot completo de los datos actuales
2. La visualización se detiene, permitiendo zoom y análisis detallado
3. El SerialWorker continúa capturando datos en segundo plano (protegido por data_mutex)
4. Al presionar "Reanudar", vuelve al modo en vivo sin pérdida de datos

#### **Hilos de Trabajo**
1. **Serial Worker** (`SerialWorker()`):
   - Lee datos del puerto serie
   - Transforma valores ADC (0-255) a voltaje (-6V a +6V)
   - Aplica filtro seleccionado
   - Escribe datos filtrados de vuelta al puerto
   - **Protección**: Usa data_mutex durante freeze/unfreeze
   - **Frecuencia**: Limitada por la velocidad del puerto serie

2. **Analysis Worker** (`AnalysisWorker()`):
   - Calcula FFT de las últimas N muestras
   - Identifica frecuencia dominante y offset DC
   - **Activación**: Solo en modo en vivo (pausado durante freeze)
   - **Frecuencia**: ~10 Hz (cada 100ms)

#### **Interfaz Gráfica** (`Draw()`)
Renderiza tres secciones principales:

1. **Panel Lateral (Sidebar)**:
   - Selector de puerto COM
   - Configuración de frecuencia de muestreo y baud rate
   - Mapeo ADC (máximo/mínimo)
   - Stride (optimización de renderizado)
   - Botones Conectar/Desconectar
   - **Botón Congelar/Reanudar**: Alterna entre modo en vivo y congelado

2. **Gráfica de Entrada**:
   - Muestra la señal original recibida
   - Eje X: Tiempo (con formato métrico: ms, s)
   - Eje Y: Voltaje (con formato métrico: mV, V)
   - Zoom sincronizado entre Entrada y Salida en modo en vivo
   - Zoom independiente en modo congelado

3. **Sección de Filtro** (plegable):
   - **Gráfica de Salida**: Señal filtrada
   - **Selectores**: Ninguno / Pasa-bajos / Pasa-altos
   - **Slider**: Frecuencia de corte
   - Zoom sincronizado con gráfica de Entrada

4. **Sección de Análisis** (plegable):
   - **Espectro de frecuencias**: Gráfica logarítmica
   - **Información**: Frecuencia dominante y offset DC
   - Solo se actualiza en modo en vivo

5. **Sección de Información** (inferior del sidebar):
   - Tiempo transcurrido
   - Indicador visual "[CONGELADO]" cuando está activo
   - Métricas de rendimiento (FPS, ms/frame si está habilitado)

**Transformación de Muestras**:
```cpp
// ADC (0-255) ? Voltaje (-6V a +6V)
double voltage = (sample - minimum) * map_factor - 6;

// Voltaje ? ADC
uint8_t sample = round((voltage + 6) * (maximum - minimum) / 12.0 + minimum);

```

### Seguridad de Hilos

#### Variables Compartidas Protegidas
```cpp
std::mutex data_mutex;               // Protege scrollX, scrollY, filter_scrollY durante freeze/unfreeze
std::mutex analysis_mutex;           // Protege acceso a FFT
std::condition_variable analysis_cv; // Señalización entre hilos
```

#### Patrones Usados
1. **Producer-Consumer**: Serial Worker ? Buffers ? Main Thread
2. **Conditional Wait**: Analysis Worker espera notificación para computar FFT
3. **Atomic Flags**: `do_serial_work`, `do_analysis_work` para detención limpia
4. **Lock Guard**: Protección de snapshot en modo freeze

#### Escenario de Freeze
```cpp
// Al congelar (UI Thread):
{
    std::lock_guard<std::mutex> lock(data_mutex);
    // Copiar snapshot de scrollX, scrollY, filter_scrollY
    // SerialWorker no puede escribir durante esta operación
}

// SerialWorker sigue corriendo:
{
    std::lock_guard<std::mutex> lock(data_mutex);
    scrollY->push(valor);  // Continúa acumulando datos
}
