// Console.h - Gestión de la visibilidad de la consola de Windows
//
// La clase Console permite controlar la ventana de consola en aplicaciones Windows:
// - Ocultar/mostrar la consola de forma programática
// - Detectar si la consola pertenece al proceso actual
// - Restaurar automáticamente el estado original al finalizar (opcional)
//
// Uso típico: Ocultar la consola en aplicaciones GUI para evitar mostrar dos ventanas

#pragma once

#include <Windows.h>

class Console {
    HWND console_window;  // Handle de la ventana de consola
    int state;            // Estado original de la ventana (SW_SHOW, SW_HIDE, etc.)
    bool _restore = true; // Si debe restaurar el estado original al destruir

public:
    // Constructor - captura el handle y estado actual de la consola
    explicit Console();

    // Destructor - restaura el estado original si _restore es true
    ~Console();

    // Oculta la consola
    // persist: si es true, no restaurará el estado al destruir la instancia
    void Hide(bool persist = false);

    // Muestra la consola
    // persist: si es true, no restaurará el estado al destruir la instancia
    void Show(bool persist = false);

    // Verifica si la consola pertenece al proceso actual
    // Retorna true si el PID de la consola coincide con el del proceso
    bool IsOwn();
};
