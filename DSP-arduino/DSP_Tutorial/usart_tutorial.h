/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                            🔌 USART TUTORIAL COMENTADO 🔌                           ║
 * ║                        Tutorial Educativo de Comunicación Serie                     ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 📚 OBJETIVO EDUCATIVO:
 * Este archivo enseña DESDE CERO cómo funciona la comunicación serie UART/USART
 * en microcontroladores AVR, explicando cada concepto como si nunca hubieras
 * trabajado con comunicación serie antes.
 * 
 * 🎯 QUÉ APRENDERÁS:
 * ✅ ¿Qué es UART? ¿Qué es USART? ¿Cuál es la diferencia?
 * ✅ ¿Cómo funciona la comunicación serie bit por bit?
 * ✅ ¿Qué significa "baud rate" y cómo se calcula?
 * ✅ ¿Qué son los registros UBRR, UCSR0A, UCSR0B, UCSR0C?
 * ✅ ¿Cómo enviar y recibir datos de forma eficiente?
 * ✅ ¿Por qué usar interrupciones vs polling?
 * 
 * 💡 NIVEL: Principiante total → Avanzado
 */

#ifndef USART_TUTORIAL_H
#define USART_TUTORIAL_H

#include <avr/io.h>        // ← Acceso a registros del hardware AVR
#include <avr/interrupt.h> // ← Para manejar interrupciones de USART

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                          📡 ¿QUÉ ES COMUNICACIÓN SERIE? 📡                          ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * COMUNICACIÓN PARALELA vs SERIE:
 * 
 * 🔄 PARALELA (8 cables para 8 bits):
 *    Arduino ----[D0]---- Dispositivo
 *            ----[D1]----
 *            ----[D2]----  ← 8 cables = 8 bits simultáneos
 *            ----[D3]----     Ventaja: ¡Súper rápido!
 *            ----[D4]----     Desventaja: Muchos cables
 *            ----[D5]----
 *            ----[D6]----
 *            ----[D7]----
 *            ----[CLK]---- ← Clock para sincronizar
 *            ----[GND]----
 * 
 * 📡 SERIE (1 cable para todos los bits):
 *    Arduino ----[TX]---- RX ---- Dispositivo
 *            ----[GND]---GND
 *    
 *    Datos: 01001101 se envían como:
 *    Tiempo: |0|1|0|0|1|1|0|1|
 *           bit bit bit bit bit bit bit bit
 *            1   2   3   4   5   6   7   8
 *    
 *    Ventaja: ¡Solo 2 cables!
 *    Desventaja: Más lento que paralelo
 * 
 * 🤔 ¿CUÁNDO USAR CADA UNA?
 * • PARALELA: Cuando necesitas MÁXIMA velocidad (ej: memoria RAM)
 * • SERIE: Cuando necesitas MÍNIMO cables (ej: sensores, PC, Bluetooth)
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                            🔤 UART vs USART: ¿DIFERENCIA? 🔤                        ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 📡 UART = Universal Asynchronous Receiver Transmitter
 *    ¿Qué significa "Asynchronous" (Asíncrono)?
 *    → No necesita cable de clock compartido
 *    → Cada dispositivo tiene su propio reloj interno
 *    → Se sincronizan usando "start bits" y "stop bits"
 * 
 * 📡 USART = Universal Synchronous/Asynchronous Receiver Transmitter  
 *    → Puede trabajar TANTO asíncrono (UART) COMO síncrono
 *    → Modo síncrono: SÍ usa cable de clock compartido
 *    → Modo asíncrono: Igual que UART
 * 
 * 🎯 EN ESTE PROYECTO:
 * Usamos USART en modo ASÍNCRONO (como UART) porque:
 * • Solo tenemos 2 cables: TX y RX
 * • No queremos cable adicional de clock
 * • La PC y Arduino se sincronizan automáticamente
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                          ⚡ ¿QUÉ ES EL BAUD RATE? ⚡                                 ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 🔢 BAUD RATE = "Bits Por Segundo" que se transmiten
 * 
 * EJEMPLOS COMUNES:
 * • 9600 baud   = 9,600 bits/segundo   ← Lento pero muy confiable
 * • 38400 baud  = 38,400 bits/segundo  ← Lo que usamos en este proyecto
 * • 115200 baud = 115,200 bits/segundo ← Rápido para debug
 * 
 * 📊 CÁLCULO DE TIEMPO POR BIT:
 * Tiempo por bit = 1 / baud_rate
 * 
 * Para 38400 baud:
 * Tiempo por bit = 1 / 38400 = 26.04 microsegundos
 * 
 * 📦 ENVÍO DE 1 BYTE (8 bits de datos):
 * |START|D0|D1|D2|D3|D4|D5|D6|D7|STOP|
 * | 26μs|26|26|26|26|26|26|26|26| 26μs|
 * 
 * Total = 10 bits × 26μs = 260μs por byte completo
 * Velocidad real = 1 / 260μs = 3,846 bytes/segundo
 * 
 * 🚀 ¿POR QUÉ 38400 EN NUESTRO PROYECTO?
 * • Frecuencia muestreo = 3840 Hz
 * • Cada muestra = 1 byte
 * • Necesitamos: 3840 bytes/segundo
 * • Con 38400 baud podemos: 3846 bytes/segundo
 * • ¡Perfecto! Con un pequeño margen de seguridad
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                      🔧 REGISTROS USART DEL ATMEGA2560 🔧                           ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * El ATmega2560 tiene CUATRO puertos USART: USART0, USART1, USART2, USART3
 * En este proyecto usamos USART0 (conectado al USB del Arduino)
 * 
 * 📋 REGISTROS PRINCIPALES:
 * 
 * 1️⃣ UBRR0 (USART Baud Rate Register):
 *    • Configura la velocidad de transmisión
 *    • Se calcula con la fórmula: UBRR0 = (F_CPU / (16 * baud)) - 1
 * 
 * 2️⃣ UCSR0A (USART Control and Status Register A):
 *    • Flags de estado: ¿transmisión lista? ¿datos recibidos?
 *    • Control de velocidad: ¿modo doble velocidad?
 * 
 * 3️⃣ UCSR0B (USART Control and Status Register B):  
 *    • Habilita/deshabilita: transmisión, recepción, interrupciones
 *    • Control de tamaño de datos (bit 9 si usas 9 bits)
 * 
 * 4️⃣ UCSR0C (USART Control and Status Register C):
 *    • Configura formato: ¿8 bits o 9? ¿paridad? ¿stop bits?
 *    • Modo: asíncrono vs síncrono
 * 
 * 5️⃣ UDR0 (USART Data Register):
 *    • ENVIAR: Escribes aquí el byte a transmitir
 *    • RECIBIR: Lees aquí el byte que llegó
 */

