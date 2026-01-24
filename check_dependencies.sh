#!/bin/bash
# Script para verificar dependencias de SerialPlotter
# Uso: ./check_dependencies.sh

echo "=== Verificación de Dependencias SerialPlotter ==="
echo ""

PROJECT_ROOT="$(dirname "$0")"
EXTERN_DIR="$PROJECT_ROOT/extern"

# Función para verificar directorio y archivos críticos
check_dependency() {
    local name=$1
    local dir=$2
    local critical_files=("${@:3}")
    
    echo -n "Verificando $name... "
    
    if [ ! -d "$EXTERN_DIR/$dir" ]; then
        echo "❌ FALTANTE - Carpeta $dir no encontrada"
        return 1
    fi
    
    local missing_files=()
    for file in "${critical_files[@]}"; do
        if [ ! -f "$EXTERN_DIR/$dir/$file" ] && [ ! -d "$EXTERN_DIR/$dir/$file" ]; then
            missing_files+=("$file")
        fi
    done
    
    if [ ${#missing_files[@]} -eq 0 ]; then
        echo "✅ OK"
        return 0
    else
        echo "⚠️  INCOMPLETO - Faltan: ${missing_files[*]}"
        return 1
    fi
}

# Verificar cada dependencia
missing_deps=0

check_dependency "FFTW3" "fftw3" "api/fftw3.h" "CMakeLists.txt"
((missing_deps += $?))

check_dependency "GLFW" "glfw" "include/GLFW/glfw3.h" "CMakeLists.txt"
((missing_deps += $?))

check_dependency "ImGui" "imgui" "imgui.h" "imgui.cpp" "backends"
((missing_deps += $?))

check_dependency "ImPlot" "implot" "implot.h" "implot.cpp"
((missing_deps += $?))

check_dependency "IIR1" "iir1" "Iir.h"
((missing_deps += $?))

echo ""
echo "=== Resumen ==="
if [ $missing_deps -eq 0 ]; then
    echo "✅ Todas las dependencias están instaladas correctamente"
    echo "Puedes compilar SerialPlotter con:"
    echo "  cd SerialPlotter && cmake -B build && cmake --build build"
else
    echo "❌ $missing_deps dependencia(s) faltante(s)"
    echo "Ver DEPENDENCIES.md para instrucciones de instalación"
fi

exit $missing_deps