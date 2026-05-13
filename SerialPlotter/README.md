# SerialPlotter — Interfaz PC para DSP en Tiempo Real

Aplicación profesional de visualización y análisis espectral desarrollada en C++17. Comunica bidireccionalamente con Arduino Mega 2560 mediante UART para procesar señales en tiempo real con FFT y filtros digitales IIR.

## Características

- **Visualización dual**: gráficos temporales + análisis espectral (FFT) en paralelo
- **FFT en tiempo real**: FFTW3 optimizado, detección automática de armónicas y THD
- **Filtros Butterworth IIR orden 8**: pasa-bajos y pasa-altos configurables
- **Interfaz gráfica moderna**: ImGui + ImPlot, responsiva y configurable
- **Comunicación bidireccional**: Arduino → PC (señales), PC → Arduino (resultados procesados)
- **Buffer circular 256 bytes**: manejo eficiente de datos sin pérdida

## Especificaciones

| Característica | Detalle |
|---|---|
| **Lenguaje** | C++17 |
| **Build** | CMake 3.20+, Ninja |
| **GUI** | ImGui + ImPlot + GLFW + OpenGL (GLAD) |
| **Procesamiento** | FFTW3 (FFT), IIR1 (Filtros) |
| **Comunicación** | UART 38400 baud, bidireccional |
| **Plataforma** | Windows (MSVC 2019+) |
| **Latencia** | ~0.6–0.8 ms extremo a extremo |

## Estructura del Código Fuente

```
SerialPlotter/
├── src/
│   ├── main.cpp/h              # Punto de entrada, loop principal
│   ├── MainWindow.cpp/h        # Ventana principal, composición UI
│   ├── Serial.cpp/h            # Control de puerto COM
│   ├── FFT.cpp/h               # Análisis espectral con FFTW3
│   ├── Settings.cpp/h          # Persistencia de configuración
│   ├── Console.cpp/h           # Salida de debugging (Windows)
│   ├── Buffers.h               # Estructuras circulares para muestras
│   └── Widgets.h               # Componentes UI reutilizables
│
├── extern/
│   ├── fftw3/                  # Transformada rápida de Fourier
│   ├── imgui/                  # Interfaz gráfica inmediata
│   ├── implot/                 # Gráficos para ImGui
│   ├── glfw/                   # Framework OpenGL
│   └── iir1/                   # Filtros digitales IIR
│
├── include/
│   ├── fftw3.h
│   ├── glad/glad.h
│   └── KHR/khrplatform.h
│
└── CMakeLists.txt
```

## Compilar

```bash
cd SerialPlotter
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/SerialPlotter.exe
```

**Requisitos previos**: instalar dependencias según `DEPENDENCIES.md` en la raíz del proyecto.

## Uso

1. Conectar Arduino Mega 2560 por USB
2. Ejecutar `SerialPlotter.exe`
3. Seleccionar puerto COM
4. Configurar filtros y parámetros FFT desde la interfaz
5. Comenzar adquisición de datos

---

*Componente PC del sistema DSP integrado Arduino + PC. Ver README.md en la raíz para contexto completo.*
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