// 🎯 CONFIGURACIÓN DE BAUD RATE PARA NUESTRO PROYECTO
#define BAUD_RATE 38400UL

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                        🧮 CÁLCULO MATEMÁTICO DEL BAUD RATE 🧮                       ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 📐 FÓRMULA OFICIAL (modo asíncrono, velocidad normal):
 * UBRR0 = (F_CPU / (16 * BAUD)) - 1
 * 
 * 🔢 SUSTITUYENDO NUESTROS VALORES:
 * F_CPU = 16,000,000 Hz (Arduino Mega 2560 a 16MHz)
 * BAUD = 38,400
 * 
 * UBRR0 = (16,000,000 / (16 * 38,400)) - 1
 * UBRR0 = (16,000,000 / 614,400) - 1  
 * UBRR0 = 26.04 - 1
 * UBRR0 = 25.04 → 25 (redondeamos a entero)
 * 
 * 🎯 VERIFICACIÓN (¿qué baud rate real obtenemos?):
 * BAUD_real = F_CPU / (16 * (UBRR0 + 1))
 * BAUD_real = 16,000,000 / (16 * (25 + 1))
 * BAUD_real = 16,000,000 / (16 * 26)  
 * BAUD_real = 16,000,000 / 416
 * BAUD_real = 38,461.54 baud
 * 
 * 📊 ERROR PORCENTUAL:
 * Error = |38,461.54 - 38,400| / 38,400 × 100%
 * Error = 61.54 / 38,400 × 100% = 0.16%
 * 
 * ✅ ¡Excelente! Error menor al 1% es perfectamente aceptable.
 */
