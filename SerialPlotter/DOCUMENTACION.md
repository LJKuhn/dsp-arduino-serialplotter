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
- **Retransmite** las señales filtradas de vuelta al microcontrolador

### Tecnologías Principales
- **Lenguaje**: C++20
- **Sistema de Build**: CMake 3.20+
- **Generador**: Ninja
- **Plataforma**: Windows (usa API de Windows para comunicación serial)

---

## Arquitectura del Sistema

```
???????????????????????????????????????????????????????????
?                    Aplicación Principal                  ?
?                      (main.cpp)                          ?
???????????????????????????????????????????????????????????
                 ?
         ?????????????????
         ?               ?
    ????????????   ?????????????
    ? MainWindow?   ? Settings  ?
    ?  (GUI)    ?   ?  Window   ?
    ?????????????   ?????????????
         ?
    ??????????????????????????????????
    ?                                ?
??????????  ????????  ????????  ????????
? Serial ?  ? FFT  ?  ?Filter?  ?Buffer?
?        ?  ?      ?  ?(IIR) ?  ?      ?
??????????  ????????  ????????  ????????
```

### Hilos de Ejecución

1. **Hilo Principal**: Renderizado de la interfaz gráfica (ImGui + OpenGL)
2. **Hilo Serial**: Lectura/escritura continua del puerto serie
3. **Hilo de Análisis**: Cálculo de FFT en segundo plano

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

### `main.cpp` / `main.h`
**Responsabilidad**: Punto de entrada y bucle principal de la aplicación.

**Funcionalidades**:
- Inicialización de GLFW y OpenGL
- Creación del contexto de ImGui e ImPlot
- Bucle de renderizado principal
- Gestión de eventos de ventana (minimización, foco, redimensionamiento)
- Control de framerate adaptativo (reduce CPU cuando la ventana está minimizada)

**Flujo**:
```cpp
1. Inicializar GLFW
2. Crear ventana OpenGL
3. Inicializar ImGui/ImPlot
4. Bucle principal:
   - Procesar eventos
   - Actualizar lógica
   - Renderizar interfaz
   - Intercambiar buffers
5. Limpieza y cierre
```

---

### `MainWindow.cpp` / `MainWindow.h`
**Responsabilidad**: Ventana principal y lógica central de la aplicación.

**Componentes**:

#### **Gestión de Datos**
- `ScrollBuffer<double>* scrollX, *scrollY, *filter_scrollY`: Buffers circulares para las muestras
- `FFT* fft`: Motor de análisis espectral
- `Serial serial`: Interfaz de comunicación serie

#### **Hilos de Trabajo**
1. **Serial Worker** (`SerialWorker()`):
   - Lee datos del puerto serie
   - Transforma valores ADC (0-255) a voltaje (-6V a +6V)
   - Aplica filtro seleccionado
   - Escribe datos filtrados de vuelta al puerto
   - **Frecuencia**: Limitada por la velocidad del puerto serie

2. **Analysis Worker** (`AnalysisWorker()`):
   - Calcula FFT de las últimas N muestras
   - Identifica frecuencia dominante y offset DC
   - **Activación**: Solo cuando la pestaña "Análisis" está abierta
   - **Frecuencia**: ~10 Hz (cada 100ms)

#### **Interfaz Gráfica** (`Draw()`)
Renderiza tres secciones principales:

1. **Menú Superior**:
   - Selector de puerto COM
   - Botón de configuración
   - Botón Conectar/Desconectar

2. **Gráfica de Entrada**:
   - Muestra la señal original recibida
   - Eje X: Tiempo (con formato métrico: ms, s)
   - Eje Y: Voltaje (con formato métrico: mV, V)
   - Zoom y paneo sincronizado con otras gráficas

3. **Sección de Filtro** (plegable):
   - **Gráfica de Salida**: Señal filtrada
   - **Selectores**: Ninguno / Pasa-bajos / Pasa-altos
   - **Slider**: Frecuencia de corte

4. **Sección de Análisis** (plegable):
   - **Espectro de frecuencias**: Gráfica logarítmica
   - **Información**: Frecuencia dominante y offset DC

