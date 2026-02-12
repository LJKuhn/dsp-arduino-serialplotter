/**
 * ============================================================================
 * TUTORIAL COMPLETO: DSP (Procesamiento Digital de Señales) - Arduino Mega 2560
 * ============================================================================
 * 
 * Este archivo es una versión educativa EXTREMADAMENTE comentada del proyecto DSP.
 * Cada línea, función y registro está explicado en detalle para entender
 * completamente cómo funciona un sistema de procesamiento de señales.
 * 
 * OBJETIVO: Sistema que lee señales analógicas, las envía a una PC para procesamiento
 * con filtros digitales, y genera una salida analógica con la señal filtrada.
 * 
 * HARDWARE NECESARIO:
 * - Arduino Mega 2560 (NO funciona en Arduino Uno por falta de PORTA)
 * - Circuito DAC R2R de 8 bits conectado a pines 22-29
 * - Señal de entrada en pin A1 (después de acondicionador de señal)
 * - Computadora con SerialPlotter corriendo
 * 
 * FLUJO DEL SISTEMA:
 * Señal Analógica → ADC → Arduino → UART → PC → Filtros → UART → Arduino → DAC → Señal Filtrada
 */

// ============================================================================
// INCLUSIÓN DE BIBLIOTECAS Y HEADERS
// ============================================================================

// ¿QUÉ SON LOS #include?
// Los #include le dicen al compilador "incluye el código de otro archivo aquí"
// Es como copiar y pegar el contenido de otro archivo en este punto

#include "adc.h"       // Contiene la clase para controlar el ADC (Analog-to-Digital Converter)
#include "timer1.h"    // Contiene la clase para configurar el Timer1 del microcontrolador
#include "tablas.h"    // Contiene arrays con formas de onda precalculadas (seno, triangular, etc.)
#include "usart.h"     // Contiene la clase para comunicación serie UART
#include <avr/io.h>    // Biblioteca estándar de AVR para acceso a registros del hardware
#include <avr/interrupt.h>  // Biblioteca para manejar interrupciones del microcontrolador

// ============================================================================
// CREACIÓN DE OBJETOS (INSTANCIAS DE CLASES)
// ============================================================================

// ¿QUÉ ES UNA INSTANCIA/OBJETO?
// Es como crear una "copia" de una clase que puedes usar en tu programa
// La clase es el "molde", el objeto es la "galleta" hecha con ese molde

ADCController adc;           // Objeto para controlar el convertidor analógico-digital
                            // Le permitirá leer voltajes del mundo real y convertirlos a números

Timer1 timer1(3840.0);      // Objeto para controlar el Timer1 a una frecuencia de 3840 Hz
                            // ¿Por qué 3840 Hz? Porque 3840 x 10 bits = 38400 baudios (velocidad serie)
                            // Esto sincroniza perfectamente el muestreo con la comunicación

// ============================================================================
// FUNCIÓN WRITE: CONVERTIR NÚMERO DIGITAL A VOLTAJE ANALÓGICO
// ============================================================================

/**
 * ¿QUÉ HACE ESTA FUNCIÓN?
 * Convierte un número digital (0-255) en un voltaje analógico (0V-5V)
 * usando un circuito llamado DAC R2R (Digital-to-Analog Converter)
 * 
 * ¿CÓMO FUNCIONA UN DAC R2R?
 * Es una red de resistencias que convierte bits digitales en voltajes:
 * - Si un bit es 1: contribuye con un voltaje proporcional a su peso
 * - Si un bit es 0: contribuye con 0V
 * - El voltaje total es la suma ponderada de todos los bits
 * 
 * EJEMPLO:
 * Si valor = 128 (binario: 10000000), solo el bit 7 está activo
 * El voltaje de salida será aproximadamente 2.5V (la mitad de 5V)
 */
void write(uint8_t valor) {
    // ¿QUÉ ES uint8_t?
    // Un tipo de dato que representa un número entero sin signo de 8 bits
    // Rango: 0 a 255 (2^8 - 1)
    
    // ¿QUÉ ES PORTA?
    // PORTA es un registro del microcontrolador ATmega2560 que controla
    // directamente los pines físicos PA0-PA7 (pines 22-29 en Arduino Mega)
    
    // ¿POR QUÉ UNA SOLA LÍNEA?
    // En Arduino Mega, PORTA controla 8 pines simultáneamente
    // Esto es MUY eficiente porque:
    // 1. Escritura ATÓMICA: todos los bits cambian al mismo tiempo
    // 2. Sin JITTER: no hay desfase temporal entre bits
    // 3. Una sola instrucción de CPU en lugar de 8 operaciones separadas
    
    PORTA = valor;  // ¡Escritura mágica! 8 bits → 8 pines → voltaje analógico
}