#define UBRR_VALUE ((F_CPU / (16UL * BAUD_RATE)) - 1)

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                          🚀 CLASE USART TUTORIAL 🚀                                 ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * Esta clase encapsula TODA la funcionalidad USART de una forma educativa.
 * Cada método está diseñado para enseñar conceptos específicos.
 */

class UsartTutorial {
public:
    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                        🏗️ CONSTRUCTOR: INICIALIZACIÓN 🏗️                        ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     * 
     * El constructor se ejecuta UNA SOLA VEZ cuando creamos el objeto.
     * Su trabajo es configurar TODOS los registros USART para que funcionen correctamente.
     */
    UsartTutorial() {
        // 🔧 PASO 1: Configurar la velocidad (baud rate)
        configurar_baud_rate();
        
        // 🔧 PASO 2: Configurar el formato de datos
        configurar_formato_datos();
        
        // 🔧 PASO 3: Habilitar transmisión y recepción
        habilitar_transceiver();
        
        // 🔧 PASO 4: Configurar interrupciones (opcional)
        configurar_interrupciones();
    }

private:
    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                        1️⃣ CONFIGURAR BAUD RATE 1️⃣                               ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     * 
     * Esta función programa el registro UBRR0 con el valor calculado para conseguir
     * exactamente 38400 baud de velocidad de transmisión.
     */
    void configurar_baud_rate() {
        // 📊 El registro UBRR0 es de 16 bits, pero está dividido en dos bytes:
        // UBRR0H = byte alto (bits 15-8)
        // UBRR0L = byte bajo (bits 7-0)
        
        // 🔢 Nuestro valor calculado es 25 (decimal) = 0x0019 (hexadecimal)
        // Como 25 < 256, el byte alto será 0 y el bajo será 25
        
        UBRR0H = (uint8_t)(UBRR_VALUE >> 8);  // ← Desplaza 8 bits a la derecha = byte alto
        UBRR0L = (uint8_t)(UBRR_VALUE);       // ← Toma solo los 8 bits bajos
        
        /*
         * 🤔 ¿POR QUÉ SEPARAR EN DOS BYTES?
         * • Los registros del AVR son de 8 bits cada uno
         * • Para valores mayores a 255, necesitamos 2 registros
         * • UBRR permite valores hasta 4095 (12 bits útiles)
         * • Esto da un rango de baud rates desde muy lento hasta muy rápido
         */
    }
    
    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                      2️⃣ CONFIGURAR FORMATO DE DATOS 2️⃣                         ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     * 
     * Esta función configura CÓMO se estructura cada byte transmitido:
     * • ¿Cuántos bits de datos? (5, 6, 7, 8 o 9)
     * • ¿Paridad? (ninguna, par, impar)  
     * • ¿Cuántos stop bits? (1 o 2)
     */
    void configurar_formato_datos() {
        // 🎯 QUEREMOS: 8 bits de datos, sin paridad, 1 stop bit
        // Esto es el formato más común llamado "8N1"
        
        /*
         * 📋 REGISTRO UCSR0C (USART Control and Status Register C):
         * 
         * Bit 7    | Bit 6    | Bit 5-4 | Bit 3   | Bit 2-1 | Bit 1-0
         * UMSEL01  | UMSEL00  | UPM01-0 | USBS0   | UCSZ01-0| 
         * ---------|----------|---------|---------|---------|
         * 0        | 0        | 00      | 0       | 11      |
         * 
         * EXPLICACIÓN DETALLADA:
         * 
         * 🔹 UMSEL01:UMSEL00 = 00 → Modo asíncrono (UART)
         *    • 00 = Asíncrono  ← Lo que queremos
         *    • 01 = Síncrono
         *    • 10 = Reservado
         *    • 11 = SPI Master
         * 
         * 🔹 UPM01:UPM00 = 00 → Sin paridad
         *    • 00 = Sin paridad ← Lo que queremos  
         *    • 01 = Reservado
         *    • 10 = Paridad par
         *    • 11 = Paridad impar
         * 
         * 🔹 USBS0 = 0 → 1 stop bit
         *    • 0 = 1 stop bit ← Lo que queremos
         *    • 1 = 2 stop bits
         * 
         * 🔹 UCSZ01:UCSZ00 = 11 → 8 bits de datos
         *    • 00 = 5 bits
         *    • 01 = 6 bits  
         *    • 10 = 7 bits
         *    • 11 = 8 bits ← Lo que queremos
         */
        
        UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  // ← Solo activamos los bits para 8 datos
        
        /*
         * 🤓 EXPLICACIÓN DEL CÓDIGO:
         * (1 << UCSZ01) → Desplaza 1 hacia la izquierda UCSZ01 posiciones
         * (1 << UCSZ00) → Desplaza 1 hacia la izquierda UCSZ00 posiciones  
         * El | (OR) combina ambos bits
         * 
         * Si UCSZ01=2 y UCSZ00=1:
         * (1 << 2) = 0b00000100
         * (1 << 1) = 0b00000010  
         * OR result = 0b00000110 → Bits 1 y 2 activados = 8 bits de datos
         */
    }
    
    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                    3️⃣ HABILITAR TRANSMISIÓN Y RECEPCIÓN 3️⃣                     ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     * 
     * Esta función activa los circuitos de transmisión (TX) y recepción (RX)
     * del hardware USART. Sin esto, ¡nada funcionará!
     */
    void habilitar_transceiver() {
        /*
         * 📋 REGISTRO UCSR0B (USART Control and Status Register B):
         * 
         * Bit 7   | Bit 6   | Bit 5   | Bit 4   | Bit 3   | Bit 2   | Bit 1   | Bit 0
         * RXCIE0  | TXCIE0  | UDRIE0  | RXEN0   | TXEN0   | UCSZ02  | RXB80   | TXB80
         * --------|---------|---------|---------|---------|---------|---------|-------
         * 0       | 0       | 0       | 1       | 1       | 0       | X       | X
         * 
         * EXPLICACIÓN DETALLADA:
         * 
         * 🔹 RXCIE0 = 0 → Interrupción RX desactivada (por ahora)
         * 🔹 TXCIE0 = 0 → Interrupción TX completo desactivada  
         * 🔹 UDRIE0 = 0 → Interrupción registro vacío desactivada
         * 🔹 RXEN0 = 1 → ACTIVAR receptor ← ¡CRÍTICO!
         * 🔹 TXEN0 = 1 → ACTIVAR transmisor ← ¡CRÍTICO!
         * 🔹 UCSZ02 = 0 → Parte del tamaño de datos (junto con UCSR0C)
         * 🔹 RXB80/TXB80 = Solo para modo 9 bits (no usamos)
         */
        
        UCSR0B = (1 << RXEN0) | (1 << TXEN0);  // ← Activar RX y TX
        
        /*
         * 🚨 ¡IMPORTANTE! ¿QUÉ PASA SI NO HACES ESTO?
         * • Sin RXEN0=1: No puedes recibir datos de la PC
         * • Sin TXEN0=1: No puedes enviar datos a la PC  
         * • ¡El hardware está físicamente desconectado!
         * 
         * 💡 ANALOGÍA: Es como tener un teléfono pero sin activar la línea.
         * El hardware está ahí, pero "no hay tono".
         */
    }
    
    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                      4️⃣ CONFIGURAR INTERRUPCIONES 4️⃣                           ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     * 
     * Las interrupciones permiten que el USART nos "avise" automáticamente cuando:
     * • Llega un nuevo byte (RX Complete)
     * • Se terminó de enviar un byte (TX Complete)  
     * • El buffer de transmisión está vacío (Data Register Empty)
     */
    void configurar_interrupciones() {
        // 🎯 Por ahora, NO activamos interrupciones
        // Las habilitaremos más adelante cuando tengamos los handlers listos
        
        /*
         * 🤔 ¿INTERRUPCIONES vs POLLING?
         * 
         * 📊 POLLING (lo que haremos inicialmente):
         * while(true) {
         *     if (datos_recibidos()) {
         *         procesar_datos();
         *     }
         *     hacer_otras_cosas();
         * }
         * 
         * Ventaja: ✅ Simple de entender y programar
         * Desventaja: ❌ Perdemos tiempo verificando constantemente
         * 
         * ⚡ INTERRUPCIONES (versión avanzada):
         * void ISR(USART_RX_vect) {
         *     procesar_datos_automaticamente();
         * }
         * 
         * void main() {
         *     hacer_otras_cosas_sin_preocuparse();
         * }
         * 
         * Ventaja: ✅ Respuesta inmediata, no perdemos tiempo
         * Desventaja: ❌ Más complejo, fácil cometer errores
         * 
         * 🎯 PARA ESTE TUTORIAL:
         * Empezamos con polling para entender los conceptos básicos,
         * luego evolucionamos a interrupciones para máxima eficiencia.
         */
        
        // Desactivar todas las interrupciones USART por ahora
        UCSR0B &= ~((1 << RXCIE0) | (1 << TXCIE0) | (1 << UDRIE0));
    }

public:
    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                        📤 ENVIAR UN BYTE DE DATOS 📤                            ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     * 
     * Esta función envía un solo byte (8 bits) a través del puerto serie.
     * Es la función más básica y fundamental de la comunicación.
     */
    void enviar_byte(uint8_t dato) {
        /*
         * 🚦 PASO 1: Esperar a que el transmisor esté listo
         * 
         * El hardware USART tiene un buffer interno donde almacena el byte
         * que está transmitiendo actualmente. Si intentamos enviar un nuevo
         * byte mientras el anterior aún se está transmitiendo, ¡se perderá!
         * 
         * Tenemos que esperar a que el flag UDRE0 (USART Data Register Empty)
         * se ponga en 1, lo que significa "buffer libre, puedes enviar".
         */
        while (!(UCSR0A & (1 << UDRE0))) {
            // ⏳ Esperamos activamente (busy waiting)
            // Esta línea se ejecuta miles de veces hasta que el buffer esté libre
        }
        
        /*
         * 🔍 ANÁLISIS DETALLADO DEL WHILE:
         * 
         * UCSR0A = registro de estado actual
         * (1 << UDRE0) = máscara para aislar solo el bit UDRE0
         * & = operación AND bit a bit
         * ! = negación lógica
         * 
         * Ejemplo paso a paso:
         * 1. UCSR0A = 0b10100001 (UDRE0=bit 5=0, buffer ocupado)
         * 2. (1 << UDRE0) = 0b00100000 (máscara para bit 5)  
         * 3. UCSR0A & máscara = 0b10100001 & 0b00100000 = 0b00000000 = 0
         * 4. !(0) = 1 (verdadero) → seguir esperando
         * 
         * Cuando el buffer se libera:
         * 1. UCSR0A = 0b10100001 → 0b10120001 (UDRE0=1)
         * 2. UCSR0A & máscara = 0b10120001 & 0b00100000 = 0b00100000 ≠ 0  
         * 3. !(algo≠0) = 0 (falso) → salir del while
         */
        
        /*
         * 🚀 PASO 2: Escribir el dato al registro UDR0
         * 
         * Una vez que sabemos que el buffer está libre, podemos escribir
         * nuestro byte al registro UDR0. El hardware automáticamente:
         * 1. Toma el byte del registro
         * 2. Lo descompone en bits individuales  
         * 3. Los envía uno por uno a la velocidad configurada (38400 baud)
         * 4. Añade automáticamente start bit, stop bit, etc.
         */
        UDR0 = dato;
        
        /*
         * 🎭 LO QUE PASA "DETRÁS DEL TELÓN":
         * 
         * En el momento que escribimos UDR0 = dato, el hardware:
         * 
         * 1. 🏁 Añade START BIT (siempre 0):
         *    TX pin: HIGH → LOW (indica "empieza transmisión")
         * 
         * 2. 📊 Envía 8 bits de datos (LSB primero):
         *    Si dato = 0b10110001 = 177 decimal
         *    Envía: 1,0,0,0,1,1,0,1 (bit 0 primero, bit 7 último)
         * 
         * 3. 🛑 Añade STOP BIT (siempre 1):  
         *    TX pin: LOW → HIGH (indica "termina transmisión")
         * 
         * 4. 📏 Todo esto toma exactamente 260μs @ 38400 baud
         * 
         * 5. 🚩 Cuando termina, pone UDRE0=1 para avisar "listo para próximo byte"
         */
    }
    
    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                         📥 RECIBIR UN BYTE DE DATOS 📥                          ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     * 
     * Esta función recibe un byte del puerto serie, pero solo SI hay datos
     * disponibles. Si no hay datos, devuelve 0.
     */
    uint8_t recibir_byte() {
        /*
         * 🚦 PASO 1: Verificar si hay datos disponibles
         * 
         * El flag RXC0 (RX Complete) se pone en 1 automáticamente cuando
         * el hardware ha recibido un byte completo y lo ha guardado en UDR0.
         */
        if (UCSR0A & (1 << RXC0)) {
            /*
             * 🎊 ¡HAY DATOS! Leer del registro UDR0
             * 
             * Al leer UDR0, automáticamente:
             * • Obtenemos el byte recibido
             * • Se limpia el flag RXC0 (queda listo para el próximo byte)
             */
            return UDR0;
        } else {
            /*
             * 😔 No hay datos disponibles
             * Devolvemos 0 como indicador de "sin datos"
             */
            return 0;
        }
        
        /*
         * 🎭 LO QUE PASA "DETRÁS DEL TELÓN" EN LA RECEPCIÓN:
         * 
         * 1. 🎧 Hardware monitorea constantemente el pin RX
         * 
         * 2. 🏁 Detecta START BIT (transición HIGH→LOW):
         *    "¡Aha! Viene un byte"
         * 
         * 3. ⏰ Espera 1.5 períodos de bit para centrarse en la señal
         *    (esto evita errores por ruido en la transición)
         * 
         * 4. 📊 Lee 8 bits de datos (uno cada 26μs @ 38400 baud):
         *    Bit 0, Bit 1, ..., Bit 7
         * 
         * 5. 🛑 Verifica STOP BIT (debe ser HIGH):
         *    Si no está, marca error de "framing"
         * 
         * 6. ✅ Si todo OK, guarda el byte en UDR0 y activa RXC0
         * 
         * 7. 🔄 Vuelve a monitorear para el siguiente byte
         */
    }
    
