# CMakeLists.txt - Configuración de compilación del proyecto SerialPlotter
#
# Sistema de procesamiento digital de señales en tiempo real que combina:
# - Visualización gráfica con ImGui/ImPlot
# - Análisis espectral con FFTW3
# - Filtros digitales IIR con iir1
# - Comunicación serial con hardware externo
#
# Requisitos:
# - CMake 3.20+
# - C++20
# - Ninja (recomendado) o Visual Studio

cmake_minimum_required(VERSION 3.20)
project(SerialPlotter)

# Configurar estándar de C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Forzar linkeo estático para todas las bibliotecas (simplifica distribución)
set(BUILD_SHARED_LIBS FALSE CACHE BOOL "" FORCE)

# Compilar todas las bibliotecas externas (GLFW, ImGui, ImPlot, iir1, FFTW3)
add_subdirectory(extern)

# Crear ejecutable principal
add_executable(SerialPlotter
        glad.c              # Cargador de funciones OpenGL
        src/main.cpp        # Punto de entrada y bucle principal
        src/FFT.cpp         # Análisis espectral (FFTW3)
        src/MainWindow.cpp  # Ventana principal e interfaz gráfica
        src/Serial.cpp      # Comunicación serial (Windows API)
        src/Settings.cpp    # Configuración y widgets de ajustes
        src/Console.cpp)    # Gestión de consola de Windows

# Directorios de headers públicos
target_include_directories(SerialPlotter PUBLIC
        include)  # Contiene fftw3.h y otros headers

# Vincular todas las bibliotecas necesarias
target_link_libraries(SerialPlotter PRIVATE
        glfw            # Gestión de ventanas y contexto OpenGL
        imgui           # Interfaz gráfica inmediata
        implot          # Gráficos para ImGui
        iir::iir_static # Filtros digitales IIR (Butterworth)
        fftw3)          # Transformada rápida de Fourier
