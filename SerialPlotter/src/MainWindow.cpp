// MainWindow.cpp - Ventana principal del visualizador de señales
//
// Arquitectura:
// - Panel lateral izquierdo (240px): Controles de puerto, configuración y conexión
// - Área de gráficos derecha: 3 gráficos apilados verticalmente
//   1. Entrada: señal cruda recibida por serial
//   2. Salida: señal filtrada (pasa bajos, pasa altos o ninguno)
//   3. Espectro: análisis FFT de la señal
//
// Modo Congelar:
// - Permite "pausar" la visualización sin detener la adquisición de datos
// - Guarda un snapshot (copia completa) de los datos actuales en frozen_dataX/Y/Y_filtered
// - Permite hacer zoom independiente del modo en vivo
// - La adquisición continúa en segundo plano y se puede reanudar sin pérdida
//
// Thread-safety:
// - SerialWorker (serial_thread): lee datos del puerto serial y actualiza scrollX, scrollY y filter_scrollY
//   * Protegido por data_mutex para evitar condiciones de carrera durante freeze/unfreeze
// - AnalysisWorker (analysis_thread): calcula FFT periódicamente cuando está en modo en vivo
//   * Pausado automáticamente en modo congelado para no procesar datos nuevos
// - Draw (UI thread): visualiza datos congelados (snapshot) o en vivo (buffers circulares)
//   * Lee buffers de forma thread-safe usando data_mutex solo durante la copia del snapshot

#include <imgui.h>
#include <imgui_internal.h>

#include <implot.h>
#include <Iir.h>
#include <thread>

#include "MainWindow.h"

#include "Buffers.h"
#include "Settings.h"

// Velocidades de comunicación serial estándar (bits por segundo)
const int bauds[] = { 1200, 2400, 4800, 9600, 14400, 19200, 38400, 57600, 115200, 230400, 250000, 460800, 500000, 921600, 1000000, 2000000 };

// Frecuencias de muestreo disponibles (Hz) - deben coincidir con las opciones de baud rate
const int frecuencias[] = { 120, 240, 480, 960, 1440, 1920, 3840, 5760, 11520, 23040, 25000, 46080, 50000, 92160, 100000, 2000000 };

#include "Widgets.h"

using namespace std::chrono_literals;

Iir::Butterworth::LowPass<8> lowpass_filter;
Iir::Butterworth::HighPass<8> highpass_filter;

// Declaraciones de funciones de Settings.cpp
void ComboFrecuenciaMuestreo(int& selected);
void ComboBaudRate(int& selected);
void ComboPuertos(std::string& selected_port);

void MenuPuertos(std::string& selected_port) {
    std::function to_string = [](std::string s) { return s; };

    select_menu("Puerto", selected_port, std::function(EnumerateComPorts), to_string,
                "No hay ningún dispositivo conectado");
}

bool Button(const char* label, bool disabled = false) {
    if (disabled) {
        // Simular botón deshabilitado (ImGui no tiene disable nativo en versiones antiguas)
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

        ImGui::Button(label);

        ImGui::PopStyleVar();
        ImGui::PopItemFlag();
        return false;
    }
    return ImGui::Button(label);
}

bool Button(const char* label, ImVec2 size, bool disabled = false) {
    if (disabled) {
        // Simular botón deshabilitado con transparencia reducida
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

        ImGui::Button(label, size);

        ImGui::PopStyleVar();
        ImGui::PopItemFlag();
        return false;
    }
    return ImGui::Button(label, size);
}

std::string MetricFormatter(double value, std::string_view unit) {
    static double v[] = { 1e12, 1e9, 1e6, 1e3, 1, 1e-3, 1e-6, 1e-9, 1e-12 };
    static const char* p[] = { "T", "G", "M", "k", "", "m", "u", "n", "p" };

    if (value == 0) {
        return "";
    }

    // Seleccionar el prefijo métrico apropiado (T, G, M, k, m, u, n, p)
    for (int i = 0; i < std::size(p); ++i) {
        if (fabs(value) >= v[i]) {
            return std::format("{:g} {}{}", value / v[i], p[i], unit);
        }
    }
    return "";
}

