// main.cpp - Punto de entrada principal de la aplicación
//
// Este archivo está basado en un ejemplo de Omar Cornut (creador de ImGui)
// Fuente: https://github.com/ocornut/imgui/blob/master/examples/example_glfw_opengl3/main.cpp
//
// Funcionalidad:
// - Inicialización de GLFW (ventana y contexto OpenGL)
// - Inicialización de ImGui e ImPlot
// - Configuración del tema visual (oscuro con acentos verdes #1CC809)
// - Bucle principal de renderizado con optimizaciones de rendimiento
// - Gestión de eventos de ventana (resize, minimize, focus)
// - Ocultación automática de consola en Windows
//
// Arquitectura del bucle principal:
// 1. Procesamiento de eventos (glfwPollEvents / glfwWaitEvents)
// 2. Limitación de FPS cuando la ventana no tiene foco (ahorro de CPU)
// 3. Inicio de frame ImGui
// 4. Renderizado de MainWindow y SettingsWindow
// 5. Renderizado OpenGL y swap de buffers

// The MIT License (MIT)
//
// Copyright (c) 2014-2023 Omar Cornut

#include "main.h"
#include "Settings.h"
#include "MainWindow.h"
#include "Console.h"


int width = 1280, height = 720;

Settings settings;
SettingsWindow settings_window(settings);

MainWindow mainWindow(width, height, settings, settings_window);

// Callback: actualizar dimensiones cuando la ventana cambia de tamaño
void window_resize(GLFWwindow*, int w, int h) {
    width = w;
    height = h;
    mainWindow.SetSize(w, h);
}

bool minimized = false;
// Callback: detectar cuando la ventana se minimiza (para pausar renderizado)
void window_minimized(GLFWwindow*, int _minimized) {
    minimized = _minimized;
}

bool focused = true;
// Callback: detectar cuando la ventana pierde el foco (para limitar FPS)
void window_focused(GLFWwindow*, int _focused) {
    focused = _focused;
}

// Main code
int main(int, char**)
{
    // Ocultar consola de Windows si pertenece a este proceso
    Console console;
    if (console.IsOwn())
        console.Hide(true);

    // Inicializar GLFW
    if (!glfwInit())
        return -1;

    // Configurar versión de OpenGL según la plataforma
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
        const char* glsl_version = "#version 100";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 330
        const char* glsl_version = "#version 330";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.3 + GLSL 330 (Windows/Linux)
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
#endif

    // Crear ventana con contexto gráfico
    GLFWwindow* window = glfwCreateWindow(width, height, "Procesamiento Digital de Señales", nullptr, nullptr);
    if (window == nullptr)
        return -1;

    // Registrar callbacks de eventos
    glfwSetWindowSizeCallback(window, window_resize);
    glfwSetWindowIconifyCallback(window, window_minimized);
    glfwSetWindowFocusCallback(window, window_focused);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Maximizar ventana al inicio para aprovechar toda la pantalla
    glfwMaximizeWindow(window);
    
    // Actualizar dimensiones reales después de maximizar
    glfwGetFramebufferSize(window, &width, &height);
    mainWindow.SetSize(width, height);

    // Cargar funciones de OpenGL con GLAD
    if (!gladLoadGL()) {
        glfwTerminate();
        return -1;
    }

    // Inicializar ImGui e ImPlot
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Aplicar tema oscuro personalizado con paleta #111112 (fondo) y #1CC809 (verde)
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Conversión de colores HEX a RGB float (0-1):
    // #111112 = RGB(17, 17, 18) -> (0.067, 0.067, 0.071)
    // #1CC809 = RGB(28, 200, 9) -> (0.110, 0.784, 0.035)
    
    // Fondos y bordes
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.067f, 0.067f, 0.071f, 1.0f);           // #111112
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.067f, 0.067f, 0.071f, 1.0f);            // #111112
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.067f, 0.067f, 0.071f, 0.95f);           // #111112
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);                // Negro
    style.Colors[ImGuiCol_Border] = ImVec4(0.3f, 0.3f, 0.3f, 0.5f);                   // Gris oscuro
    
    // Texto
    style.Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);                  // Blanco
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);             // Gris
    
    // Botones - Variantes de #1CC809 (verde)
    style.Colors[ImGuiCol_Button] = ImVec4(0.110f, 0.784f, 0.035f, 0.8f);             // #1CC809
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.15f, 0.9f, 0.05f, 1.0f);          // Más brillante
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.08f, 0.65f, 0.03f, 1.0f);          // Más oscuro
    
    // Headers y secciones colapsables
    style.Colors[ImGuiCol_Header] = ImVec4(0.110f, 0.784f, 0.035f, 0.5f);             // #1CC809 semi-transparente
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.110f, 0.784f, 0.035f, 0.7f);      // #1CC809
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.110f, 0.784f, 0.035f, 1.0f);       // #1CC809 sólido
    
    // Frames (inputs, sliders)
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);                  // Negro
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.067f, 0.067f, 0.071f, 1.0f);     // #111112
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.110f, 0.784f, 0.035f, 0.3f);      // #1CC809 tenue
    
    // Sliders
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.110f, 0.784f, 0.035f, 1.0f);         // #1CC809
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.15f, 0.9f, 0.05f, 1.0f);       // Más brillante
    
    // Checkboxes
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.110f, 0.784f, 0.035f, 1.0f);          // #1CC809
    
    // Títulos de ventanas
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);                  // Negro
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.067f, 0.067f, 0.071f, 1.0f);      // #111112
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);        // Negro transparente
    
    // Scrollbars
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);              // Negro semi-transparente
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.110f, 0.784f, 0.035f, 0.5f);      // #1CC809
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.110f, 0.784f, 0.035f, 0.7f); // #1CC809
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.110f, 0.784f, 0.035f, 1.0f);  // #1CC809 sólido
    
    // Separadores
    style.Colors[ImGuiCol_Separator] = ImVec4(0.110f, 0.784f, 0.035f, 0.4f);          // #1CC809 tenue
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.110f, 0.784f, 0.035f, 0.7f);   // #1CC809
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.110f, 0.784f, 0.035f, 1.0f);    // #1CC809

    style.WindowMenuButtonPosition = ImGuiDir_None;  // Ocultar botón de menú de ventana

    // Color de fondo negro para ventana OpenGL
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);

    double last_time = glfwGetTime();
    int minimized_fps = 20;
    double minimized_frametime = 1.0 / minimized_fps;

    // Bucle principal de renderizado
    while (!glfwWindowShouldClose(window))
    {
        // Optimización: esperar eventos si la ventana está minimizada (ahorro de CPU)
        if (minimized)
            glfwWaitEvents();
        else
            glfwPollEvents();

        // Optimización: limitar FPS a 20 cuando la ventana no tiene foco (ahorro de CPU)
        if (!focused) {
            if (minimized_frametime > 0.02)
                std::this_thread::sleep_for(std::chrono::duration<double>(minimized_frametime - 0.02));

            double elapsed = glfwGetTime() - last_time;
            while (elapsed < minimized_frametime)
                elapsed = glfwGetTime() - last_time;
            last_time = glfwGetTime();
        }

        // Iniciar frame de ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Dibujar interfaz de usuario
        mainWindow.Draw();
        settings_window.Draw();

        // Renderizar OpenGL
        glViewport(0, 0, width, height);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // Renderizar ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Intercambiar buffers (presentar frame)
        glfwSwapBuffers(window);
    }

    // Limpieza y cierre
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
