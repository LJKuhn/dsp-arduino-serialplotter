#pragma once
#include <string>

// Configuración global de la aplicación
struct Settings {
    int sampling_rate = 3840;                       // Frecuencia de muestreo en Hz
    int baud_rate = sampling_rate * 10;             // Velocidad del puerto serial (bits/s)
    int samples = sampling_rate;                    // Número de muestras para FFT
    std::string port;                               // Puerto COM seleccionado

    int maximum = 49, minimum = 175;                // Rango de valores ADC para mapeo
    int stride = 4;                                 // Dibuja 1 de cada N muestras (optimización)
    int byte_stride = sizeof(double) * stride;      // Stride en bytes

    double map_factor = 12.0 / (maximum - minimum); // Factor de conversión ADC a voltaje

    bool show_frame_time = false;                   // Mostrar FPS en UI
    bool open = false;                              // Estado ventana de configuración (deprecated)
};

// Ventana de configuración (DEPRECATED: ahora integrada en el sidebar principal)
class SettingsWindow {
    Settings& settings;
    bool& open;

public:
    explicit SettingsWindow(Settings& settings);

    void Toggle();
    void Draw();
};