int MetricFormatter(double value, char* buff, int size, void* data) {
    const char* unit = (const char*)data;
    static double v[] = { 1e12, 1e9, 1e6, 1e3, 1, 1e-3, 1e-6, 1e-9, 1e-12 };
    static const char* p[] = { "T", "G", "M", "k", "", "m", "u", "n", "p" };

    if (value == 0) {
        return snprintf(buff, size, "0 %s", unit);
    }

    // Seleccionar el prefijo métrico apropiado (T, G, M, k, m, u, n, p)
    for (int i = 0; i < std::size(p); ++i) {
        if (fabs(value) >= v[i]) {
            return snprintf(buff, size, "%g %s%s", value / v[i], p[i], unit);
        }
    }
    return snprintf(buff, size, "%g %s%s", value / v[std::size(v) - 1], p[std::size(p) - 1], unit);
}


MainWindow::MainWindow(int width, int height, Settings& config, SettingsWindow& ventanaConfig) :
    settings(&config), settingsWindow(&ventanaConfig), width(width), height(height)
{
    CreateBuffers();
}

MainWindow::~MainWindow()
{
    Stop();
    DestroyBuffers();
}

void MainWindow::CreateBuffers() {
    int speed = settings->sampling_rate;
    int max_size = speed * max_time;  // Buffer para max_time segundos de datos
    size = 0;
    int view_size = 30 * speed;  // Vista inicial de 30 segundos
    next_time = 0;

    DestroyBuffers();
    read_buffer.resize(128);
    write_buffer.resize(128);

    // Crear buffers circulares para datos en tiempo real
    fft = new FFT(settings->sampling_rate);
    scrollX = new ScrollBuffer<double>(max_size, view_size);
    scrollY = new ScrollBuffer<double>(max_size, view_size);
    filter_scrollY = new ScrollBuffer<double>(max_size, view_size);
}

void MainWindow::DestroyBuffers() {
    delete scrollX;
    delete scrollY;
    delete filter_scrollY;
}

double MainWindow::TransformSample(uint8_t v) {
    return (v - settings->minimum) * settings->map_factor - 6;
}

uint8_t MainWindow::InverseTransformSample(double v) {
    double result = round((v + 6) * (settings->maximum - settings->minimum) / 12.0 + settings->minimum);
    if (result < 0)
        return 0;
    if (result > 255)
        return 255;
    return (int)result;
}


void MainWindow::ToggleConnection()
{
    if (!started) {
        if (settings->port.empty())
            return;

        Start();
    }
    else {
        Stop();
    }
    started = !started;
}

void MainWindow::ToggleFreeze()
{
    frozen = !frozen;
    
    if (frozen) {
        // Guardar límites de zoom actuales para modo congelado independiente
        frozen_left_limit = left_limit;
        frozen_right_limit = right_limit;
        frozen_down_limit = down_limit;
        frozen_up_limit = up_limit;
        
        // Copiar snapshot de datos actuales de forma thread-safe
        // El mutex protege contra escrituras del SerialWorker mientras copiamos
        std::lock_guard<std::mutex> lock(data_mutex);
        
        if (scrollX && scrollY && filter_scrollY) {
            frozen_size = scrollX->count();
            
            if (frozen_size > 0) {
                frozen_dataX.resize(frozen_size);
                frozen_dataY.resize(frozen_size);
                frozen_dataY_filtered.resize(frozen_size);
                
                // Copiar elemento por elemento (el operador [] maneja el offset del buffer circular)
                for (int i = 0; i < frozen_size; i++) {
                    frozen_dataX[i] = (*scrollX)[i];
                    frozen_dataY[i] = (*scrollY)[i];
                    frozen_dataY_filtered[i] = (*filter_scrollY)[i];
                }
            }
        }
    }
    else {
        // Liberar memoria del snapshot al reanudar modo en vivo
        frozen_dataX.clear();
        frozen_dataY.clear();
        frozen_dataY_filtered.clear();
    }
}

