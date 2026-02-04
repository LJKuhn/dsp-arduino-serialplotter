// Serial.cpp - Implementación de comunicación serial para Windows
//
// Usa las APIs nativas de Windows para comunicación con puertos COM.
// Configura automáticamente los parámetros de comunicación y timeouts.

#include "Serial.h"
#include <format>
#include <algorithm>

// Código basado en: https://stackoverflow.com/a/17387176
void printErrorMessage(uint32_t error) {
    static WCHAR messageBuffer[128];

    if (error == -1)
        error = GetLastError();  // Obtener último error de Windows
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    // Formatear mensaje de error usando la API de Windows
    size_t size = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), messageBuffer, 128, nullptr);

    WriteConsoleW(out, messageBuffer, size, nullptr, nullptr);
}

// Función para enumerar puertos COM disponibles en el sistema
std::vector<std::string> EnumerateComPorts() {
    std::vector<std::string> com_ports;

    // Acceder al registro de Windows donde se almacenan los puertos COM
    HKEY key;
    LSTATUS result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_QUERY_VALUE, &key);
    if (result != ERROR_SUCCESS)
        return com_ports;

    // Enumerar todos los valores en la clave del registro
    for (DWORD i = 0; result != ERROR_NO_MORE_ITEMS; i++)
    {
        char name[64];
        uint8_t data[64];
        DWORD nameChars = 64, dataSize = 64;
        DWORD type;
        result = RegEnumValueA(key, i, name, &nameChars, nullptr, &type, data, &dataSize);

        if (result == ERROR_SUCCESS)
            com_ports.emplace_back((char*)data);  // data contiene el nombre del puerto (ej: "COM3")
    }
    RegCloseKey(key);

    // Ordenar alfabéticamente para presentación
    std::sort(com_ports.begin(), com_ports.end());
    return com_ports;
}

Serial::~Serial() {
    close();
}

// Función para abrir un puerto serial
bool Serial::open(std::string port, int baud) {
    DWORD access = GENERIC_READ | GENERIC_WRITE;
    DWORD mode = OPEN_EXISTING;

    // Agregar prefijo necesario para puertos COM (especialmente COM10+)
    const std::string prefijo = "\\\\.\\";
    if (!port.starts_with(prefijo))
        port.insert(0, prefijo);
    
    // Abrir el dispositivo COM como archivo
    file = CreateFileA(port.c_str(), access, 0, 0, mode, 0, 0);

    if (file == INVALID_HANDLE_VALUE) {
        printErrorMessage();
        return false;
    }

    // Configurar buffers de comunicación (2KB entrada y salida)
    SetupComm(file, 2048, 2048);
    
    // Limpiar buffers previos
    PurgeComm(file, PURGE_RXABORT | PURGE_TXABORT | PURGE_RXCLEAR | PURGE_TXCLEAR);
    ClearCommError(file, nullptr, nullptr);

    // Configurar parámetros de comunicación
    DCB state { .DCBlength = sizeof(DCB) };
    GetCommState(file, &state);

    state.ByteSize = 8;           // 8 bits de datos
    state.BaudRate = baud;        // Velocidad en baudios
    state.Parity = NOPARITY;      // Sin paridad
    state.StopBits = ONESTOPBIT;  // 1 bit de parada

    // Deshabilitar DTR y RTS para evitar reset automático de Arduino
    state.fDtrControl = DTR_CONTROL_DISABLE;
    state.fRtsControl = RTS_CONTROL_DISABLE;
    SetCommState(file, &state);

    // Configurar timeouts (1 segundo para lectura y escritura)
    COMMTIMEOUTS timeouts;
    GetCommTimeouts(file, &timeouts);
    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 1000;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 1000;
    SetCommTimeouts(file, &timeouts);
    return true;
}

// Función para leer datos del puerto serial
int Serial::read(uint8_t* buffer, int size) {
    DWORD bytesRead = 0;

    ReadFile(file, buffer, size, &bytesRead, nullptr);
    return bytesRead;
}

// Función para escribir datos en el puerto serial
int Serial::write(uint8_t* buffer, int size) {
    DWORD bytesWritten = 0;

    WriteFile(file, buffer, size, &bytesWritten, nullptr);
    return bytesWritten;
}

// Cerrar el puerto serial
void Serial::close() {
    CloseHandle(file);
    file = nullptr;
}

// Devuelve la cantidad de bytes disponibles para leer en el puerto serial
size_t Serial::available()
{
    COMSTAT stat;
    ClearCommError(file, nullptr, &stat);
    return stat.cbInQue;  // Bytes disponibles en el buffer de entrada
}