    /*
     * ╔══════════════════════════════════════════════════════════════════════════════════╗
     * ║                      📈 FUNCIONES DE DIAGNÓSTICO 📈                             ║
     * ╚══════════════════════════════════════════════════════════════════════════════════╝
     * 
     * Estas funciones nos ayudan a entender el estado actual del USART
     * y diagnosticar problemas de comunicación.
     */
    
    // 🔍 ¿Hay datos esperando ser leídos?
    bool hay_datos_disponibles() {
        return (UCSR0A & (1 << RXC0)) != 0;
        
        /*
         * 💡 USO TÍPICO:
         * if (usart.hay_datos_disponibles()) {
         *     uint8_t dato = usart.recibir_byte();
         *     procesar(dato);
         * }
         */
    }
    
    // 🔍 ¿El transmisor está listo para enviar?
    bool transmisor_listo() {
        return (UCSR0A & (1 << UDRE0)) != 0;
        
        /*
         * 💡 USO TÍPICO:
         * if (usart.transmisor_listo()) {
         *     usart.enviar_byte(mi_dato);
         * } else {
         *     // Hacer otras cosas mientras esperamos
         * }
         */
    }
    
    // 🔍 ¿Se terminó de transmitir completamente el último byte?
    bool transmision_completa() {
        return (UCSR0A & (1 << TXC0)) != 0;
        
        /*
         * 🤔 ¿DIFERENCIA ENTRE UDRE0 y TXC0?
         * 
         * UDRE0 = "Data Register Empty"
         * → Buffer interno libre, puedes enviar el PRÓXIMO byte
         * → Pero el byte anterior podría aún estar transmitiéndose físicamente
         * 
         * TXC0 = "Transmission Complete"  
         * → El byte salió COMPLETAMENTE por el pin TX
         * → Transmisión 100% terminada incluyendo stop bit
         * 
         * 📊 CRONOLOGÍA:
         * enviar_byte(0x55)
         * |
         * ├─ Inmediatamente: UDRE0 = 0 (buffer ocupado)
         * ├─ ~1μs después: UDRE0 = 1 (puede enviar próximo)
         * └─ ~260μs después: TXC0 = 1 (transmisión completamente terminada)
         */
    }
    