// ============================================================================
// VARIABLES GLOBALES DEL SISTEMA
// ============================================================================

// ¿QUÉ SON LAS VARIABLES GLOBALES?
// Variables que pueden ser accedidas desde cualquier función del programa
// Se almacenan en la RAM del microcontrolador

uint8_t counter = 0;        // Contador para recorrer las tablas de formas de onda
uint8_t valor = 0;          // Valor actual que se escribirá al DAC (0-255)

// ¿QUÉ SIGNIFICA "volatile"?
// Le dice al compilador "esta variable puede cambiar inesperadamente"
// Es necesario cuando una variable es modificada por una interrupción
// Sin "volatile", el compilador podría optimizar incorrectamente el código
volatile bool beat = false; // Bandera que indica cuándo el Timer1 generó una interrupción

// ============================================================================
// INTERRUPCIÓN DEL TIMER1: EL CORAZÓN DEL SISTEMA
// ============================================================================

/**
 * ¿QUÉ ES UNA INTERRUPCIÓN?
 * Es como una "alarma" del hardware que detiene temporalmente el programa
 * principal para ejecutar una función específica (ISR = Interrupt Service Routine)
 * 
 * ¿POR QUÉ USAR INTERRUPCIONES?
 * - PRECISIÓN: Se ejecutan exactamente cada 1/3840 segundos
 * - INDEPENDENCIA: No dependen del tiempo que tome el programa principal
 * - TIEMPO REAL: Garantizan que el DAC se actualice a tiempo constante
 * 
 * TIMER1_COMPA_vect:
 * Es el "nombre especial" de la interrupción del Timer1 en modo Compare Match
 * El compilador automaticamente conecta esta función con la interrupción del hardware
 */
ISR(TIMER1_COMPA_vect) {
    // ¿QUÉ HACE ISR()?
    // ISR significa "Interrupt Service Routine"
    // Esta función se ejecuta automáticamente cada 1/3840 segundos
    
    // PASO 1: Escribir el valor actual al DAC
    // Esto convierte el número digital en voltaje analógico
    write(valor);  // valor viene del loop principal (datos de la PC o ADC)
    
    // PASO 2: Activar bandera para el programa principal
    // Le dice al loop() "ocurrió una nueva muestra, haz algo"
    beat = true;   
    
    // ¿POR QUÉ ES TAN CORTA ESTA FUNCIÓN?
    // Las ISR deben ser MUY rápidas porque:
    // 1. Bloquean otras interrupciones mientras se ejecutan
    // 2. Si demoran mucho, el sistema se vuelve inestable
    // 3. La frecuencia es alta (3840 Hz = cada 260 microsegundos)
}

// ============================================================================
// CONFIGURACIÓN INICIAL DEL SISTEMA (SE EJECUTA UNA VEZ)
// ============================================================================

/**
 * ¿QUÉ ES LA FUNCIÓN setup()?
 * Es una función especial de Arduino que se ejecuta UNA SOLA VEZ
 * cuando el microcontrolador se enciende o se reinicia.
 * Aquí configuramos todos los componentes del hardware.
 */
