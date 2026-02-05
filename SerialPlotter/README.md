# SerialPlotter

Este es un programa simple de DSP que intercambia información de señales con un microcontrolador mediante el puerto serie de la computadora.

## 🎯 **Características Principales**

- **🔄 Procesamiento bidireccional**: ADC → PC → DAC en tiempo real
- **📊 Análisis espectral**: FFT en tiempo real con FFTW3
- **🔧 Filtros digitales**: Butterworth IIR de 8º orden (pasa-bajos, pasa-altos)
- **📈 Visualización**: Gráficas de entrada y salida simultáneas
- **❄️ Modo Congelar**: Pausa la visualización sin detener la adquisición de datos
- **⚙️ Configuración**: Frecuencias y parámetros ajustables por interfaz
- **🎛️ Generador de pruebas**: 6 tipos de señales automáticas para testing
- **🖥️ Interfaz gráfica**: Construida con ImGui para una experiencia de usuario intuitiva

## 📁 **Estructura del Proyecto C++**

La organización del proyecto sigue estándares modernos de C++ y CMake. Cada carpeta tiene un propósito específico:

### **📂 Carpetas de Código Fuente**

#### **`src/` - Código Fuente Principal**
Contiene todos los archivos `.cpp` y `.h` del proyecto principal:
```
src/
├── main.cpp/h          # Punto de entrada, configuración OpenGL/ImGui
├── MainWindow.cpp/h    # Ventana principal, lógica de UI
├── Serial.cpp/h        # Comunicación serie con Arduino
├── FFT.cpp/h           # Análisis espectral con FFTW3
├── Settings.cpp/h      # Configuraciones del usuario
├── Console.cpp/h       # Manejo de consola Windows
├── Buffers.h          # Estructuras de datos para muestras
└── Widgets.h          # Componentes de UI reutilizables
```
**¿Por qué aquí?** Código específico del proyecto, lógica de negocio, implementaciones concretas.

#### **`include/` - Headers Públicos**
Headers de bibliotecas externas que se pueden incluir:
```
include/
├── fftw3.h            # API principal de FFTW3
├── glad/glad.h        # Loader de OpenGL
└── KHR/khrplatform.h  # Platform definitions para OpenGL
```
**¿Por qué aquí?** Headers que el proyecto puede incluir directamente, interfaz pública de dependencias.

### **📂 Carpetas de Dependencias**

#### **`extern/` - Bibliotecas Externas**
Código fuente completo de bibliotecas de terceros:
```
extern/
├── CMakeLists.txt     # Configuración de build para dependencias
├── fftw3/            # Biblioteca FFT (toda la fuente)
├── glfw/            # Framework de ventanas OpenGL
├── imgui/           # Immediate Mode GUI
├── implot/          # Extensión de plotting para ImGui
└── iir1/            # Filtros digitales IIR
```
**¿Por qué aquí?** Dependencias completas, código que NO escribimos nosotros, bibliotecas reutilizables.

### **📂 Carpetas de Compilación**

#### **`build/` - Build Release**
```
build/
├── CMakeCache.txt     # Cache de configuración CMake
├── CMakeFiles/        # Archivos internos de CMake
├── SerialPlotter.exe  # Ejecutable final optimizado
└── extern/           # Bibliotecas compiladas en Release
```

#### **`build-debug/` - Build Debug**  
```
build-debug/
├── SerialPlotter.exe  # Ejecutable con símbolos de debug
├── SerialPlotter.pdb  # Base de datos de símbolos para debugger
└── CMakeFiles/        # Archivos de build para modo debug
```
**¿Por qué separados?** Diferentes optimizaciones, symbols de debug vs performance.

#### **`out/` - Output de Visual Studio**
Carpeta generada automáticamente por Visual Studio para builds locales.
**¿Por qué aquí?** IDE-specific, no parte del sistema de build principal.

### **📂 Archivos de Configuración**

#### **Archivos Root**
```
├── CMakeLists.txt     # Script principal de build
├── .gitignore         # Archivos a ignorar en Git
├── .gitmodules        # Submódulos de Git (dependencias)
├── glad.c             # Implementación de glad (OpenGL loader)
└── imgui.ini          # Configuración de ImGui (layouts, etc.)
```

### **🏗️ Lógica de Organización**

#### **Separación por Responsabilidad:**
- **`src/`**: "Lo que escribimos nosotros"
- **`extern/`**: "Lo que otros escribieron" 
- **`include/`**: "Lo que queremos incluir fácilmente"
- **`build*/`**: "Lo que genera el compilador"

#### **Convenciones Estándar C++:**
1. **Headers (.h)** y **Source (.cpp)** juntos en `src/`
2. **Bibliotecas externas** en `extern/` o `third_party/`
3. **Builds separados** por configuración (Debug/Release)
4. **CMake out-of-source builds** (nunca compilar en la carpeta fuente)

#### **¿Cómo decidir dónde va cada archivo?**
- **¿Lo escribí yo?** → `src/`
- **¿Es una biblioteca externa?** → `extern/`  
- **¿Es un header que debo incluir?** → `include/`
- **¿Lo generó el compilador?** → `build*/` o `out/`
- **¿Configura el proyecto?** → Root directory

## Librerías utilizadas
- [GLFW](https://github.com/glfw/glfw): A multi-platform library for OpenGL, OpenGL ES, Vulkan, window and input.
- [glad](https://github.com/Dav1dde/glad): Multi-Language Vulkan/GL/GLES/EGL/GLX/WGL Loader-Generator based on the official specs.
- [Dear ImGui](https://github.com/ocornut/imgui): Dear ImGui: Bloat-free Graphical User interface for C++ with minimal dependencies.
- [ImPlot](https://github.com/epezent/implot): Immediate Mode Plotting.
- [FFTW](https://fftw.org/): C subroutine library for computing the discrete Fourier transform (DFT) in one or more dimensions, of arbitrary input size, and of both real and complex data.
- [iir1](https://github.com/berndporr/iir1): DSP IIR realtime filter library written in C++.

## Compilación

    cmake -DCMAKE_BUILD_TYPE=Release -S ruta/al/proyecto -B build
    cmake --build build

## Problemas conocidos
- Cuando se arrastra o se cambia el estado de la ventana se produce un pequeño desfase.