void MainWindow::DrawSidebar()
{
    static int stride_exp = 2;  // Exponente para calcular stride (2^n)
    
    // Panel lateral izquierdo: ocupa toda la altura de la ventana
    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize(ImVec2(sidebar_width, height));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
    
    ImGui::Begin("Panel de Control", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | 
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
    
    ImGui::PopStyleVar(2);

    // Título principal
    ImGui::TextColored(ImVec4(0.110f, 0.784f, 0.035f, 1.0f), "CONTROL");
    ImGui::Separator();
    ImGui::Spacing();

    // === SECCIÓN PUERTO ===
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.110f, 0.784f, 0.035f, 1.0f));
    ImGui::Text("PUERTO");
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();
    ComboPuertos(settings->port);
    ImGui::Spacing();

    // === SECCIÓN CONFIGURACIÓN ===
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.110f, 0.784f, 0.035f, 1.0f));
    ImGui::Text("CONFIGURACION");
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Frecuencia de muestreo (actualiza samples y baud_rate automáticamente)
    int old_sampling = settings->sampling_rate;
    ComboFrecuenciaMuestreo(settings->sampling_rate);
    if (old_sampling != settings->sampling_rate) {
        settings->samples = settings->sampling_rate;
        settings->baud_rate = settings->sampling_rate * 10;  // Relación 10:1 para transmisión estable
    }
    
    ComboBaudRate(settings->baud_rate);
    
    // Mapeo de valores ADC (0-255) a voltaje (-6V a +6V)
    if (ImGui::SliderInt("Maximo", &settings->maximum, 0, 255)) {
        settings->map_factor = 12.0 / (settings->maximum - settings->minimum);
    }
    
    if (ImGui::SliderInt("Minimo", &settings->minimum, 0, 255)) {
        settings->map_factor = 12.0 / (settings->maximum - settings->minimum);
    }
    
    // Stride: dibuja 1 de cada 2^n muestras para mejorar rendimiento
    if (ImGui::SliderInt("Stride", &stride_exp, 0, 10)) {
        settings->stride = (int)exp2(stride_exp);
        settings->byte_stride = sizeof(double) * settings->stride;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Dibuja 1 de cada 2^n muestras");
    }
    
    ImGui::Checkbox("Mostrar FPS", &settings->show_frame_time);
    ImGui::Spacing();

    // === SECCIÓN CONEXIÓN ===
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.110f, 0.784f, 0.035f, 1.0f));
    ImGui::Text("CONEXION");
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Botón Conectar/Desconectar (deshabilitado si no hay puerto seleccionado)
    if (Button(started ? "Desconectar" : "Conectar", ImVec2(-1, 0), settings->port.empty())) {
        ToggleConnection();
    }
    if (settings->port.empty()) {
        ImGui::SetItemTooltip("Selecciona un dispositivo primero");
    }
    ImGui::Spacing();

    // Botón Congelar/Reanudar (solo visible cuando hay conexión activa)
    if (started) {
        if (frozen) {
            // Botón verde brillante cuando está en modo congelado
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.9f, 0.05f, 1.0f));
            if (ImGui::Button("Reanudar", ImVec2(-1, 0))) {
                ToggleFreeze();
            }
            ImGui::PopStyleColor();
        }
        else {
            if (ImGui::Button("Congelar", ImVec2(-1, 0))) {
                ToggleFreeze();
            }
        }
        ImGui::SetItemTooltip("Congela la visualización para analizar sin detener la adquisición");
    }

    // === SECCIÓN INFORMACIÓN (siempre en la parte inferior del sidebar) ===
    float info_start_y = ImGui::GetWindowHeight() - 80;
    if (ImGui::GetCursorPosY() < info_start_y) {
        ImGui::SetCursorPosY(info_start_y);
    }
    
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.110f, 0.784f, 0.035f, 1.0f));
    ImGui::Text("INFORMACION");
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Tiempo transcurrido (del snapshot congelado o de datos en vivo)
    double elapsed = 0.0;
    if (started) {
        if (frozen && !frozen_dataX.empty()) {
            elapsed = frozen_dataX.back();
        }
        else if (!frozen && scrollX && scrollX->count() > 0) {
            elapsed = scrollX->back();
        }
    }
    ImGui::Text("Tiempo: %.1fs", elapsed);
    
    // Indicador visual de estado congelado
    if (frozen) {
        ImGui::TextColored(ImVec4(0.110f, 0.784f, 0.035f, 1.0f), "[CONGELADO]");
    }
    
    // FPS (si está habilitado en configuración)
    if (settings->show_frame_time) {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("FPS: %.1f", io.Framerate);
    }

    ImGui::End();
}

void MainWindow::Start() {
    CreateBuffers();

    left_limit = 0, right_limit = max_time_visible;

    // Configurar parámetros de filtros según frecuencia de muestreo
    SetupFilter();
    ResetFilters();

    // Iniciar hilos de trabajo en paralelo
    do_serial_work = true;
    do_analysis_work = true;
    analysis_thread = std::thread(&MainWindow::AnalysisWorker, this);

    // Abrir puerto serial y comenzar adquisición
    serial.open(settings->port, settings->baud_rate);
    serial_thread = std::thread(&MainWindow::SerialWorker, this);
    start_time = clock::now();
}

