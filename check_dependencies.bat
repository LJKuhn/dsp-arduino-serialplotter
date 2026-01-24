@echo off
REM Script para verificar dependencias de SerialPlotter en Windows
REM Uso: check_dependencies.bat

echo === Verificacion de Dependencias SerialPlotter ===
echo.

set "PROJECT_ROOT=%~dp0"
set "EXTERN_DIR=%PROJECT_ROOT%extern"
set "missing_deps=0"

REM Función para verificar dependencia
goto :check_all_deps

:check_dependency
set "name=%~1"
set "dir=%~2"
set "dep_path=%EXTERN_DIR%\%dir%"

echo|set /p="Verificando %name%... "

if not exist "%dep_path%" (
    echo ❌ FALTANTE - Carpeta %dir% no encontrada
    set /a missing_deps+=1
    goto :eof
)

REM Verificar archivos específicos según la dependencia
if "%name%"=="FFTW3" (
    if not exist "%dep_path%\api\fftw3.h" (
        echo ⚠️  INCOMPLETO - Falta api/fftw3.h
        set /a missing_deps+=1
        goto :eof
    )
)

if "%name%"=="GLFW" (
    if not exist "%dep_path%\include\GLFW\glfw3.h" (
        echo ⚠️  INCOMPLETO - Falta include/GLFW/glfw3.h
        set /a missing_deps+=1
        goto :eof
    )
)

if "%name%"=="ImGui" (
    if not exist "%dep_path%\imgui.h" (
        echo ⚠️  INCOMPLETO - Falta imgui.h
        set /a missing_deps+=1
        goto :eof
    )
)

if "%name%"=="ImPlot" (
    if not exist "%dep_path%\implot.h" (
        echo ⚠️  INCOMPLETO - Falta implot.h
        set /a missing_deps+=1
        goto :eof
    )
)

if "%name%"=="IIR1" (
    if not exist "%dep_path%\Iir.h" (
        echo ⚠️  INCOMPLETO - Falta Iir.h
        set /a missing_deps+=1
        goto :eof
    )
)

echo ✅ OK
goto :eof

:check_all_deps
call :check_dependency "FFTW3" "fftw3"
call :check_dependency "GLFW" "glfw"
call :check_dependency "ImGui" "imgui"
call :check_dependency "ImPlot" "implot"
call :check_dependency "IIR1" "iir1"

echo.
echo === Resumen ===
if %missing_deps%==0 (
    echo ✅ Todas las dependencias están instaladas correctamente
    echo Puedes compilar SerialPlotter con:
    echo   cd SerialPlotter ^&^& cmake -B build ^&^& cmake --build build
) else (
    echo ❌ %missing_deps% dependencia^(s^) faltante^(s^)
    echo Ver DEPENDENCIES.md para instrucciones de instalacion
)

exit /b %missing_deps%