    // 📊 Obtener estadísticas del puerto serie
    void obtener_estadisticas() {
        /*
         * Esta función es puramente educativa para entender
         * el estado actual de todos los registros USART
         */
        
        // 📋 Leer todos los registros de estado
        uint8_t ucsr0a = UCSR0A;
        uint8_t ucsr0b = UCSR0B;  
        uint8_t ucsr0c = UCSR0C;
        uint16_t ubrr = ((uint16_t)UBRR0H << 8) | UBRR0L;
        
        // 🎯 Los datos se pueden enviar via debug o guardar en variables
        // para análisis posterior (implementación específica depende del uso)
    }
};

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                            🎓 CONCEPTOS AVANZADOS 🎓                               ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 🔥 MODO DOBLE VELOCIDAD (U2X0):
 * • Cambia el divisor de 16 a 8 en la fórmula del baud rate
 * • Permite baud rates más altos o mejor precisión
 * • Formula: UBRR = (F_CPU / (8 * BAUD)) - 1
 * • Se activa con: UCSR0A |= (1 << U2X0);
 * 
 * 📡 MODO SÍNCRONO:
 * • Requiere cable adicional de clock (XCK0)
 * • Más rápido y confiable que asíncrono  
 * • Usado en SPI y protocolos especializados
 * • Se activa con bits UMSEL en UCSR0C
 * 
 * 🛡️ DETECCIÓN DE ERRORES:
 * • Frame Error (FE0): Stop bit incorrecto
 * • Data OverRun (DOR0): Perdimos datos por lentitud
 * • Parity Error (UPE0): Error de paridad (si está activada)
 * • Se leen desde UCSR0A junto con los datos
 * 
 * ⚡ INTERRUPCIONES AVANZADAS:
 * • RX_vect: Se ejecuta automáticamente al recibir datos
 * • TX_vect: Se ejecuta al completar transmisión  
 * • UDRE_vect: Se ejecuta cuando buffer está vacío
 * • Permite comunicación 100% asíncrona sin polling
 * 
 * 🎯 BUFFERS CIRCULARES:
 * • Para manejar múltiples bytes sin perder datos
 * • Especialmente útil con interrupciones
 * • Buffer de entrada y salida independientes
 * • Manejo de overflow y underflow
 */

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                        🚀 EJEMPLO DE USO PRÁCTICO 🚀                               ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * // Crear objeto USART (se inicializa automáticamente)
 * UsartTutorial usart;
 * 
 * void setup() {
 *     // ¡Ya está listo para usar!
 * }
 * 
 * void loop() {
 *     // Enviar datos a la PC
 *     usart.enviar_byte(0x42);  // Envía 'B' en ASCII
 *     
 *     // Recibir datos de la PC  
 *     if (usart.hay_datos_disponibles()) {
 *         uint8_t comando = usart.recibir_byte();
 *         procesar_comando(comando);
 *     }
 *     
 *     delay(10);  // No saturar la comunicación
 * }
 */

#endif // USART_TUTORIAL_H

/*
 * ╔══════════════════════════════════════════════════════════════════════════════════════╗
 * ║                              🎉 ¡FELICITACIONES! 🎉                                ║
 * ║                                                                                      ║
 * ║  Has aprendido los fundamentos de la comunicación serie USART desde cero.          ║
 * ║  Ahora entiendes cómo funciona cada bit, registro y timing del sistema.            ║
 * ║                                                                                      ║
 * ║  🎯 Próximos pasos sugeridos:                                                       ║
 * ║  • Implementar buffers circulares                                                   ║
 * ║  • Añadir manejo de errores robusto                                                 ║
 * ║  • Migrar a sistema basado en interrupciones                                        ║
 * ║  • Experimentar con diferentes baud rates                                           ║
 * ║  • Crear protocolos de comunicación de más alto nivel                              ║
 * ║                                                                                      ║
 * ║                          ¡Sigue explorando! 🚀                                     ║
 * ╚══════════════════════════════════════════════════════════════════════════════════════╝
 */