void MainWindow::Stop() {
    // Señalizar a los hilos que deben terminar
    do_serial_work = false;
    do_analysis_work = false;
    analysis_cv.notify_one();  // Despertar AnalysisWorker si está esperando

    // Esperar a que terminen los hilos de forma ordenada
    if (serial_thread.joinable())
        serial_thread.join();
    if (analysis_thread.joinable())
        analysis_thread.join();
    
    // Cerrar puerto serial
    serial.close();
}

void MainWindow::SelectFilter(Filter filter) {
    selected_filter = filter;
    
    // Ajustar rango de frecuencia de corte según el tipo de filtro
    switch (selected_filter)
    {
        case Filter::LowPass:
            // Pasa bajos: frecuencia de corte entre 1 Hz y Nyquist/2
            min_cutoff_frequency = 1;
            max_cutoff_frequency = settings->sampling_rate / 4;
            break;
        case Filter::HighPass:
            // Pasa altos: frecuencia de corte entre Nyquist/2 y casi Nyquist
            min_cutoff_frequency = settings->sampling_rate / 4;
            max_cutoff_frequency = settings->sampling_rate / 2 - 1;
            break;
        case Filter::None:
            break;
    }
}

void MainWindow::SetupFilter() {
    switch (selected_filter)
    {
        case Filter::LowPass:
            lowpass_filter.setup(settings->sampling_rate, cutoff_frequency[1]);
            break;
        case Filter::HighPass:
            highpass_filter.setup(settings->sampling_rate, cutoff_frequency[2]);
            break;
        case Filter::None:
            break;
    }
}

void MainWindow::ResetFilters() {
    lowpass_filter.reset();
    highpass_filter.reset();
}

void MainWindow::SerialWorker() {
    while (do_serial_work) {
        // Leer muestras del puerto serial (pocas por iteración para reducir latencia)
        int read = serial.read(read_buffer.data(), 1);

        if (read > 0) {
            // Proteger escritura en buffers contra acceso concurrente durante freeze/unfreeze
            std::lock_guard<std::mutex> lock(data_mutex);
            
            for (size_t i = 0; i < read; i++)
            {
                // Transformar valor ADC (0-255) a voltaje (-6V a +6V aproximadamente)
                double transformado = TransformSample(read_buffer[i]);

                scrollY->push(transformado);
                scrollX->push(next_time);

                double resultado = transformado;

                // Aplicar filtro seleccionado (pasa bajos, pasa altos o ninguno)
                switch (selected_filter)
                {
                    case Filter::LowPass:
                        resultado = lowpass_filter.filter(transformado);
                        break;
                    case Filter::HighPass:
                        resultado = highpass_filter.filter(transformado);
                        break;
                    case Filter::None:
                        break;
                }

                filter_scrollY->push(resultado);
                next_time += 1.0 / settings->sampling_rate;

                // Enviar señal procesada de vuelta (invertida para DAC)
                write_buffer[i] = 255 - InverseTransformSample(resultado);
            }

            size = scrollX->count();
        }

        // Enviar datos procesados de vuelta por serial
        serial.write(write_buffer.data(), read);
    }
}

void MainWindow::AnalysisWorker() {
    while (do_analysis_work) {
        std::unique_lock lock(analysis_mutex);
        // Esperar notificación desde Draw() - solo se notifica en modo en vivo
        analysis_cv.wait(lock);

        if (!fft || !scrollY)
            continue;

        // Tomar hasta 1 segundo de muestras para el análisis FFT
        uint32_t available = scrollY->count();
        uint32_t max = settings->sampling_rate;
        uint32_t count = available > max ? max : available;

        auto end = scrollY->data() + available;
        fft->SetData(end - count, count);
        fft->Compute();

        std::this_thread::sleep_for(100ms);
    }
}

