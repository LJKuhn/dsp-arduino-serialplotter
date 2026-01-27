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

    enum class Filter {
        None,
        LowPass,
        HighPass
    };

    Serial serial;
    std::thread serial_thread, analysis_thread;
    std::mutex analysis_mutex;
    std::condition_variable analysis_cv;
    std::mutex data_mutex;  // Protege acceso concurrente a scrollX, scrollY y filter_scrollY

    FFT* fft = nullptr;
    ScrollBuffer<double>* scrollX = nullptr, * scrollY = nullptr, * filter_scrollY = nullptr;

    int max_time = 120;

    float max_time_visible = 5;

    time_point start_time = clock::now();

    int max = 0;

    // Límites de zoom del gráfico (sincronizados entre Entrada y Salida)
    double left_limit = 0, right_limit = max_time_visible;
    double down_limit = -7, up_limit = 7;

    double next_time = 0;

    // Cantidad de puntos a dibujar
    int size = 0;

    std::vector<uint8_t> read_buffer, write_buffer;

    Settings* settings;
    SettingsWindow* settingsWindow;

    int min_cutoff_frequency = 1, max_cutoff_frequency = 100;
    int cutoff_frequency[3] = { 0, 20, 100 };
    Filter selected_filter = Filter::None;  // Estado inicial sin filtro

    int width, height;

    // Variables para el modo freeze (congelar visualización sin detener adquisición)
    bool frozen = false;
    double frozen_left_limit = 0, frozen_right_limit = 5;
    double frozen_down_limit = -7, frozen_up_limit = 7;
    int frozen_size = 0;
    
    // Buffers para almacenar snapshot de datos cuando se congela
    std::vector<double> frozen_dataX;
    std::vector<double> frozen_dataY;
    std::vector<double> frozen_dataY_filtered;

public:
    MainWindow(int width, int height, Settings& config, SettingsWindow& ventanaConfig);
    ~MainWindow();

private:
    void CreateBuffers();
    void DestroyBuffers();

    double TransformSample(uint8_t v);
    uint8_t InverseTransformSample(double v);

    bool started = false;
    void ToggleConnection();

    void Start();
    void Stop();

    void SelectFilter(Filter filter);
    void SetupFilter();
    static void ResetFilters();

    bool do_serial_work = true;
    bool filter_open = true;  // Sección Filtro abierta por defecto
    void SerialWorker();

    bool do_analysis_work = true;
    bool analysis_open = true;  // Sección Análisis abierta por defecto
    void AnalysisWorker();

    float sidebar_width = 240;  // Ancho del panel lateral de control

    void ToggleFreeze();  // Alterna entre modo congelado y en vivo
    void DrawSidebar();   // Dibuja el panel lateral con todos los controles

public:
    bool open = true;
    void Draw();

    void SetSize(int width, int height);
};
