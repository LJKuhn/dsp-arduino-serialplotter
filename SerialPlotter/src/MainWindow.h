// MainWindow.h - Declaración de la ventana principal de la aplicación
//
// MainWindow gestiona toda la interfaz gráfica y lógica de adquisición/visualización:
// - Comunicación serial con dispositivo externo (Arduino, etc.)
// - Visualización en tiempo real de señales (entrada, filtrada y espectro FFT)
// - Aplicación de filtros digitales (pasa bajos, pasa altos)
// - Modo congelado (freeze) para análisis sin detener adquisición
// - Multi-threading para adquisición y análisis paralelos
//
// Arquitectura de hilos:
// - UI Thread: renderizado de gráficos con ImGui/ImPlot
// - SerialWorker: lectura continua del puerto serial y filtrado
// - AnalysisWorker: cálculo periódico de FFT en segundo plano

#pragma once
#include <chrono>
#include <thread>

#include "Buffers.h"
#include "Serial.h"
#include "FFT.h"
#include "Settings.h"


class MainWindow {
    using clock = std::chrono::high_resolution_clock;
    using time_point = clock::time_point;
    using duration = std::chrono::duration<double>;

    // Tipos de filtros disponibles
    enum class Filter {
        None,      // Sin filtrado
        LowPass,   // Filtro pasa bajos (Butterworth orden 8)
        HighPass   // Filtro pasa altos (Butterworth orden 8)
    };

    // Comunicación serial
    Serial serial;
    
    // Hilos de trabajo en paralelo
    std::thread serial_thread, analysis_thread;
    std::mutex analysis_mutex;
    std::condition_variable analysis_cv;
    std::mutex data_mutex;  // Protege acceso concurrente a scrollX, scrollY y filter_scrollY durante freeze/unfreeze

    // Estructuras de datos principales
    FFT* fft = nullptr;
    ScrollBuffer<double>* scrollX = nullptr;      // Eje temporal (segundos)
    ScrollBuffer<double>* scrollY = nullptr;      // Señal de entrada (voltaje)
    ScrollBuffer<double>* filter_scrollY = nullptr; // Señal filtrada (voltaje)

    int max_time = 120;  // Tiempo máximo de buffer (segundos)

    float max_time_visible = 5;  // Ventana de tiempo visible por defecto (segundos)

    time_point start_time = clock::now();

    int max = 0;

    // Límites de zoom del gráfico en modo en vivo (sincronizados entre Entrada y Salida)
    double left_limit = 0, right_limit = max_time_visible;
    double down_limit = -7, up_limit = 7;

    double next_time = 0;  // Tiempo acumulado desde inicio de adquisición

    int size = 0;  // Cantidad de puntos a dibujar (con stride aplicado)

    // Buffers temporales para lectura/escritura serial
    std::vector<uint8_t> read_buffer, write_buffer;

    Settings* settings;
    SettingsWindow* settingsWindow;

    // Configuración de filtros digitales
    int min_cutoff_frequency = 1, max_cutoff_frequency = 100;
    int cutoff_frequency[3] = { 0, 20, 100 };  // [None, LowPass, HighPass]
    Filter selected_filter = Filter::None;

    int width, height;  // Dimensiones de la ventana

    // Variables para el modo freeze (congelar visualización sin detener adquisición)
    bool frozen = false;
    double frozen_left_limit = 0, frozen_right_limit = 5;
    double frozen_down_limit = -7, frozen_up_limit = 7;
    int frozen_size = 0;
    
    // Buffers para almacenar snapshot de datos cuando se congela la visualización
    std::vector<double> frozen_dataX;
    std::vector<double> frozen_dataY;
    std::vector<double> frozen_dataY_filtered;

public:
    MainWindow(int width, int height, Settings& config, SettingsWindow& ventanaConfig);
    ~MainWindow();

private:
    // Gestión de buffers
    void CreateBuffers();
    void DestroyBuffers();

    // Transformación de valores ADC (0-255) a voltaje (-6V a +6V)
    double TransformSample(uint8_t v);
    uint8_t InverseTransformSample(double v);

    // Control de conexión serial
    bool started = false;
    void ToggleConnection();

    void Start();  // Inicia adquisición y hilos de trabajo
    void Stop();   // Detiene adquisición y espera a que terminen los hilos

    // Gestión de filtros digitales
    void SelectFilter(Filter filter);
    void SetupFilter();       // Configura parámetros del filtro según sampling_rate
    static void ResetFilters(); // Limpia el estado interno de los filtros

    // Control de hilos de trabajo
    bool do_serial_work = true;
    bool filter_open = true;  // Sección Filtro abierta por defecto en UI
    void SerialWorker();      // Hilo que lee datos del puerto serial y aplica filtros

    bool do_analysis_work = true;
    bool analysis_open = true;  // Sección Análisis abierta por defecto en UI
    void AnalysisWorker();      // Hilo que calcula FFT periódicamente

    float sidebar_width = 240;  // Ancho del panel lateral de control (píxeles)

    void ToggleFreeze();  // Alterna entre modo congelado y en vivo
    void DrawSidebar();   // Dibuja el panel lateral con todos los controles

public:
    bool open = true;
    void Draw();  // Renderiza toda la interfaz gráfica (llamado desde el main loop)

    void SetSize(int width, int height);  // Actualiza dimensiones de ventana
};