void MainWindow::Draw()
{
    static double elapsed_time = 0;

    // Actualizar tiempo transcurrido y límites de zoom automático solo en modo en vivo
    if (started && scrollX && scrollX->count() > 0 && !frozen) {
        elapsed_time = scrollX->back();

        // Auto-scroll: mantener ventana visible de max_time_visible segundos
        if (elapsed_time > max_time_visible) {
            right_limit = elapsed_time;
            left_limit = elapsed_time - max_time_visible;
        }
    }

    // Seleccionar fuente de datos según el estado (congelado vs en vivo)
    const double* dataX = nullptr;
    const double* dataY = nullptr;
    const double* dataY_filtered = nullptr;
    int current_draw_size = 0;
    
    if (frozen && !frozen_dataX.empty()) {
        // Modo congelado: usar snapshot guardado (no se actualiza hasta reanudar)
        dataX = frozen_dataX.data();
        dataY = frozen_dataY.data();
        dataY_filtered = frozen_dataY_filtered.data();
        current_draw_size = frozen_size / settings->stride;
    }
    else {
        // Modo en vivo: usar buffers circulares actuales (actualizados por SerialWorker)
        dataX = scrollX ? scrollX->data() : nullptr;
        dataY = scrollY ? scrollY->data() : nullptr;
        dataY_filtered = filter_scrollY ? filter_scrollY->data() : nullptr;
        current_draw_size = size / settings->stride;
    }

    // Dibujar panel lateral con controles
    DrawSidebar();

    // Ventana principal: área de gráficos a la derecha del sidebar
    ImGui::SetNextWindowPos({ sidebar_width, 0 });
    ImGui::SetNextWindowSize(ImVec2(width - sidebar_width, height));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    ImGui::Begin("Ventana principal", &open,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar(2);

    auto win_pos = ImGui::GetWindowPos();
    auto win_size = ImGui::GetWindowSize();

    // Calcular altura para distribuir los 3 gráficos equitativamente
    float available_height = ImGui::GetContentRegionAvail().y;
    float header_height = 25.0f;  // Espacio para headers colapsables
    float graph_height = (available_height - header_height * 2) / 3.0f;

    // === GRÁFICO 1: ENTRADA (señal cruda) ===
    if (ImPlot::BeginPlot("Entrada", { -1, graph_height }, ImPlotFlags_NoLegend)) {
        // Configurar ejes según el estado de freeze
        if (frozen) {
            // Modo congelado: zoom manual independiente del modo en vivo
            ImPlot::SetupAxisLinks(ImAxis_X1, &frozen_left_limit, &frozen_right_limit);
            ImPlot::SetupAxisLinks(ImAxis_Y1, &frozen_down_limit, &frozen_up_limit);
            
            ImPlot::SetupAxisFormat(ImAxis_Y1, MetricFormatter, (void*)"V");
            ImPlot::SetupAxisFormat(ImAxis_X1, MetricFormatter, (void*)"s");

            ImPlot::SetupAxisLimits(ImAxis_Y1, frozen_down_limit, frozen_up_limit, ImGuiCond_Once);
            ImPlot::SetupAxisLimits(ImAxis_X1, frozen_left_limit, frozen_right_limit, ImGuiCond_Once);
        }
        else {
            // Modo en vivo: zoom sincronizado con gráfico de Salida (comparten left/right/down/up_limit)
            ImPlot::SetupAxisLinks(ImAxis_X1, &left_limit, &right_limit);
            ImPlot::SetupAxisLinks(ImAxis_Y1, &down_limit, &up_limit);
            
            ImPlot::SetupAxisFormat(ImAxis_Y1, MetricFormatter, (void*)"V");
            ImPlot::SetupAxisFormat(ImAxis_X1, MetricFormatter, (void*)"s");

            ImPlot::SetupAxisLimits(ImAxis_Y1, -7, 7, ImGuiCond_FirstUseEver);
            ImPlot::SetupAxisLimits(ImAxis_X1, left_limit, right_limit, started ? ImGuiCond_Always : ImGuiCond_None);
        }

        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0, INFINITY);

        // Dibujar línea con color verde #1CC809
        if (dataX && dataY && current_draw_size > 0) {
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.110f, 0.784f, 0.035f, 1.0f));
            ImPlot::PlotLine("", dataX, dataY, current_draw_size, 0, 0, settings->byte_stride);
            ImPlot::PopStyleColor();
        }
        ImPlot::EndPlot();
    }

    // === SECCIÓN FILTRO (colapsable) ===
    filter_open = ImGui::CollapsingHeader("Filtro", ImGuiTreeNodeFlags_DefaultOpen);
    if (filter_open) {
        // === GRÁFICO 2: SALIDA (señal filtrada) ===
        if (ImPlot::BeginPlot("Salida", { -1, graph_height }, ImPlotFlags_NoLegend)) {
            // Configurar ejes según el estado de freeze
            if (frozen) {
                // Modo congelado: zoom manual independiente (usa frozen_*_limit)
                ImPlot::SetupAxisLinks(ImAxis_X1, &frozen_left_limit, &frozen_right_limit);
                ImPlot::SetupAxisLinks(ImAxis_Y1, &frozen_down_limit, &frozen_up_limit);
                
                ImPlot::SetupAxisFormat(ImAxis_Y1, MetricFormatter, (void*)"V");
                ImPlot::SetupAxisFormat(ImAxis_X1, MetricFormatter, (void*)"s");

                ImPlot::SetupAxisLimits(ImAxis_Y1, frozen_down_limit, frozen_up_limit, ImGuiCond_Once);
                ImPlot::SetupAxisLimits(ImAxis_X1, frozen_left_limit, frozen_right_limit, ImGuiCond_Once);
            }
            else {
                // Modo en vivo: zoom sincronizado con gráfico de Entrada (comparten left/right/down/up_limit)
                ImPlot::SetupAxisLinks(ImAxis_X1, &left_limit, &right_limit);
                ImPlot::SetupAxisLinks(ImAxis_Y1, &down_limit, &up_limit);
                
                ImPlot::SetupAxisFormat(ImAxis_Y1, MetricFormatter, (void*)"V");
                ImPlot::SetupAxisFormat(ImAxis_X1, MetricFormatter, (void*)"s");

                ImPlot::SetupAxisLimits(ImAxis_Y1, -7, 7, ImGuiCond_FirstUseEver);
                ImPlot::SetupAxisLimits(ImAxis_X1, left_limit, right_limit, started ? ImGuiCond_Always : ImGuiCond_None);
            }

            ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0, INFINITY);

            // Dibujar línea filtrada avec color verde #1CC809
            if (dataX && dataY_filtered && current_draw_size > 0) {
                ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.110f, 0.784f, 0.035f, 1.0f));
                ImPlot::PlotLine("", dataX, dataY_filtered, current_draw_size, 0, 0, settings->byte_stride);
                ImPlot::PopStyleColor();
            }
            ImPlot::EndPlot();
        }

        // Botones de selección de filtro
        const char* nombres[] = { "Ninguno", "Pasa bajos", "Pasa altos" };
        for (int i = 0; i < std::size(nombres); i++)
        {
            if (i > 0)
                ImGui::SameLine();

            if (i == (int)selected_filter) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
                ImGui::Button(nombres[i]);
                ImGui::PopStyleColor();
            }
            else {
                if (ImGui::Button(nombres[i])) {
                    SelectFilter((Filter)i);
                    ResetFilters();
                }
            }
        }

        // Control de frecuencia de corte (solo visible si hay filtro activo)
        if (selected_filter != Filter::None
            && ImGui::SliderInt("Frecuencia de corte", &cutoff_frequency[(int)selected_filter], min_cutoff_frequency, max_cutoff_frequency)) {
            SetupFilter();
            ResetFilters();
        }
    }

    // === SECCIÓN ANÁLISIS (colapsable) ===
    if (ImGui::CollapsingHeader("Análisis", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Notificar al worker de análisis FFT (solo en modo en vivo)
        // En modo congelado no se calcula FFT nuevo para evitar procesamiento innecesario
        if (!frozen) {
            analysis_cv.notify_one();
        }
        
        // === GRÁFICO 3: ESPECTRO (FFT) ===
        if (ImPlot::BeginPlot("Espectro", { -1, graph_height }, ImPlotFlags_NoLegend)) {
            ImPlot::SetupAxisFormat(ImAxis_Y1, MetricFormatter, (void*)"V");
            ImPlot::SetupAxisFormat(ImAxis_X1, MetricFormatter, (void*)"Hz");
            ImPlot::SetupAxisLimits(ImAxis_X1, 0.99, settings->samples, ImGuiCond_FirstUseEver);
            ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_AutoFit);

            ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
            ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, 0, INFINITY);

            fft->Plot(settings->sampling_rate);
            ImPlot::EndPlot();

            // Mostrar información de frecuencia dominante y offset DC
            if (scrollY && scrollY->count() > 0) {
                ImGui::Text("Frecuencia: %s\tDesplazamiento %s",
                            MetricFormatter(fft->Frequency(settings->sampling_rate), "Hz").data(),
                            MetricFormatter(fft->Offset(), "V").data()
                );
            }
        }
    }

    ImGui::End();
}

void MainWindow::SetSize(int width, int height) {
    this->width = width;
    this->height = height;
}