void setup() {
    // ============================================================================
    // CONFIGURACIÓN DEL PUERTO SERIE PARA DEPURACIÓN
    // ============================================================================
    
    // ¿QUÉ ES Serial?
    // Es el puerto de comunicación USB que conecta Arduino con la computadora
    // Lo usamos para ver mensajes de depuración en el Monitor Serie del IDE
    
    Serial.begin(9600);     // Inicializar a 9600 baudios (diferente del UART principal)
    Serial.println("=== SISTEMA DSP INICIANDO ===");
    Serial.println("Hardware: Arduino Mega 2560");
    Serial.println("Frecuencia: 3840 Hz");
    Serial.println("Comunicacion: 38400 baudios");
    
    // ============================================================================
    // CONFIGURACIÓN DEL ADC (CONVERTIDOR ANALÓGICO-DIGITAL)
    // ============================================================================
    
    // ¿QUÉ ES EL ADC?
    // Un circuito que convierte voltajes analógicos (mundo real) en números digitales
    // Arduino Mega tiene ADC de 10 bits, pero usamos solo 8 bits (0-255)
    
    Serial.println("Configurando ADC...");
    adc.begin(1);           // Inicializar ADC en pin A1
                           // Pin A1 = Canal 1 del ADC
                           // Después del acondicionador de señal, la entrada debe estar en 0V-5V
    
    // ============================================================================
    // CONFIGURACIÓN DE PINES PARA EL DAC
    // ============================================================================
    
    // ¿QUÉ ES DDRA?
    // Es el "Data Direction Register" para el puerto A
    // Controla si cada pin es entrada (0) o salida (1)
    
    Serial.println("Configurando pines DAC (22-29)...");
    DDRA = 0xFF;            // 0xFF en binario = 11111111
                           // Esto configura los 8 pines del PORTA como SALIDAS
                           // Pines PA0-PA7 = Arduino pines 22-29
    
    // VERIFICACIÓN: ¿Por qué 0xFF?
    // 0xFF es el número 255 en hexadecimal
    // En binario: 11111111 (8 bits todos en 1)
    // Cada 1 significa "este pin es salida"
    
    // ============================================================================
    // CONFIGURACIÓN DEL TIMER1 PARA INTERRUPCIONES PRECISAS
    // ============================================================================
    
    // ¿QUÉ ES EL TIMER1?
    // Es un contador de hardware que cuenta pulsos de reloj automáticamente
    // Cuando llega a un valor específico, genera una interrupción
    
    Serial.println("Configurando Timer1 a 3840 Hz...");
    timer1.setup();         // Configurar registros del Timer1 según la frecuencia especificada
    timer1.start();         // Iniciar el conteo - ¡Las interrupciones empiezan AHORA!
    
    // ============================================================================
    // CONFIGURACIÓN DE LA COMUNICACIÓN UART
    // ============================================================================
    
    // ¿QUÉ ES UART?
    // Universal Asynchronous Receiver-Transmitter
    // Es el protocolo de comunicación serie con la PC
    
    Serial.println("Configurando UART a 38400 baudios...");
    usart.begin(38400);     // ¡CRÍTICO! Debe coincidir con SerialPlotter
                           // 38400 = 3840 muestras/seg × 10 bits/byte
                           // Esto sincroniza perfectamente el muestreo con la comunicación
    
    // ============================================================================
    // HABILITAR INTERRUPCIONES GLOBALES
    // ============================================================================
    
    // ¿QUÉ HACE sei()?
    // "Set Enable Interrupts" - habilita todas las interrupciones del sistema
    // Sin esta línea, la ISR del Timer1 nunca se ejecutaría
    
    Serial.println("Habilitando interrupciones...");
    sei();                  // ¡MOMENTO CRÍTICO! A partir de aquí empiezan las interrupciones
    
    Serial.println("=== SISTEMA DSP LISTO ===");
    Serial.println("Esperando datos de SerialPlotter...");
}

// ============================================================================
// BUCLE PRINCIPAL: PROCESAMIENTO CONTINUO DE DATOS
// ============================================================================

/**
 * ¿QUÉ ES LA FUNCIÓN loop()?
 * Es una función especial de Arduino que se ejecuta repetidamente
 * después de setup(). Es como un "while(true)" infinito.
 * 
 * FILOSOFÍA DEL DISEÑO:
 * - La ISR del Timer1 maneja el TIMING (cuándo)
 * - El loop() maneja los DATOS (qué enviar)
 * - Esta separación garantiza timing preciso independiente del procesamiento
 */
