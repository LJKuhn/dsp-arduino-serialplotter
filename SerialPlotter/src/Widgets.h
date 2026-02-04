// Widgets.h - Widgets reutilizables para interfaz ImGui
//
// Proporciona templates genéricos para crear controles de UI comunes:
// - select_menu: menú desplegable para seleccionar valores
// - combo: combobox para seleccionar valores con búsqueda
//
// Estos widgets son genéricos y funcionan con cualquier tipo de contenedor
// (vector, array, list, etc.) gracias al uso de templates de C++.
//
// Ventajas:
// - Reutilización de código
// - Type-safe (verificación en tiempo de compilación)
// - Flexible (trabaja con cualquier tipo de dato)

#pragma once
#include <functional>
#include <imgui_internal.h>


// select_menu - Crea un menú desplegable (parte de una barra de menús) para seleccionar valores
//
// Template genérico que funciona con cualquier contenedor de C++ (vector, array, list, etc.)
//
// Parámetros:
//   title: texto del menú
//   selection: variable que almacenará el valor seleccionado (se modifica por referencia)
//   get_values: función que retorna el contenedor con los valores disponibles
//   to_string: función que convierte cada valor a string para visualización
//   empty_msg: mensaje a mostrar si el contenedor está vacío
//
// Ejemplo de uso:
//   std::string puerto = "COM3";
//   select_menu("Puerto", puerto, EnumerateComPorts, 
//               [](std::string s){ return s; }, "No hay puertos");
template <class Container>
void select_menu(const char* title, typename Container::value_type& selection, std::function<Container()> get_values, const std::function<std::string(typename Container::value_type)>& to_string, const char* empty_msg = "Vacio") {
    if (!ImGui::BeginMenu(title))
        return;

    Container values = get_values();
    if (std::empty(values)) {
        ImGui::Text(empty_msg);
        ImGui::EndMenu();
        return;
    }

    bool selected = true;
    for (const auto& value : values) {
        auto str = to_string(value);
        // Marcar el ítem actual como seleccionado
        if (ImGui::MenuItem(str.c_str(), nullptr, value == selection ? &selected : nullptr)) {
            selection = value;
        }
    }

    ImGui::EndMenu();
}

// combo - Crea un combobox (lista desplegable) para seleccionar valores
//
// Similar a select_menu pero usa el widget Combo de ImGui en lugar de MenuItem.
// Más apropiado para formularios y configuraciones.
//
// Parámetros:
//   title: etiqueta del combo
//   selection: variable que almacenará el valor seleccionado (se modifica por referencia)
//   values: contenedor con los valores disponibles (por referencia constante)
//   to_string: función que convierte cada valor a string para visualización
//   empty_msg: mensaje a mostrar si el contenedor está vacío
//
// Ejemplo de uso:
//   int baud = 9600;
//   std::vector<int> bauds = {1200, 2400, 9600, 115200};
//   combo("Velocidad", baud, bauds, [](int n){ return std::to_string(n); });
template <class Container, typename T>
void combo(const char* title, T& selection, const Container& values, const std::function<std::string(T)>& to_string, const char* empty_msg = "Vacio") {
    auto str = to_string(selection);
    if (!ImGui::BeginCombo(title, str.c_str()))
        return;

    if (std::empty(values)) {
        ImGui::Text(empty_msg);
        ImGui::EndCombo();
        return;
    }

    for (const auto& value : values) {
        auto str = to_string(value);
        bool isSelected = value == selection;
        // Resaltar el ítem seleccionado
        if (ImGui::Selectable(str.c_str(), &isSelected)) {
            selection = value;
        }
    }

    ImGui::EndCombo();
}