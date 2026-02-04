// Settings.h - Configuración global de la aplicación
//
// Define todas las opciones configurables del sistema de adquisición y visualización:
// - Parámetros de comunicación serial (puerto, baud rate)
// - Frecuencia de muestreo y cantidad de muestras para FFT
// - Mapeo de valores ADC a voltaje
// - Opciones de rendimiento (stride para optimización de gráficos)
// - Opciones de visualización (FPS, etc.)

#pragma once
#include <string>

// Estructura de configuración global
struct Settings {
    // Parámetros de muestreo
    int sampling_rate = 3840;                       // Frecuencia de muestreo en Hz (muestras por segundo)
    int baud_rate = sampling_rate * 10;             // Velocidad del puerto serial en bits/segundo (relación 10:1)
    int samples = sampling_rate;                    // Número de muestras para análisis FFT
    std::string port;                               // Puerto COM seleccionado (ej: "COM3")

    // Mapeo de valores ADC (8 bits: 0-255) a voltaje
    int maximum = 49, minimum = 175;                // Valores ADC que corresponden a +6V y -6V respectivamente
    double map_factor = 12.0 / (maximum - minimum); // Factor de conversión: 12V / rango_ADC

    // Optimización de rendimiento gráfico
    int stride = 4;                                 // Dibuja 1 de cada N muestras (reduce puntos en gráfico)
    int byte_stride = sizeof(double) * stride;      // Stride en bytes para ImPlot

    // Opciones de interfaz
    bool show_frame_time = false;                   // Mostrar FPS en UI
    bool open = false;                              // Estado ventana de configuración (DEPRECATED)
};

// Ventana de configuración independiente
// NOTA: Esta clase está DEPRECATED - todas las opciones ahora están integradas
// en el sidebar de MainWindow para acceso más rápido y mejor UX.
// Se mantiene por compatibilidad pero ya no se usa activamente.
class SettingsWindow {
    Settings& settings;
    bool& open;

public:
    explicit SettingsWindow(Settings& settings);

    void Toggle();  // Alterna visibilidad de la ventana
    void Draw();    // Renderiza la ventana (ya no se usa)
};