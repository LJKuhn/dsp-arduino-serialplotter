# Dependencias Externas

Este archivo documenta las bibliotecas externas necesarias para compilar el proyecto SerialPlotter.

## Bibliotecas Requeridas

Las siguientes bibliotecas deben descargarse en la carpeta `extern/`:

### 1. FFTW3 (Fastest Fourier Transform in the West)
- **Versión**: 3.3.10 o superior
- **Propósito**: Análisis FFT para señales
- **Carpeta destino**: `extern/fftw3/`
- **URL**: http://fftw.org/download.html
- **Archivos críticos**: 
  - `api/fftw3.h`
  - Bibliotecas compiladas (.lib/.dll)

### 2. GLFW (Graphics Library Framework)
- **Versión**: 3.3 o superior
- **Propósito**: Manejo de ventanas para OpenGL
- **Carpeta destino**: `extern/glfw/`
- **URL**: https://github.com/glfw/glfw
- **Archivos críticos**: 
  - `include/GLFW/glfw3.h`
  - Bibliotecas compiladas

### 3. ImGui (Immediate Mode GUI)
- **Versión**: 1.89 o superior
- **Propósito**: Interfaz gráfica de usuario
- **Carpeta destino**: `extern/imgui/`
- **URL**: https://github.com/ocornut/imgui
- **Archivos críticos**: 
  - `imgui.h`, `imgui.cpp`
  - `backends/` para OpenGL/GLFW

### 4. ImPlot (Plotting extension for ImGui)
- **Versión**: 0.14 o superior
- **Propósito**: Gráficos y plots en tiempo real
- **Carpeta destino**: `extern/implot/`
- **URL**: https://github.com/epezent/implot
- **Archivos críticos**: 
  - `implot.h`, `implot.cpp`

### 5. IIR1 (Digital Filter Library)
- **Versión**: 1.9.0 o superior
- **Propósito**: Filtros digitales Butterworth
- **Carpeta destino**: `extern/iir1/`
- **URL**: https://github.com/berndporr/iir1
- **Archivos críticos**: 
  - `Iir.h`
  - Bibliotecas compiladas

## Instalación Automática (Recomendado)

### Opción 1: Script de descarga
Ejecutar desde la raíz del proyecto:
```bash
# TODO: Crear script de descarga automática
./download_dependencies.sh  # Linux/Mac
download_dependencies.bat   # Windows
```

### Opción 2: Git Submodules (Avanzado)
```bash
git submodule add https://github.com/glfw/glfw extern/glfw
git submodule add https://github.com/ocornut/imgui extern/imgui
git submodule add https://github.com/epezent/implot extern/implot
git submodule add https://github.com/berndporr/iir1 extern/iir1
# FFTW3 requiere descarga manual desde sitio oficial
```

## Instalación Manual

1. Crear carpeta `extern/` en la raíz del proyecto
2. Descargar cada biblioteca en su respectiva subcarpeta
3. Seguir instrucciones de compilación de cada biblioteca
4. Verificar que CMake encuentre los headers en `include/`

## Verificación de Instalación

**Verificación automática:**
```bash
# Linux/Mac:
./check_dependencies.sh

# Windows:
check_dependencies.bat
```

**Verificación manual con CMake:**
```bash
cd SerialPlotter/
mkdir build
cd build
cmake ..
```

Si hay errores de bibliotecas faltantes, revisar que estén en las rutas correctas dentro de `extern/`.

## Estructura Final Esperada

```
extern/
├── fftw3/
│   ├── api/fftw3.h
│   └── [bibliotecas compiladas]
├── glfw/
│   ├── include/GLFW/
│   └── [bibliotecas compiladas]
├── imgui/
│   ├── imgui.h
│   ├── imgui.cpp
│   └── backends/
├── implot/
│   ├── implot.h
│   └── implot.cpp
└── iir1/
    ├── Iir.h
    └── [bibliotecas compiladas]
```

## Notas Importantes

- **FFTW3**: Requiere compilación específica para Windows (usar CMake)
- **Todas las bibliotecas**: Compilar en modo Release para mejor rendimiento
- **Versiones**: Las versiones listadas son las mínimas probadas
- **CMake**: El archivo CMakeLists.txt está configurado para buscar en `extern/`

## Problemas Comunes

1. **Error "fftw3.h not found"**: Verificar ruta `extern/fftw3/api/`
2. **Error de linking**: Compilar bibliotecas en el mismo modo (Debug/Release)
3. **OpenGL errors**: Instalar drivers gráficos actualizados
4. **CMake no encuentra bibliotecas**: Revisar rutas en CMakeLists.txt