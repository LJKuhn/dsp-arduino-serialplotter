//
// Created by USUARIO on 2/12/2023.
//

#include <iostream>
#include "Console.h"

// Console.cpp - Implementación de gestión de consola de Windows
//
// Permite controlar la visibilidad de la ventana de consola en aplicaciones Windows.
// Útil para aplicaciones GUI que no necesitan mostrar la consola por defecto.

Console::Console() {
    // Obtener handle de la ventana de consola actual
    console_window = GetConsoleWindow();

    if (console_window){
        // Guardar el estado actual de la ventana para restauración posterior
        WINDOWPLACEMENT placement;
        GetWindowPlacement(console_window, &placement);
        state = placement.showCmd;
    }
}

Console::~Console() {
    if (_restore){
        // Restaurar el estado original de la consola al destruir la instancia
        ShowWindow(console_window, state);
    }
}

void Console::Hide(bool persist) {
    if (!console_window)
        return;

    ShowWindow(console_window, SW_HIDE);
    if (persist)
        _restore = false;  // No restaurar en el destructor
}

void Console::Show(bool persist) {
    if (!console_window)
        return;

    ShowWindow(console_window, SW_SHOW);
    if (persist)
        _restore = false;  // No restaurar en el destructor
}

bool Console::IsOwn() {
    DWORD processId;
    GetWindowThreadProcessId(console_window, &processId);

    // Comparar el PID de la consola con el del proceso actual
    return GetCurrentProcessId() == processId;
}
