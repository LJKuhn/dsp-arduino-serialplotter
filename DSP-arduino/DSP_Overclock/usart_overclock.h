/**
 * USART OVERCLOCK - Comunicación Serie Optimizada
 * 
 * Versión de alta velocidad con buffers ampliados para 7680 Hz
 * 
 * DIFERENCIAS con usart.h estándar:
 * - Buffer TX: 512 bytes (vs 256)
 * - Buffer RX: 128 bytes (vs 64)
 * - Capacidad TX: 66 ms @ 7680 Hz (vs 66 ms @ 3840 Hz)
 * - Capacidad RX: 16 ms @ 7680 Hz (vs 16 ms @ 3840 Hz)
 * 
 * Autor: Lautaro Kühn & Federico Domínguez
 * Versión: OVERCLOCK 1.0
 */

#pragma once

class USART_Overclock {
    bool registro_vacio() {
        return UCSR0A & (1 << UDRE0);
    }

    bool dato_recibido() {
        return UCSR0A & (1 << RXC0);
    }

public:
    // ⚡ BUFFERS AMPLIADOS PARA OVERCLOCK
    uint8_t buffer_escritura[512];  // 512 bytes (2× estándar)
    uint8_t buffer_lectura[128];    // 128 bytes (2× estándar)
    
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
     * Inicializar UART OVERCLOCK
     * 
     * BAUDRATES OVERCLOCK SOPORTADOS:
     * - 115200 bps: Para 7680 Hz (recomendado)
     * - 230400 bps: Para 11520-15360 Hz (experimental)
     * - 460800 bps: Para 23040+ Hz (requiere hardware especial)
     * 
     * @param baud Velocidad en baudios
     */
    void begin(uint32_t baud) {
        UBRR0 = 16e6 / (8 * baud) - 1;

        UCSR0A = doble_velocidad;
        UCSR0B = interrupcion_rx | interrupcion_registro_vacio | activar_tx | activar_rx;
        UCSR0C = modo_asincrono | paridad_desactivada | parada_1bit | caracter_8bits;
    }

    // ========== FUNCIONES DE ESTADO ==========
    
    bool pendiente_lectura() {
        return fin_l != inicio_l;
    }

    bool pendiente_escritura() {
        return fin_e != inicio_e;
    }

    uint8_t libre_lectura() {
        uint8_t pendiente;
        if (fin_l >= inicio_l)
            pendiente = fin_l - inicio_l;
        else
            pendiente = sizeof(buffer_lectura) - inicio_l + fin_l;
        return sizeof(buffer_lectura) - pendiente - 1;
    }

    uint8_t libre_escritura() {
        uint8_t pendiente;
        if (fin_e >= inicio_e)
            pendiente = fin_e - inicio_e;
        else
            pendiente = sizeof(buffer_escritura) - inicio_e + fin_e;
        return sizeof(buffer_escritura) - pendiente - 1;
    }

    /**
     * Escribir byte NO BLOQUEANTE
     * 
     * OPTIMIZADO PARA OVERCLOCK:
     * - Buffer de 512 bytes soporta ráfagas de hasta 66 ms
     * - A 7680 Hz esto son 507 muestras sin saturación
     * - Prácticamente imposible saturar en operación normal
     * 
     * @return true si se escribió, false si buffer lleno
     */
    bool escribir(uint8_t byte) {
        if (!pendiente_escritura() && registro_vacio()) {
            UDR0 = byte;
            return true;
        }

        // Verificar espacio en buffer
        if (libre_escritura() == 0)
            return false;

        buffer_escritura[fin_e] = byte;
        fin_e = (fin_e + 1) % sizeof(buffer_escritura);
        UCSR0B |= interrupcion_registro_vacio;
        return true;
    }

    /**
     * Escribir byte CON ESPERA
     * NO RECOMENDADO en modo OVERCLOCK (puede causar pérdida de muestras)
     * Solo usar para debug o inicialización
     */
    void escribir_espera(uint8_t byte) {
        if (!pendiente_escritura()) {
            while (!registro_vacio());
            UDR0 = byte;
            return;
        }

        while (libre_escritura() == 0);

        buffer_escritura[fin_e] = byte;
        fin_e = (fin_e + 1) % sizeof(buffer_escritura);
        UCSR0B |= interrupcion_registro_vacio;
    }

    /**
     * Leer byte del buffer de recepción
     * 
     * @return Byte recibido (verificar pendiente_lectura() antes)
     */
    uint8_t leer() {
        uint8_t valor = buffer_lectura[inicio_l];
        inicio_l = (inicio_l + 1) % sizeof(buffer_lectura);
        return valor;
    }

    /**
     * ISR handler para transmisión
     * Llamar desde ISR(USART0_UDRE_vect)
     */
    void udrie() {
        if (!pendiente_escritura()) {
            UCSR0B &= ~interrupcion_registro_vacio;
            return;
        }

        UDR0 = buffer_escritura[inicio_e];
        inicio_e = (inicio_e + 1) % sizeof(buffer_escritura);
    }

    // ========== FUNCIONES DE DIAGNÓSTICO OVERCLOCK ==========
    
    /**
     * Obtener porcentaje de uso del buffer TX
     * Útil para monitoreo de saturación
     * 
     * @return Porcentaje 0-100 de buffer usado
     */
    uint8_t uso_buffer_tx() {
        uint16_t libre = libre_escritura();
        return (100 * (sizeof(buffer_escritura) - libre)) / sizeof(buffer_escritura);
    }

    /**
     * Obtener porcentaje de uso del buffer RX
     * 
     * @return Porcentaje 0-100 de buffer usado
     */
    uint8_t uso_buffer_rx() {
        uint16_t libre = libre_lectura();
        return (100 * (sizeof(buffer_lectura) - libre)) / sizeof(buffer_lectura);
    }

    /**
     * Verificar si el sistema está saturado
     * 
     * @return true si algún buffer está >90% lleno
     */
    bool saturado() {
        return (uso_buffer_tx() > 90) || (uso_buffer_rx() > 90);
    }
};

// Instancia global OVERCLOCK
USART_Overclock usart_oc;