5. **Barra de Estado**:
   - Tiempo transcurrido
   - Métricas de rendimiento (FPS, ms/frame)

**Transformación de Muestras**:
```cpp
// ADC (0-255) ? Voltaje (-6V a +6V)
double voltage = (sample - minimum) * map_factor - 6;

// Voltaje ? ADC
uint8_t sample = round((voltage + 6) * (maximum - minimum) / 12.0 + minimum);
```

---

### `Serial.cpp` / `Serial.h`
**Responsabilidad**: Comunicación con el puerto serie (Windows API).

**Funciones Principales**:

#### `EnumerateComPorts()`
- **Propósito**: Lista todos los puertos COM disponibles
- **Método**: Consulta el registro de Windows (`HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM`)
- **Retorna**: `std::vector<std::string>` con nombres de puertos (ej: "COM3", "COM4")

#### `Serial::open(port, baud)`
- **Propósito**: Abre un puerto COM con configuración específica
- **Parámetros**:
  - `port`: Nombre del puerto (ej: "COM3")
  - `baud`: Velocidad en baudios (por defecto: velocidad de muestreo × 10)
- **Configuración**:
  - 8 bits de datos
  - Sin paridad
  - 1 bit de parada
  - Sin control de flujo
  - Timeouts ajustados para lectura no bloqueante

#### `Serial::read(buffer, size)`
- **Propósito**: Lee bytes del puerto
- **Retorna**: Número de bytes leídos (0 si no hay datos)
- **Nota**: No bloqueante gracias a `COMMTIMEOUTS` configurados

#### `Serial::write(buffer, size)`
- **Propósito**: Escribe bytes al puerto
- **Retorna**: Número de bytes escritos

#### `Serial::available()`
- **Propósito**: Consulta cuántos bytes están disponibles en el buffer de entrada
- **Uso**: `ClearCommError()` de la API de Windows

---

### `FFT.cpp` / `FFT.h`
**Responsabilidad**: Análisis espectral de señales usando FFTW3.

**Estructura Interna**:
```cpp
class FFT {
    fftw_complex* complex;      // Salida compleja de la FFT
    fftw_plan p;                // Plan precomputado de FFTW
    vector<double> samples;     // Entrada (señal temporal)
    vector<double> amplitudes;  // Salida (magnitudes)
    double offset;              // Componente DC
    int n_frequency;            // Índice de frecuencia dominante
};
```

**Proceso de Análisis**:

1. **Inicialización** (`constructor`):
   - Crea un plan FFT real-a-complejo (`fftw_plan_dft_r2c_1d`)
   - Reserva memoria para `samples_size` muestras
   - Reserva memoria para `samples_size/2 + 1` amplitudes (espectro unilateral)

2. **Carga de Datos** (`SetData`):
   - Copia datos de entrada al buffer interno
   - Rellena con ceros si hay menos muestras que el tamaño del plan

3. **Cálculo** (`Compute`):
   - Ejecuta el plan FFT (`fftw_execute(p)`)
   - Calcula magnitudes: `sqrt(real² + imag²) / N`
   - Normaliza dividiendo por el número de muestras
   - Identifica la frecuencia con mayor amplitud (excluyendo DC)
   - Guarda el componente DC (offset)

4. **Resultados**:
   - `Offset()`: Valor medio de la señal (componente DC)
   - `Frequency(fs)`: Frecuencia dominante en Hz
   - `Plot(fs)`: Renderiza el espectro con ImPlot

**Fórmula de Frecuencia**:
```
f = (índice × frecuencia_muestreo) / tamaño_muestra
```

---

### `Settings.cpp` / `Settings.h`
**Responsabilidad**: Configuración de la aplicación y ventana de ajustes.

**Parámetros Configurables**:

#### `Settings` (estructura)
```cpp
int sampling_rate = 3840;        // Hz - Frecuencia de muestreo
int baud_rate = 38400;           // baudios (sampling_rate × 10)
int samples = 3840;              // Número de muestras para FFT
string port;                     // Puerto COM seleccionado

int maximum = 49;                // Valor ADC para +6V
int minimum = 175;               // Valor ADC para -6V
double map_factor;               // Factor de conversión ADC?Voltaje

int stride = 4;                  // Dibuja 1 de cada N puntos
int byte_stride;                 // stride × sizeof(double)

bool show_frame_time = false;    // Muestra FPS en barra de estado
bool open = false;               // Estado de la ventana de config
```

#### `SettingsWindow::Draw()`
Renderiza la ventana de configuración con:

1. **Combo de Frecuencia de Muestreo**:
   - Opciones predefinidas: 120, 240, 480, 960, ... 100000 Hz
   - Ajusta automáticamente `baud_rate = sampling_rate × 10`

2. **Combo de Velocidad (Baud Rate)**:
   - Velocidades estándar: 9600, 115200, etc.

3. **Combo de Puerto COM**:
   - Lista dinámica de puertos disponibles

4. **Sección "Mapeo"** (TreeNode):
   - Sliders para calibrar máximo y mínimo del ADC
   - Permite ajustar el rango de voltaje según el hardware

5. **Sección "Rendimiento"**:
   - **Slider de Stride**: Reduce puntos dibujados (mejora FPS)
   - **Checkbox**: Mostrar tiempo de renderizado

**Valores Predefinidos**:
```cpp
const int frecuencias[] = {
    120, 240, 480, 960, 1440, 1920, 3840, 5760,
    11520, 23040, 25000, 46080, 50000, 92160, 100000, 2000000
};

const int bauds[] = {
    1200, 2400, 4800, 9600, 14400, 19200, 38400, 57600,
    115200, 230400, 250000, 460800, 500000, 921600, 1000000, 2000000
};
```

---

### `Buffers.h`
**Responsabilidad**: Estructuras de datos eficientes para señales en tiempo real.

#### `ScrollBuffer<T>`
**Propósito**: Buffer circular con ventana deslizante.

**Características**:
- **Capacidad**: Tamaño total del buffer interno
- **View**: Tamaño de la ventana visible (últimas N muestras)
- **Comportamiento**:
  - Cuando se llena, elimina automáticamente las muestras más antiguas
  - Mantiene siempre las últimas `view` muestras accesibles
  - Acceso eficiente sin copias innecesarias

**Métodos Principales**:
```cpp
void push(T value);              // Agrega una muestra
void write(T* buffer, count);    // Agrega múltiples muestras
uint32_t count() const;          // Número de muestras visibles
T& front();                      // Primera muestra visible
T& back();                       // Última muestra visible
const T* data() const;           // Puntero a los datos (para ImPlot)
```

**Uso en el proyecto**:
- `scrollX`: Marca de tiempo de cada muestra
- `scrollY`: Valores de la señal original
- `filter_scrollY`: Valores de la señal filtrada

#### `Buffer<T>`
**Propósito**: Buffer circular thread-safe para comunicación productor-consumidor.

**Características**:
- Protegido con mutexes
- Lectura y escritura no bloqueante
- Usado internamente por `Serial` (aunque no se ve explícitamente en el código analizado)

---

### `Console.cpp` / `Console.h`
**Responsabilidad**: Gestión de la consola de Windows.

**Funcionalidad**:
- Detecta si la aplicación fue lanzada desde la consola o como aplicación GUI
- Oculta la consola automáticamente al iniciar (mejora la experiencia de usuario)
- Permite restaurar la consola para debugging

**Métodos**:
```cpp
Console::IsOwn()        // ¿La consola pertenece a este proceso?
Console::Hide(persist)  // Oculta la consola
Console::Show(persist)  // Muestra la consola
```

**Caso de Uso**:
```cpp
Console console;
if (console.IsOwn())
    console.Hide(true);  // Solo oculta si la consola es propia del proceso
```

---

### `Widgets.h`
**Responsabilidad**: Componentes de UI reutilizables.

#### `select_menu<Container>()`
**Propósito**: Crea un menú desplegable con opciones dinámicas.

**Parámetros**:
- `title`: Texto del menú
- `selection`: Referencia al valor seleccionado
- `get_values`: Función que retorna el contenedor de opciones
- `to_string`: Función que convierte cada opción a string
- `empty_msg`: Mensaje cuando no hay opciones

