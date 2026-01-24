/**
 * Generador Automático de Formas de Onda - Arduino Uno
 * 
 * Sistema que genera formas de onda con frecuencias y rangos de voltaje específicos
 * cambiando cada 30 segundos en un ciclo de 6 estados.
 * 
 * Hardware requerido: Arduino Uno
 * - Pines 2-7: DAC R2R de 6 bits (64 niveles)
 * - Pin 13: LED indicador que parpadea con cada cambio de forma
 * 
 * Ciclo automático (cada 30 segundos):
 * 1. Triangular 2Hz,   1V-4V   (offset 1V, amplitud 3V)
 * 2. Triangular 300Hz, 0V-5V   (offset 0V, amplitud 5V) 
 * 3. Cuadrada 2Hz,     1V-4V   (offset 1V, amplitud 3V)
 * 4. Cuadrada 300Hz,   0V-5V   (offset 0V, amplitud 5V)
 * 5. Senoidal 2Hz,     1V-4V   (offset 1V, amplitud 3V)
 * 6. Senoidal 300Hz,   0V-5V   (offset 0V, amplitud 5V)
 * - Repetir...
 * 
 * Frecuencia de muestreo: 3840 Hz
 * Resolución: 6 bits (0-63 niveles)
 * DAC: 0-63 → 0V-5V (aprox. 78.7mV por nivel)
 */

#include "timer1.h"
#include "tablas_6bit.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// Instancia del Timer1 a 3840 Hz
Timer1 timer1(3840.0);

// Variables para control de forma de onda
uint8_t estado_actual = 0;              // 0-5: Estados del ciclo
uint16_t indice_tabla = 0;              // Índice actual en la tabla de forma de onda
uint32_t ultimo_cambio = 0;             // Timestamp del último cambio de forma
const uint32_t INTERVALO_CAMBIO = 30000;  // 30 segundos en milisegundos

// Variables de estado
volatile bool generar_muestra = false;  // Flag de sincronización con timer
bool led_estado = false;                 // Estado del LED indicador

// Configuración de cada estado (forma, frecuencia, rango)
struct ConfigEstado {
  uint8_t forma;        // 0=Triangular, 1=Cuadrada, 2=Senoidal
  uint8_t frecuencia;   // Índice de frecuencia
  uint8_t offset_6bit;  // Offset en escala de 6 bits (0-63)
  uint8_t amp_6bit;     // Amplitud en escala de 6 bits (0-63)
  const char* nombre;   // Descripción para debug
};

const ConfigEstado configuraciones[6] = {
  {0, 0, 13, 38, "Triangular 2Hz 1V-4V"},    // offset=1V(13), amp=3V(38)
  {0, 1, 0,  63, "Triangular 300Hz 0V-5V"},  // offset=0V(0),  amp=5V(63) 
  {1, 0, 13, 38, "Cuadrada 2Hz 1V-4V"},      // offset=1V(13), amp=3V(38)
  {1, 1, 0,  63, "Cuadrada 300Hz 0V-5V"},    // offset=0V(0),  amp=5V(63)
  {2, 0, 13, 38, "Senoidal 2Hz 1V-4V"},      // offset=1V(13), amp=3V(38)
  {2, 1, 0,  63, "Senoidal 300Hz 0V-5V"}     // offset=0V(0),  amp=5V(63)
};

// Incrementos de índice para diferentes frecuencias
// Para 3840 Hz de muestreo y tabla de 256 muestras:
// - 2Hz:   incremento = (2 * 256) / 3840 = 0.133 ≈ usar cada 7.5 muestras
// - 300Hz: incremento = (300 * 256) / 3840 = 20
const uint16_t incrementos_freq[2] = {
  1,    // 2Hz: usar cada muestra (15Hz real con tabla 256 @ 3840Hz)
  20    // 300Hz: saltar 20 posiciones por muestra 
};

// Divisores para simular frecuencias bajas
uint16_t contador_div = 0;
const uint16_t divisores_freq[2] = {
  7,    // 2Hz: dividir por 7 (aproximadamente 2.14Hz)
  1     // 300Hz: sin división
};

/**
 * Escribe un valor de 6 bits al DAC R2R usando pines 2-7
 * 
 * Arduino Uno - DAC de 6 bits:
 * - Pines 2-7 (PORTD bits 7:2): Los 6 bits del DAC
 * - Pin 2 (PORTD.2) = Bit 0 (LSB)
 * - Pin 3 (PORTD.3) = Bit 1
 * - Pin 4 (PORTD.4) = Bit 2
 * - Pin 5 (PORTD.5) = Bit 3
 * - Pin 6 (PORTD.6) = Bit 4
 * - Pin 7 (PORTD.7) = Bit 5 (MSB)
 * 
 * @param valor Valor de 0-63 a convertir a voltaje analógico
 */
void escribir_dac(uint8_t valor) {
  // Desplazar valor para alinear con pines 2-7 (bits 7:2 de PORTD)
  uint8_t salida = valor << 2;
  
  // Preservar bits 0 y 1 de PORTD (pines 0 y 1 - UART)
  salida |= (PORTD & 0b00000011);
  
  // Escribir al puerto
  PORTD = salida;
}

