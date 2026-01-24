#pragma once

class USART {
    bool registro_vacio() {
        return UCSR0A & (1 << UDRE0);
    }

    bool dato_recibido() {
        return UCSR0A & (1 << RXC0);
    }

public:
    uint8_t buffer_escritura[128], buffer_lectura[64];
    // Desactiva optimizaciones
    volatile uint8_t inicio_e = 0, fin_e = 0, inicio_l = 0, fin_l = 0;

    const static uint8_t interrupcion_rx = 1 << RXCIE0;
    const static uint8_t interrupcion_tx = 1 << TXCIE0;
    const static uint8_t interrupcion_registro_vacio = 1 << UDRIE0;

    const static uint8_t modo_asincrono = 0;
    const static uint8_t modo_sincrono = 1 << UMSEL00;
    const static uint8_t modo_maestro_spi = 3 << UMSEL00;

    const static uint8_t paridad_desactivada = 0;
    const static uint8_t paridad_par = 2 << UPM00;
    const static uint8_t paridad_impar = 3 << UPM00;

    const static uint8_t parada_1bit = 0;
    const static uint8_t parada_2bits = 1 << USBS0;

    const static uint8_t caracter_5bits = 0;
    const static uint8_t caracter_6bits = 1 << UCSZ00;
    const static uint8_t caracter_7bits = 2 << UCSZ00;
    const static uint8_t caracter_8bits = 3 << UCSZ00;

    const static uint8_t activar_tx = 1 << TXEN0;
    const static uint8_t activar_rx = 1 << RXEN0;

    const static uint8_t doble_velocidad = 1 << U2X0;

    /**
     * Inicializar UART con baudrate específico
     * 
     * CONFIGURACIÓN PARA SERIALPLOTTER:
     * - Baudrate recomendado: 38400 (para sampling rate 3840 Hz)
     * - Cálculo: sampling_rate × 10 bits/byte = baudrate mínimo
     * - Ejemplo: 3840 Hz × 10 = 38400 baudios
     * 
     * @param baud Velocidad en baudios (ej: 38400, 115200)
     */
    void begin(uint32_t baud) {
        UBRR0 = 16e6 / (8 * baud) - 1;

        UCSR0A = doble_velocidad;
        UCSR0B = interrupcion_rx | interrupcion_registro_vacio | activar_tx | activar_rx;
        UCSR0C = modo_asincrono | paridad_desactivada | parada_1bit | caracter_8bits;
    }

    // Estado

    bool pendiente_lectura(){
        return fin_l != inicio_l;
    }

    bool pendiente_escritura(){
        return fin_e != inicio_e;
    }

    // Devuelve el espacio libre en el buffer de lectura
    uint8_t libre_lectura(){
        uint8_t pendiente;
        if (fin_l >= inicio_l)
            pendiente = fin_l - inicio_l;
        else
            pendiente = sizeof(buffer_lectura) - inicio_l + fin_l;
        return sizeof(buffer_lectura) - pendiente - 1;
    }

    // Devuelve el espacio libre en el buffer de escritura
    uint8_t libre_escritura(){
        uint8_t pendiente;
        if (fin_e >= inicio_e)
            pendiente = fin_e - inicio_e;
        else
            pendiente = sizeof(buffer_escritura) - inicio_e + fin_e;
        return sizeof(buffer_escritura) - pendiente - 1;  // Corregido: era buffer_lectura
    }

    // Intenta escribir y devuelve true si lo logra
    bool escribir(uint8_t byte){
        if (!pendiente_escritura() && registro_vacio()){
            UDR0 = byte;
            return true;
        }

        // Espera hasta que haya espacio
        if (libre_escritura() == 0)
            return false;

        buffer_escritura[fin_e] = byte;
        fin_e = (fin_e + 1) % sizeof(buffer_escritura);
        UCSR0B |= interrupcion_registro_vacio;
        return true;
    }

    // Espera el tiempo necesario para escribir
    void escribir_espera(uint8_t byte){
        if (!pendiente_escritura()){
            while (!registro_vacio());

            UDR0 = byte;
            return;
        }

        // Espera hasta que haya espacio
        while (libre_escritura() == 0);

        buffer_escritura[fin_e] = byte;
        fin_e = (fin_e + 1) % sizeof(buffer_escritura);
        UCSR0B |= interrupcion_registro_vacio;
    }

    //
    uint8_t leer(){
        uint8_t valor = buffer_lectura[inicio_l];
        inicio_l = (inicio_l + 1) % sizeof(buffer_lectura);
        return valor;
    }

    // Esperar hasta que haya algo para leer
    uint8_t leer_espera(){
        if (pendiente_lectura()){
            uint8_t valor = buffer_lectura[inicio_l];
            inicio_l = (inicio_l + 1) % sizeof(buffer_lectura);
            return valor;
        }

        // Deshabilita temporalmente las interrupciones y lee directamente
        UCSR0B &= ~interrupcion_rx;
        while(!dato_recibido());
        uint8_t valor = UDR0;
        UCSR0B |= interrupcion_rx;
        return valor;

        // Implementación simple
        // while (!pendiente_lectura());

        // uint8_t valor = buffer_lectura[inicio_l];
        // inicio_l = (inicio_l + 1) % sizeof(buffer_lectura);
        // return valor;
    }

    void udrie(){
        if (!pendiente_escritura()){
            UCSR0B &= ~interrupcion_registro_vacio;
            return;
        }

        UDR0 = buffer_escritura[inicio_e];
        inicio_e = (inicio_e + 1) % sizeof(buffer_escritura);
    }
};

USART usart;