**Ejemplo de Uso**:
```cpp
select_menu("Puerto", settings->port, 
            EnumerateComPorts, 
            [](string s) { return s; },
            "No hay dispositivos");
```

#### `combo<Container>()`
**Propósito**: Similar a `select_menu` pero con estilo de combo box.

**Uso en el proyecto**:
- Selector de frecuencia de muestreo
- Selector de velocidad de baudios
- Selector de puerto COM

---

## Flujo de Datos

### Recepción de Señal
```
Microcontrolador ? Puerto COM ? Serial::read()
                                     ?
                              TransformSample()
                              (ADC ? Voltaje)
                                     ?
                    ??????????????????????????????????
                    ?                                ?
             scrollY (original)              Filtro IIR
                    ?                                ?
              Gráfica "Entrada"         filter_scrollY (filtrada)
                                                     ?
                                         Gráfica "Salida"
                                                     ?
                                       InverseTransformSample()
                                         (Voltaje ? ADC)
                                                     ?
                                            Serial::write()
                                                     ?
                                            Microcontrolador
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
```

---

## Compilación y Configuración

### Dependencias del Sistema
- **Windows 10/11** (para API de serial)
- **Visual Studio 2019+** o **MinGW-w64** con soporte C++20
- **CMake 3.20+**
- **Ninja** (generador de build)

### Comandos de Compilación

#### Debug
```bash
cmake -B build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
.\build-debug\SerialPlotter.exe
```

#### Release
```bash
cmake -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
.\build-release\SerialPlotter.exe
```

### Configuración de CMake

#### `CMakeLists.txt` Principal
```cmake
cmake_minimum_required(VERSION 3.20)
project(SerialPlotter)

set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS FALSE)  # Linkeo estático

add_subdirectory(extern)      # Compila bibliotecas externas

add_executable(SerialPlotter
    glad.c
    src/main.cpp
    src/FFT.cpp
    # ... otros archivos
)

target_link_libraries(SerialPlotter PRIVATE
    glfw
    imgui
    implot
    iir::iir_static
    fftw3
)
```

#### Configuración de FFTW3
Opciones importantes en `extern/fftw3/CMakeLists.txt`:
```cmake
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ENABLE_SSE2 "Compile with SSE2 support" OFF)
option(ENABLE_AVX "Compile with AVX support" OFF)
option(ENABLE_AVX2 "Compile with AVX2 support" OFF)
```

**Recomendación**: Activar `ENABLE_AVX2` si el procesador lo soporta para máximo rendimiento.

---

## Detalles Técnicos

### Optimizaciones de Rendimiento

#### 1. **Stride en Gráficas**
- **Problema**: Renderizar miles de puntos en tiempo real es costoso
- **Solución**: Parámetro `stride` que dibuja solo 1 de cada N puntos
- **Configuración**: `Settings::stride` (potencia de 2: 1, 2, 4, 8...)
- **Trade-off**: Mayor stride = mejor FPS pero menor detalle

#### 2. **FFT en Hilo Separado**
- **Motivo**: `fftw_execute()` puede tomar varios milisegundos
- **Implementación**: 
  - `AnalysisWorker()` corre en hilo propio
  - Activado solo cuando la pestaña "Análisis" está visible
  - Actualización cada 100ms (no en cada frame)

#### 3. **Framerate Adaptativo**
- **Comportamiento**:
  - Ventana enfocada: 60 FPS (con VSync)
  - Ventana desenfocada: 20 FPS
  - Ventana minimizada: 0 FPS (espera de eventos)
- **Beneficio**: Reduce consumo de CPU/GPU cuando no es necesario

#### 4. **Buffers Circulares**
- **Ventaja**: No hay realocaciones de memoria
- **`ScrollBuffer`**: Solo mueve un offset, los datos permanecen en su lugar
- **Acceso O(1)**: ImPlot puede acceder directamente al buffer con stride

### Limitaciones Conocidas

