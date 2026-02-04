// main.h - Cabecera principal con todas las dependencias del proyecto
//
// Incluye todas las librerías necesarias para el proyecto:
// - C++ STL: thread, queue, chrono, concepts, etc.
// - GLFW: gestión de ventanas y contexto OpenGL
// - GLAD: cargador de funciones OpenGL modernas
// - ImGui: interfaz gráfica inmediata
// - ImPlot: gráficos y visualizaciones
// - Iir: filtros digitales IIR (Butterworth)
// - Clases propias: Serial, Buffers, FFT
//
// Esta cabecera centraliza todas las dependencias para facilitar
// la compilación y evitar problemas de orden de inclusión.

#pragma once

// Librerías estándar de C++20
#include <thread>
#include <queue>
#include <fstream>
#include <chrono>
#include <set>
#include <type_traits>
#include <concepts>
#include <cmath>

// OpenGL y gestión de ventanas
#include <glad/glad.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// Interfaz gráfica ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Gráficos y filtros
#include <implot.h>
#include <imgui_internal.h>
#include <Iir.h>

// Módulos propios del proyecto
#include "Serial.h"
#include "Buffers.h"
#include "FFT.h"
