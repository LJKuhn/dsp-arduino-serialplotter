// Serial.h - Comunicación serial con dispositivos externos (Windows).
//
// Proporciona una interfaz simplificada para comunicación serial en Windows.
// Encapsula las API nativas de Windows (CreateFile, ReadFile, WriteFile, etc.)
// para facilitar la comunicación con dispositivos como Arduino, sensores, etc.
//        
// Características:
// - Apertura y configuración de puertos COM (baud rate, paridad, bits de datos)
// - Lectura y escritura sincrónica de datos
// - Enumeración de puertos COM disponibles en el sistema
// - Gestión automática de timeouts y buffers
//

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>

// Imprime el mensaje de error de Windows correspondiente a un código de error
// error: código de error (si es -1, usa GetLastError())
void printErrorMessage(uint32_t error = -1);

// Enumera todos los puertos COM disponibles en el sistema
// Retorna un vector de strings con los nombres de los puertos (ej: "COM3", "COM4")
std::vector<std::string> EnumerateComPorts();

// Clase para gestionar comunicación serial con puertos COM
class Serial {
    HANDLE file = nullptr;  // Handle del archivo/dispositivo COM abierto

public:
    ~Serial();

    // Abre un puerto COM con la velocidad especificada
    // port: nombre del puerto (ej: "COM3" o "\\\\.\\COM3")
    // baud: velocidad en baudios (bits por segundo)
    // Retorna true si se abrió correctamente
    bool open(std::string port, int baud = 9600);

    // Lee datos del puerto serial
    // buffer: buffer de destino para los datos leídos
    // size: cantidad máxima de bytes a leer
    // Retorna la cantidad de bytes realmente leídos
    int read(uint8_t* buffer, int size);

    // Escribe datos al puerto serial
    // buffer: buffer con los datos a escribir
    // size: cantidad de bytes a escribir
    // Retorna la cantidad de bytes realmente escritos
    int write(uint8_t* buffer, int size);

    // Cierra el puerto serial
    void close();

    // Devuelve la cantidad de bytes disponibles para lectura en el buffer
    size_t available();
};