#### 1. **Desfase al Arrastrar Ventana**
- **Causa**: Windows bloquea el bucle de mensajes durante operaciones modales
- **Efecto**: Los hilos de trabajo siguen corriendo pero la interfaz no se actualiza
- **Posible solución**: Pausar captura durante operaciones de ventana

#### 2. **Solo Windows**
- **Razón**: Usa API nativa de Windows para serial (`CreateFile`, `ReadFile`, etc.)
- **Portabilidad**: Requeriría implementación con librerías cross-platform (ej: Boost.Asio)

#### 3. **Frecuencias de Muestreo Altas**
- **Limitación**: El puerto serie puede ser un cuello de botella
- **Cálculo**: A 115200 baudios ? ~11520 bytes/s ? máx ~11.5 kHz (asumiendo 1 byte/muestra)
- **Solución para frecuencias mayores**: Usar USB nativo o compresión

### Seguridad de Hilos

#### Variables Compartidas Protegidas
```cpp
std::mutex analysis_mutex;           // Protege acceso a FFT
std::condition_variable analysis_cv; // Señalización entre hilos
```

#### Patrones Usados
1. **Producer-Consumer**: Serial Worker ? Buffers ? Main Thread
2. **Conditional Wait**: Analysis Worker espera notificación para computar FFT
3. **Atomic Flags**: `do_serial_work`, `do_analysis_work` para detención limpia

### Configuración de Filtros

#### Filtro Pasa-Bajos (Butterworth de 8º Orden)
```cpp
Iir::Butterworth::LowPass<8> lowpass_filter;
lowpass_filter.setup(sampling_rate, cutoff_frequency);
```
- **Orden 8**: Pendiente de ~48 dB/octava
- **Tipo Butterworth**: Respuesta plana en banda de paso
- **Rango típico**: 1 Hz - `sampling_rate/4`

#### Filtro Pasa-Altos (Butterworth de 8º Orden)
```cpp
Iir::Butterworth::HighPass<8> highpass_filter;
highpass_filter.setup(sampling_rate, cutoff_frequency);
```
- **Rango típico**: `sampling_rate/4` - `sampling_rate/2 - 1`

**Nota**: Los filtros se resetean (`reset()`) al cambiar parámetros para evitar transitorios.

---

## Glosario de Términos

| Término | Significado |
|---------|-------------|
| **FFT** | Fast Fourier Transform - Algoritmo eficiente para calcular la DFT |
| **DFT** | Discrete Fourier Transform - Transforma señal temporal a frecuencial |
| **IIR** | Infinite Impulse Response - Tipo de filtro digital recursivo |
| **ADC** | Analog-to-Digital Converter - Convierte señal analógica a digital |
| **DSP** | Digital Signal Processing - Procesamiento de señales digitales |
| **ImGui** | Immediate Mode GUI - Paradigma de interfaz sin estado retenido |
| **VSync** | Vertical Synchronization - Sincroniza FPS con tasa de refresco |
| **Stride** | Paso o salto entre elementos procesados |
| **Baudios** | Bits por segundo en comunicación serial |

---

## Referencias

### Documentación de Bibliotecas
- [FFTW Manual](http://www.fftw.org/fftw3_doc/)
- [ImGui GitHub](https://github.com/ocornut/imgui)
- [ImPlot Documentation](https://github.com/epezent/implot/wiki)
- [iir1 Examples](https://github.com/berndporr/iir1/tree/master/demo)
- [GLFW Documentation](https://www.glfw.org/documentation.html)

### Algoritmos y Conceptos
- [Butterworth Filters (Wikipedia)](https://en.wikipedia.org/wiki/Butterworth_filter)
- [Discrete Fourier Transform](https://en.wikipedia.org/wiki/Discrete_Fourier_transform)
- [Serial Communication](https://en.wikipedia.org/wiki/Serial_communication)

---

## Autor y Licencia

**Proyecto**: SerialPlotter  
**Repositorio**: https://github.com/c-mateo/SerialPlotter  
**Autor**: c-mateo  
**Propósito**: Proyecto académico de Procesamiento Digital de Señales

---

*Última actualización: [Fecha de generación de este documento]*
