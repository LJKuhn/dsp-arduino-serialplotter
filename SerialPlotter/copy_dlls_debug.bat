@echo off
REM Script para copiar DLLs necesarias al directorio de debug
REM Ejecutar después de cada compilación

echo ========================================
echo   Copiando DLLs para Debug
echo ========================================

REM Verificar que exista el directorio build-debug
if not exist "build-debug" (
    echo ERROR: No existe el directorio build-debug
    pause
    exit /b 1
)

REM Buscar y copiar glfw3.dll
echo.
echo Buscando glfw3.dll...
for /r "extern" %%f in (glfw3.dll) do (
    if exist "%%f" (
        echo   Encontrada: %%f
        copy /Y "%%f" "build-debug\" >nul
        echo   Copiada a build-debug\
        goto :found_glfw
    )
)
echo   NO ENCONTRADA - El programa podria no ejecutarse
:found_glfw

REM Buscar y copiar fftw3.dll (puede tener varios nombres)
echo.
echo Buscando fftw3.dll o libfftw3-3.dll...
for /r "extern" %%f in (fftw3.dll libfftw3-3.dll) do (
    if exist "%%f" (
        echo   Encontrada: %%f
        copy /Y "%%f" "build-debug\fftw3.dll" >nul
        echo   Copiada a build-debug\fftw3.dll
        goto :found_fftw
    )
)
echo   NO ENCONTRADA - El programa podria no ejecutarse
:found_fftw

REM Desbloquear el ejecutable
echo.
echo Desbloqueando SerialPlotter.exe...
if exist "build-debug\SerialPlotter.exe" (
    powershell -Command "Unblock-File -Path 'build-debug\SerialPlotter.exe' -ErrorAction SilentlyContinue"
    echo   Ejecutable desbloqueado
) else (
    echo   WARNING: SerialPlotter.exe no encontrado
)

echo.
echo ========================================
echo   Proceso completado
echo ========================================
echo.
echo Ahora puedes ejecutar el debug desde Visual Studio
pause