void loop() {
    // ============================================================================
    // SINCRONIZACIÓN CON EL TIMER1
    // ============================================================================
    
    // ¿POR QUÉ ESTA CONDICIÓN?
    // Solo procesamos datos cuando el Timer1 dice "es hora de una nueva muestra"
    // Esto mantiene la sincronización perfecta a 3840 Hz
    
    if (beat) {             // ¿La interrupción del Timer1 activó la bandera?
        beat = false;       // IMPORTANTE: Resetear la bandera inmediatamente
                           // Si no hacemos esto, el if se ejecutaría continuamente
        
        // ========================================================================
        // PASO 1: LEER NUEVA MUESTRA DEL ADC
        // ========================================================================
        
        // ¿POR QUÉ LEER EL ADC CADA VEZ?
        // Porque queremos capturar señales que cambian en tiempo real
        // Cada muestra representa el voltaje en este instante específico
        
        uint8_t muestra_adc = adc.get();  // Leer voltaje actual del pin A1
                                         // Resultado: 0-255 (el ADC 10-bit se escala a 8-bit)
        
        // ========================================================================
        // PASO 2: ENVIAR MUESTRA A LA PC PARA PROCESAMIENTO
        // ========================================================================
        
        // ¿POR QUÉ ENVIAR A LA PC?
        // Porque la PC tiene mucha más potencia de cálculo para:
        // - Filtros digitales complejos (Butterworth 8º orden)
        // - Análisis FFT en tiempo real
        // - Interfaz gráfica con visualización
        
        usart.escribir(muestra_adc);      // Transmitir por UART a 38400 baudios
                                         // ¡Esta muestra llegará a SerialPlotter!
        
        // ========================================================================
        // PASO 3: VERIFICAR SI HAY DATOS PROCESADOS DE LA PC
        // ========================================================================
        
        // ¿QUÉ SIGNIFICA "PROCESADOS"?
        // La PC recibió nuestra muestra, la filtró, y nos envió el resultado
        // Nosotros tomamos ese resultado y lo convertimos a voltaje analógico
        
        if (usart.pendiente_lectura()) {  // ¿Hay datos esperando en el buffer de recepción?
            valor = usart.leer();         // Leer el byte de datos filtrados (0-255)
                                         // Este valor será usado por la ISR en write()
        } else {
            // ========================================================================
            // MODO FALLBACK: SI NO HAY DATOS DE LA PC
            // ========================================================================
            
            // ¿POR QUÉ UN MODO FALLBACK?
            // Si SerialPlotter no está corriendo o hay problemas de comunicación,
            // el sistema seguirá funcionando pero sin filtrado
            
            valor = muestra_adc;          // Usar directamente la muestra del ADC
                                         // Esto es "bypass" - la señal pasa sin procesar
        }
        
        // ========================================================================
        // INFORMACIÓN DE DEPURACIÓN (OPCIONAL)
        // ========================================================================
        
        // Cada 1000 muestras (aprox. cada 260ms), imprimir estado del sistema
        static uint16_t contador_debug = 0;
        contador_debug++;
        
        if (contador_debug >= 1000) {
            contador_debug = 0;
            Serial.print("ADC: ");
            Serial.print(muestra_adc);
            Serial.print(" | DAC: ");
            Serial.print(valor);
            Serial.print(" | Comunicacion: ");
            Serial.println(usart.pendiente_lectura() ? "ACTIVA" : "FALLBACK");
        }
    }
    
    // ============================================================================
    // TAREAS DE MANTENIMIENTO (OPCIONAL)
    // ============================================================================
    
    // Aquí se pueden agregar tareas que no requieren timing preciso:
    // - Procesamiento de comandos de configuración
    // - Actualización de LEDs de estado
    // - Verificaciones de salud del sistema
    // - etc.
    
    // NOTA: Todo lo que esté aquí NO debe interferir con el timing del sistema
    // El beat flag garantiza que el procesamiento de muestras tenga prioridad
}

// ============================================================================
// EXPLICACIÓN DEL FLUJO COMPLETO DE UNA MUESTRA
// ============================================================================

/**
 * CRONOLOGÍA TÍPICA DE UNA MUESTRA (cada 260 microsegundos):
 * 
 * T=0µs:   Timer1 genera interrupción
 * T=1µs:   ISR ejecuta write(valor) - convierte digital→analógico
 * T=2µs:   ISR activa beat=true y termina
 * T=3µs:   loop() detecta beat=true
 * T=4µs:   loop() lee nueva muestra ADC (analógico→digital)
 * T=5µs:   loop() envía muestra por UART hacia PC
 * T=6µs:   loop() verifica si llegaron datos filtrados de PC
 * T=7µs:   loop() actualiza 'valor' para la próxima iteración
 * T=8µs:   loop() resetea beat=false y continúa
 * ...
 * T=260µs: Timer1 genera siguiente interrupción - ¡REPITE!
 * 
 * MIENTRAS TANTO EN LA PC:
 * - SerialPlotter recibe la muestra enviada en T=5µs
 * - Aplica filtros digitales (Butterworth, FFT, etc.)
 * - Calcula la muestra filtrada 
 * - Envía el resultado de vuelta por UART
 * - Arduino lo recibe en el próximo ciclo T=6µs
 * 
 * LATENCIA TOTAL DEL SISTEMA:
 * - Muestreo ADC: ~4µs
 * - Transmisión UART: ~260µs (1 byte a 38400 baud)
 * - Procesamiento PC: ~100-500µs (dependiendo del filtro)
 * - Recepción UART: ~260µs
 * - Conversión DAC: ~1µs
 * --------------------------------
 * TOTAL: ~0.6-0.8 ms de latencia
 * 
 * ¡Esto es EXCELENTE para procesamiento en tiempo real!
 */