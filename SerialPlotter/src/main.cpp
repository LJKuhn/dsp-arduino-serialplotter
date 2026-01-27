// Este archivo está basado en un ejemplo de Omar Cornut
// Fuente: https://github.com/ocornut/imgui/blob/master/examples/example_glfw_opengl3/main.cpp

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

void window_resize(GLFWwindow*, int w, int h) {
    width = w;
    height = h;
    mainWindow.SetSize(w, h);
}

bool minimized = false;
void window_minimized(GLFWwindow*, int _minimized) {
    minimized = _minimized;
}

bool focused = true;
void window_focused(GLFWwindow*, int _focused) {
    focused = _focused;
}

// Main code
int main(int, char**)
{
    // Ocultar consola
    Console console;
    if (console.IsOwn())
        console.Hide(true);

    if (!glfwInit())
        return -1;

    // Decide GL+GLSL versions
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
    // GL 3.3 + GLSL 330
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(width, height, "Procesamiento Digital de Señales", nullptr, nullptr);
    if (window == nullptr)
        return -1;

    // Maximizar la ventana al iniciar
    glfwMaximizeWindow(window);

    glfwSetWindowSizeCallback(window, window_resize);
    glfwSetWindowIconifyCallback(window, window_minimized);
    glfwSetWindowFocusCallback(window, window_focused);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGL()) {
        glfwTerminate();
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup Dear ImGui style - Cambiar a tema oscuro
    ImGui::StyleColorsDark();
    
    // Personalizar colores con tu paleta
    ImGuiStyle& style = ImGui::GetStyle();
    
    // #111112 = RGB(17, 17, 18) -> Convertir a float: 17/255 = 0.067
    // #1CC809 = RGB(28, 200, 9) -> Convertir a float: (28/255, 200/255, 9/255) = (0.110, 0.784, 0.035)
    
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

    style.WindowMenuButtonPosition = ImGuiDir_None;

    // Color de fondo de la ventana OpenGL - Negro
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);

    double last_time = glfwGetTime();
    int minimized_fps = 20;
    double minimized_frametime = 1.0 / minimized_fps;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Espera para procesar los eventos si la ventana está minimizada 
        if (minimized)
            glfwWaitEvents();
        else
            glfwPollEvents();

        // Limitar FPS y uso de CPU si la ventana está abierta pero no tiene foco
        if (!focused) {
            if (minimized_frametime > 0.02)
                std::this_thread::sleep_for(std::chrono::duration<double>(minimized_frametime - 0.02));

            double elapsed = glfwGetTime() - last_time;
            while (elapsed < minimized_frametime)
                elapsed = glfwGetTime() - last_time;
            last_time = glfwGetTime();
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        mainWindow.Draw();
        settings_window.Draw();

        glViewport(0, 0, width, height);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