/**
 * Interrupción del Timer1: Se ejecuta a 3840 Hz
 * Genera las formas de onda con frecuencia y amplitud configurables
 */
ISR(TIMER1_COMPA_vect) {
  // Obtener configuración actual
  const ConfigEstado& config = configuraciones[estado_actual];
  
  // Control de frecuencia usando divisores
  contador_div++;
  if (contador_div < divisores_freq[config.frecuencia]) {
    return;  // Saltar esta muestra para frecuencias bajas
  }
  contador_div = 0;
  
  // Obtener muestra base de la tabla (0-63)
  uint8_t muestra_base = formas_6bit[config.forma][indice_tabla];
  
  // Escalar y desplazar según configuración
  // Formula: salida = (muestra_base * amplitud / 63) + offset
  uint16_t muestra_escalada = (muestra_base * config.amp_6bit) / 63 + config.offset_6bit;
  
  // Saturar a 6 bits (0-63)
  if (muestra_escalada > 63) muestra_escalada = 63;
  
  // Escribir al DAC
  escribir_dac(muestra_escalada);
  
  // Avanzar índice con incremento según frecuencia
  indice_tabla += incrementos_freq[config.frecuencia];
  if (indice_tabla >= TABLA_SIZE) {
    indice_tabla = 0;
  }
  
  // Señalizar que se generó una muestra
  generar_muestra = true;
}

/**
 * Cambiar al siguiente estado en el ciclo
 * 6 estados: Tri2Hz_1-4V → Tri300Hz_0-5V → Cua2Hz_1-4V → Cua300Hz_0-5V → Sen2Hz_1-4V → Sen300Hz_0-5V
 */
void cambiar_estado() {
  estado_actual++;
  if (estado_actual >= 6) {
    estado_actual = 0;  // Volver al inicio del ciclo
  }
  
  // Reiniciar índices y contadores
  indice_tabla = 0;
  contador_div = 0;
  
  // Parpadear LED para indicar cambio
  led_estado = !led_estado;
  digitalWrite(13, led_estado);
  
  // Información por puerto serie
  Serial.print("Cambiando a estado ");
  Serial.print(estado_actual);
  Serial.print(": ");
  Serial.println(configuraciones[estado_actual].nombre);
}

/**
 * Configuración inicial del sistema
 */
void setup() {
  // Inicializar puerto serie para debug
  Serial.begin(9600);
  Serial.println("=== Generador Automático de Formas de Onda Avanzado ===");
  Serial.println("Arduino Uno - DAC 6 bits (Pines 2-7)");
  Serial.println("Ciclo de 6 estados, 30s cada uno:");
  Serial.println("1. Triangular 2Hz 1V-4V");
  Serial.println("2. Triangular 300Hz 0V-5V"); 
  Serial.println("3. Cuadrada 2Hz 1V-4V");
  Serial.println("4. Cuadrada 300Hz 0V-5V");
  Serial.println("5. Senoidal 2Hz 1V-4V");
  Serial.println("6. Senoidal 300Hz 0V-5V");
  Serial.println("");
  
  // Configurar pines del DAC como salida (pines 2-7)
  // PORTD bits 7:2 = pines 7,6,5,4,3,2
  DDRD |= 0b11111100;  // Solo pines 2-7 como salida (preservar UART)
  
  // Configurar LED indicador
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);  // LED encendido al inicio
  led_estado = true;
  
  // Configurar e iniciar Timer1
  timer1.setup();
  timer1.start();
  
  // Inicializar variables de control
  ultimo_cambio = millis();
  estado_actual = 0;  // Comenzar con triangular 2Hz 1V-4V
  indice_tabla = 0;
  contador_div = 0;
  
  Serial.print("Iniciando con estado 0: ");
  Serial.println(configuraciones[estado_actual].nombre);
  
  // Habilitar interrupciones globales
  sei();
}

/**
 * Bucle principal
 * Controla el cambio automático de estados cada 30 segundos
 */
void loop() {
  uint32_t tiempo_actual = millis();
  
  // Verificar si es tiempo de cambiar estado
  if (tiempo_actual - ultimo_cambio >= INTERVALO_CAMBIO) {
    cambiar_estado();
    ultimo_cambio = tiempo_actual;
  }
  
  // Mostrar información cada 5 segundos
  static uint32_t ultimo_info = 0;
  if (tiempo_actual - ultimo_info >= 5000) {
    uint32_t tiempo_restante = INTERVALO_CAMBIO - (tiempo_actual - ultimo_cambio);
    Serial.print("Estado actual: ");
    Serial.print(estado_actual);
    Serial.print(" - ");
    Serial.print(configuraciones[estado_actual].nombre);
    Serial.print(" | Cambio en: ");
    Serial.print(tiempo_restante / 1000);
    Serial.println(" segundos");
    ultimo_info = tiempo_actual;
  }
  
  // Pequeña pausa para no saturar el puerto serie
  delay(100);
}