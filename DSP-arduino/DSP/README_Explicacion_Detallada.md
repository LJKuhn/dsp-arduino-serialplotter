# Sistema DSP (Procesamiento Digital de Señales) para Arduino Mega 2560

## Descripción General

Este proyecto implementa un sistema completo de procesamiento digital de señales usando un **Arduino Mega 2560**. El sistema incluye conversión analógico-digital (ADC), generación de señales mediante tablas precalculadas, comunicación serial UART, y control de timing preciso mediante timers. Es un proyecto avanzado que maneja directamente los registros del microcontrolador ATmega2560.

**ACTUALIZACIÓN:** El código ha sido optimizado para **Arduino Mega 2560** usando PORTA completo (pines 22-29) para máxima eficiencia y menor jitter.

**CONFIGURACIÓN PARA SERIALPLOTTER:** El sistema ha sido configurado para trabajar en conjunto con la interfaz SerialPlotter a 38400 baudios para procesamiento DSP bidireccional en tiempo real.

## Ventajas del Arduino Mega 2560

### **Hardware Superior:**
- **PORTA completo:** Pines 22-29 (PA0-PA7) = 8 bits en un solo puerto
- **Cristal 16MHz:** Timing más preciso que versiones de menor frecuencia  
- **Más memoria:** 256KB Flash vs 32KB del Arduino Uno
- **Mayor RAM:** 8KB vs 2KB del Arduino Uno

### **Mejoras de Rendimiento:**
- ✅ **Escritura atómica:** `PORTA = valor` (1 operación vs 2 en Uno)
- ✅ **Menor jitter:** Sin desfase entre bits durante la escritura
- ✅ **Mayor eficiencia:** Código más simple y rápido
- ✅ **Mejor calidad de señal:** Sincronización perfecta de todos los bits

## Correcciones Implementadas

### 1. **Optimización para Mega 2560**
- **Hardware:** Adaptado para usar pines 22-29 (PORTA completo)
- **Código:** Función `write()` simplificada a una sola línea
- **Performance:** Escritura atómica elimina jitter entre bits

### 2. **Compatibilidad C++11**
- **Problema:** Las funciones `constexpr` con múltiples sentencias no son compatibles con C++11
- **Solución:** Removidos todos los `constexpr` de prescaler.h y timer1.h
- **Impacto:** Cálculos se realizan en runtime, funcionalidad idéntica

### 3. **Corrección de ISR (Interrupt Service Routines)**
- **Problema**: ISR mal definidos para ATmega2560: `USART_UDRE_vect` vs `USART0_UDRE_vect`
- **Solución**: Cambiados a `USART0_UDRE_vect` y `USART0_RX_vect` para Mega 2560
- **Beneficio**: Interrupciones funcionan correctamente en ATmega2560

### 4. **Corrección de Baudrate**
- **Problema**: Configuración incorrecta de 115200 baudios vs 38400 esperados
- **Solución**: Sincronización correcta `usart.begin(38400)` 
- **Beneficio**: Comunicación estable con SerialPlotter

### 5. **Corrección de instancia USART**
- **Problema**: Instancia `USART usart;` declarada correctamente en usart.h línea 165
- **Solución**: Se eliminó declaración duplicada innecesaria
- **Resultado**: Compilación exitosa sin redefiniciones

### 5. **Documentación Completa**
- **Añadido:** Comentarios explicativos en español en todos los archivos
- **Incluye:** Descripción de funciones, parámetros, registros y algoritmos
- **Específico:** Documentación actualizada para Arduino Mega 2560

## Arquitectura del Sistema

```
[ADC] → [Arduino Mega 2560/DSP] → [DAC (Pines 22-29)] → [Señal Analógica]
                    ↑
         [Comunicación UART]
                    ↑
         [Control PC/Terminal]
```

## Mapeo de Pines - Arduino Mega 2560

### **DAC R2R (8 bits):**
```
Pin 22 (PA0) = Bit 0 (LSB)
Pin 23 (PA1) = Bit 1
Pin 24 (PA2) = Bit 2  
Pin 25 (PA3) = Bit 3
Pin 26 (PA4) = Bit 4
Pin 27 (PA5) = Bit 5
Pin 28 (PA6) = Bit 6
Pin 29 (PA7) = Bit 7 (MSB)
```

### **Otros:**
- **Pin 13:** LED indicador de estado
- **USART0:** Comunicación serie (pines 0 y 1)
- **Cualquier pin analógico:** Entrada ADC opcional

## Conceptos Fundamentales: Archivos .h y .cpp

### ¿Qué son los archivos .h (Header Files)?

Los archivos de cabecera (`.h`) contienen las **declaraciones** de clases, funciones, constantes y macros. Su propósito es:

- **Interfaz Pública**: Define QUÉ puede hacer una clase o función, pero no CÓMO lo hace
- **Declaraciones**: Especifica nombres de funciones, parámetros y tipos de retorno
- **Prototipos**: Permite al compilador verificar que las funciones se usen correctamente
- **Inclusión Múltiple**: Se pueden incluir en múltiples archivos sin problemas

**Ejemplo en el proyecto:**
```cpp
// adc.h - Solo declara la interfaz
class ADCController {
public:
   void begin(int pin);        // Declara la función
   uint8_t get();             // Declara la función
   bool available();          // Declara la función
};
```

### ¿Qué son los archivos .cpp (Implementation Files)?

Los archivos de implementación (`.cpp`) contienen las **definiciones** reales del código. Su propósito es:

- **Implementación Real**: Define CÓMO funcionan las funciones declaradas en .h
- **Código Ejecutable**: Contiene la lógica y algoritmos reales
- **Compilación Separada**: Se compilan independientemente y se enlazan después
- **Encapsulación**: Oculta detalles internos de implementación

**Ejemplo en el proyecto:**
```cpp
// adc.cpp - Implementa la funcionalidad
void ADCController::begin(int pin) {
   // Aquí está el código REAL que hace el trabajo
   ADCSRA = ACTIVAR | AUTO_TRIGGER | PRESCALER_128 | ADC_INTERRUPT;
   ADCSRB = MODO_CONTINUO;
   ADMUX = AVcc | AJUSTAR_IZQUIERDA | pin;
   ADCSRA |= EMPEZAR;
}
```

### Ventajas de la Separación .h/.cpp

1. **Modularidad**: Cada módulo tiene responsabilidades claras
2. **Reusabilidad**: Los .h se pueden incluir en múltiples proyectos
3. **Tiempo de Compilación**: Solo se recompila lo que cambió
4. **Mantenibilidad**: Cambios en implementación no afectan otros archivos
5. **Encapsulación**: Los detalles internos permanecen ocultos

### Casos Especiales en este Proyecto

#### Archivos Solo .h (Header-Only)
Algunos archivos como `timer1.h`, `prescaler.h` y `usart.h` contienen tanto declaración como implementación:

```cpp
// timer1.h - Header-only con funciones constexpr e inline
class Timer1 {
   constexpr Timer1(float frecuencia) {  // Implementado en .h
      prescaler = elegir_prescaler(frecuencia, 65535);
      // ... implementación aquí mismo
   }
};
```

**¿Por qué solo .h?**
- **Funciones constexpr**: Deben ser evaluadas en tiempo de compilación
- **Templates**: Necesitan estar disponibles para especialización
- **Funciones pequeñas**: Pueden ser inlined para optimización
- **Clases simples**: Cuando la implementación es trivial

#### Archivos .ino (Arduino Sketch)
```cpp
// DSP.ino - Formato especial de Arduino
// Combina elementos de .h y .cpp pero con estructura específica
void setup() { }  // Función especial de inicialización
void loop() { }   // Función especial de bucle principal
```

## Estructura de Archivos

### Archivo Principal
- **DSP.ino**: Programa principal de Arduino que orquesta todo el sistema

### Módulos de Hardware
- **adc.h / adc.cpp**: Control del conversor analógico-digital
  - `.h`: Declara la clase ADCController y sus métodos públicos
  - `.cpp`: Implementa la configuración real de registros AVR
- **timer1.h**: Control del Timer1 para timing preciso (header-only)
- **usart.h**: Comunicación serial UART con buffers (header-only)
- **prescaler.h**: Cálculo de prescalers para timers (funciones constexpr)
- **tablas.h**: Tablas de valores para diferentes formas de onda (datos constantes)

### Carpetas
- **separado/**: Versiones modulares separadas de algunos componentes
  - Implementaciones alternativas con separación .h/.cpp más estricta
- **sin usar/**: Código experimental no utilizado en la versión final

---

## Análisis Detallado por Módulos

### 1. Módulo ADC (Conversor Analógico-Digital)

#### Separación .h/.cpp en el Módulo ADC

**Archivo adc.h (Declaraciones)**:
```cpp
class ADCController
{
   uint16_t data = -1;           // Variables privadas (solo declaradas)
   bool not_get = false;         
   
   void conversion_complete();   // Método privado (solo declaración)
   
public:
   void begin(int pin);          // Interfaz pública (solo firma)
   uint8_t get();               // Métodos públicos (solo declaración)
   bool available();            
   void start();                
   void stop();                 
   uint8_t ahora(int pin);      
};
```

**Archivo adc.cpp (Implementaciones)**:
```cpp
#include "adc.h"                 // Incluye las declaraciones
#include <avr/io.h>             // Librerías necesarias

// Constantes de configuración (detalles internos)
constexpr uint8_t ACTIVAR = 1 << ADEN;
constexpr uint8_t EMPEZAR = 1 << ADSC;
// ... más constantes

void ADCController::conversion_complete() {
  // IMPLEMENTACIÓN REAL - manipulación de registros AVR
  uint8_t high = ADCH;
  not_get = true;
  data = high;
}

void ADCController::begin(int pin) {
  // IMPLEMENTACIÓN REAL - configuración de hardware
  ADCSRA = ACTIVAR | AUTO_TRIGGER | PRESCALER_128 | ADC_INTERRUPT;
  ADCSRB = MODO_CONTINUO;
  ADMUX = AVcc | AJUSTAR_IZQUIERDA | pin;
  ADCSRA |= EMPEZAR;
}
```

**¿Por qué esta separación es útil aquí?**
- **Ocultación de detalles**: Las constantes de registros AVR están solo en .cpp
- **Cambios de implementación**: Se puede cambiar el algoritmo sin tocar .h
- **Múltiples plataformas**: Se podría tener adc_atmega.cpp, adc_esp32.cpp, etc.
- **Facilita testing**: Se pueden crear mocks basados solo en el .h

#### Archivo: adc.h
```cpp
class ADCController
{
   uint16_t data = -1;           // Almacena el último valor leído
   bool not_get = false;         // Flag para datos nuevos disponibles
   
   void conversion_complete();   // Llamado por interrupción
   
public:
   void begin(int pin);          // Inicializar ADC en un pin específico
   uint8_t get();               // Obtener último valor
   bool available();            // Verificar si hay datos nuevos
   void start();                // Iniciar conversiones
   void stop();                 // Detener conversiones
   uint8_t ahora(int pin);      // Lectura inmediata (bloquea)
};
```

#### Funcionalidades:
- **Configuración AVR**: Usa registros ADCSRA, ADCSRB, ADMUX directamente
- **Modo Auto-Trigger**: Conversiones automáticas continuas
- **Prescaler 128**: Para frecuencia de muestreo adecuada
- **Ajuste a la Izquierda**: Para obtener 8 bits más significativos en ADCH
- **Interrupciones**: Maneja ISR(ADC_vect) para procesamiento asíncrono

#### Proceso de Conversión:
1. Configura el pin analógico (A1 por defecto)
2. Establece referencia de voltaje (AVcc)
3. Configura prescaler para timing correcto
4. Habilita conversiones automáticas continuas
5. Genera interrupción al completar cada conversión

### 2. Módulo Timer1 (Control de Timing)

#### ¿Por qué Timer1 es Header-Only?

**Archivo timer1.h (Todo en uno)**:
```cpp
class Timer1 {
  uint16_t prescaler = 256;      // Variables de configuración
  uint8_t bits_prescaler = 0b100; 
  uint16_t comparador = 0;       

public:
  constexpr Timer1(float frecuencia) {    // IMPLEMENTADO AQUÍ MISMO
    prescaler = elegir_prescaler(frecuencia, 65535);
    bits_prescaler = obtener_bits_prescaler(prescaler);
    comparador = 16e6 / (prescaler * frecuencia) - 1;
  }

  void setup(){                           // IMPLEMENTADO AQUÍ MISMO  
    const int modo = 4;
    const uint8_t wgm10 = modo & 0b11;
    const uint8_t wgm32 = modo & 0b1100;
    TCCR1A = wgm10;
    TCCR1B = wgm32 << 1;
    OCR1A = comparador;
  }
};
```

**Razones para Header-Only:**
1. **Constructor constexpr**: Debe calcularse en tiempo de compilación
2. **Funciones pequeñas**: El compilador puede optimizar mejor (inline)
3. **Configuración específica**: Cada instancia puede tener parámetros únicos
4. **Template-like behavior**: Aunque no es template, se comporta como tal

**Ventajas en este caso:**
- **Cálculos en compile-time**: `Timer1 timer(3840.0)` calcula todo al compilar
- **Optimización**: Las funciones pequeñas se inline automáticamente
- **Menos archivos**: Simplifica la estructura del proyecto
- **Tipo-seguridad**: Los errores se detectan al compilar, no al ejecutar

#### Archivo: timer1.h
```cpp
class Timer1 {
  uint16_t prescaler = 256;      // Factor de división de reloj
  uint8_t bits_prescaler = 0b100; // Configuración de registros
  uint16_t comparador = 0;       // Valor de comparación OCR1A

public:
  constexpr Timer1(float frecuencia);  // Constructor con cálculo automático
  void setup();                        // Configurar registros
  void start();                       // Iniciar timer con interrupciones
  void stop();                        // Detener timer
  void set_frequency(float frecuencia); // Cambiar frecuencia dinámicamente
};
```

#### Cálculo de Parámetros:
```cpp
// Frecuencia objetivo: 3840 Hz en el código principal
prescaler = elegir_prescaler(frecuencia, 65535);
comparador = 16e6 / (prescaler * frecuencia) - 1;
```

#### Configuración de Hardware:
- **Modo CTC (Clear Timer on Compare)**: El timer se reinicia al alcanzar OCR1A
- **Interrupción OCIE1A**: Genera ISR(TIMER1_COMPA_vect) periódicamente
- **Prescalers disponibles**: 1, 8, 64, 256, 1024

### 3. Módulo USART (Comunicación Serial)

#### USART: Otro Caso Header-Only

**Archivo usart.h (Implementación completa)**:
```cpp
class USART {
    // Variables privadas declaradas e inicializadas
    uint8_t buffer_escritura[128], buffer_lectura[64];
    volatile uint8_t inicio_e = 0, fin_e = 0, inicio_l = 0, fin_l = 0;
    
    // Métodos privados inline
    bool registro_vacio() {
        return UCSR0A & (1 << UDRE0);
    }
    
public:
    // Métodos públicos implementados directamente
    void begin(uint32_t baud) {
        UBRR0 = 16e6 / (8 * baud) - 1;
        UCSR0A = doble_velocidad;
        // ... resto de la implementación aquí
    }
    
    bool escribir(uint8_t byte) {
        // Implementación completa aquí
        if (!pendiente_escritura() && registro_vacio()){
            UDR0 = byte;
            return true;
        }
        // ... resto del código
    }
};

USART usart;  // Instancia global declarada en el .h
```

**¿Por qué USART está todo en .h?**
1. **Funciones pequeñas e inline**: Mejor rendimiento
2. **Acceso frecuente**: Se usa constantemente en interrupciones
3. **Configuración en compile-time**: Optimizaciones del compilador
4. **Singleton pattern**: Una sola instancia global `usart`

**Comparación con separación .h/.cpp:**
```cpp
// Si fuera separado:
// usart.h - Solo declaraciones
class USART {
public:
    void begin(uint32_t baud);     // Solo declaración
    bool escribir(uint8_t byte);   // Solo declaración
};

// usart.cpp - Implementaciones
void USART::begin(uint32_t baud) {
    // Implementación aquí
}
```

**Ventajas del enfoque header-only para USART:**
- **Performance**: Funciones críticas se inline automáticamente
- **Simplicidad**: Un solo archivo para mantener
- **Optimización**: El compilador ve todo el contexto

#### Archivo: usart.h
```cpp
class USART {
    // Buffers circulares para entrada y salida
    uint8_t buffer_escritura[128], buffer_lectura[64];
    volatile uint8_t inicio_e, fin_e, inicio_l, fin_l;
    
public:
    void begin(uint32_t baud);        // Inicializar comunicación
    bool escribir(uint8_t byte);      // Escritura no bloqueante
    void escribir_espera(uint8_t byte); // Escritura bloqueante
    uint8_t leer();                   // Lectura del buffer
    uint8_t leer_espera();           // Lectura bloqueante
    bool pendiente_lectura();        // Datos disponibles para leer
    bool pendiente_escritura();      // Datos pendientes de envío
};
```

#### Características Avanzadas:
- **Buffers Circulares**: Manejo eficiente de datos sin pérdida
- **Interrupciones Múltiples**: 
  - `ISR(USART_UDRE_vect)`: Registro de transmisión vacío
  - `ISR(USART_RX_vect)`: Dato recibido
- **Velocidad Doble**: Usa U2X0 para mayor precisión en baud rate
- **Gestión de Flujo**: Control de buffers llenos/vacíos

#### Configuración de Registros:
```cpp
UBRR0 = 16e6 / (8 * baud) - 1;  // Cálculo de baud rate
UCSR0A = doble_velocidad;
UCSR0B = interrupcion_rx | interrupcion_registro_vacio | activar_tx | activar_rx;
UCSR0C = modo_asincrono | paridad_desactivada | parada_1bit | caracter_8bits;
```

### 4. Módulo de Prescalers

#### Prescaler.h: Funciones Constexpr Puras

**Archivo prescaler.h (Solo funciones constexpr)**:
```cpp
#pragma once 

constexpr uint16_t elegir_prescaler(float frecuencia, float limite){
  constexpr uint16_t pres[] = { 1, 8, 64, 256, 1024 };
  int i = 0;
  while (i < 5 && 16e6 / (pres[i] * frecuencia) - 1 > limite)
    i++;
  return i < 5 ? pres[i] : 0;
}

constexpr uint8_t obtener_bits_prescaler(uint16_t prescaler){
  switch (prescaler){
    case 1: return 1;
    case 8: return 2;
    case 64: return 3;
    case 256: return 4;
    case 1024: return 5;
    default: return 0;
  }
}
```

**¿Por qué constexpr en .h?**
- **Evaluación en compile-time**: Los cálculos se hacen al compilar, no al ejecutar
- **Zero runtime cost**: No consume tiempo ni memoria en ejecución
- **Template-like**: Se comporta como una función template
- **Validación temprana**: Errores se detectan al compilar

**Ejemplo de uso:**
```cpp
Timer1 timer1(3840.0);  // elegir_prescaler se calcula AQUÍ en compile-time
// Resultado: prescaler=256, bits=4, comparador=1302
```

#### Propósito:
- Optimiza automáticamente la configuración de timers
- Garantiza máxima resolución posible  
- Validación en tiempo de compilación

### 5. Tablas de Formas de Onda

#### Tablas.h: Solo Datos Constantes

**Archivo tablas.h (Arrays constantes)**:
```cpp
const uint8_t triangular[] = {
    0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
    // ... 256 valores total
};

const uint8_t senoidal[] = {
    128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162,
    // ... 256 valores total  
};

const uint8_t cuadrada[] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    // ... primera mitad en 255, segunda en 0
};
```

**¿Por qué las tablas están en .h y no .cpp?**
1. **Solo datos**: No hay lógica compleja que ocultar
2. **Uso múltiple**: Pueden usarse desde diferentes archivos
3. **Optimización**: El compilador puede optimizar acceso directo
4. **Inmutabilidad**: Son `const`, no pueden modificarse

**Alternativa con .cpp sería:**
```cpp
// tablas.h
extern const uint8_t senoidal[];   // Solo declaración
extern const uint8_t triangular[]; 
extern const uint8_t cuadrada[];   

// tablas.cpp  
#include "tablas.h"
const uint8_t senoidal[] = { 128, 131, 134, ... };  // Definición
const uint8_t triangular[] = { 0, 2, 4, ... };
const uint8_t cuadrada[] = { 255, 255, ... };
```

**Ventajas del enfoque actual (.h only):**
- **Simplicidad**: Un solo archivo para mantener
- **Acceso directo**: No hay indirección extra
- **Inlining**: Los datos pueden optimizarse mejor

#### Archivo: tablas.h

Contiene tres arrays predefinidos de 256 elementos cada uno:

1. **Onda Triangular**:
   ```cpp
   const uint8_t triangular[] = {
       0, 2, 4, 6, ..., 255, 253, 251, ...
   };
   ```

2. **Onda Senoidal**:
   ```cpp
   const uint8_t senoidal[] = {
       128, 131, 134, 137, ...
   };
   ```

3. **Onda Cuadrada**:
   ```cpp
   const uint8_t cuadrada[] = {
       255, 255, ..., 255,  // Primera mitad en alto
       0, 0, ..., 0          // Segunda mitad en bajo
   };
   ```

## Resumen de Decisiones de Diseño .h/.cpp

| Módulo | Tipo | Razón de la Elección |
|--------|------|---------------------|
| **ADC** | .h + .cpp | Hardware complejo, implementación oculta, múltiples plataformas posibles |
| **Timer1** | Solo .h | Funciones constexpr, optimización compile-time, clase pequeña |
| **USART** | Solo .h | Performance crítico, funciones inline, singleton global |
| **Prescaler** | Solo .h | Funciones constexpr puras, cálculo compile-time |
| **Tablas** | Solo .h | Solo datos constantes, acceso directo optimizado |

### Criterios de Decisión para .h vs .cpp:

**Usa separación .h/.cpp cuando:**
- ✅ La implementación es compleja y puede cambiar
- ✅ Hay detalles internos que deben ocultarse  
- ✅ Se manipula hardware específico (registros AVR)
- ✅ El código puede tener múltiples implementaciones
- ✅ Los tiempos de compilación son importantes

**Usa solo .h cuando:**
- ✅ Las funciones son pequeñas y se benefician de inlining
- ✅ Usas constexpr para cálculo en compile-time
- ✅ Son solo datos constantes (arrays, tablas)
- ✅ El performance es crítico (interrupciones)
- ✅ La implementación es simple y estable

**Características Avanzadas:**
- **8 bits de resolución**: Valores de 0 a 255
- **256 muestras por ciclo**: Para suavidad en la reproducción
- **Offset DC**: Onda senoidal centrada en 128 (2.5V con referencia 5V)

---

## Programa Principal (DSP.ino)

### Configuración de Hardware

#### DAC por Resistencias (R2R)
```cpp
void write(uint8_t valor){
  uint8_t lsb = valor << 2;  // 6 bits menos significativos a pines 2-7
  uint8_t msb = valor >> 6;  // 2 bits más significativos a pines 8-9
  PORTD = lsb;              // Puerto D (pines 2-7)
  PORTB = msb;              // Puerto B (pines 8-9)
}
```

#### Configuración de Pines:
```cpp
void setup() {
  DDRD = 0b11111100;  // Pines 2-7 como salida
  DDRB = 0b00000011;  // Pines 8-9 como salida
}
```

### Manejo de Interrupciones

#### ISR Timer1 - Generación de Señal
```cpp
ISR(TIMER1_COMPA_vect) {
  write(valor);      // Envía valor actual al DAC
  beat = true;       // Señaliza nuevo ciclo
}
```

#### ISR ADC - Conversión Completa
```cpp
ISR(ADC_vect) {
   adc.conversion_complete();  // Procesa nueva muestra
}
```

#### ISR USART - Comunicación
```cpp
ISR(USART_UDRE_vect) {
   usart.udrie();  // Envía siguiente byte del buffer
}

ISR(USART_RX_vect) {
   uint8_t leido = UDR0;
   // Almacena en buffer circular
   if (usart.libre_lectura()){
      usart.buffer_lectura[usart.fin_l] = leido;
      usart.fin_l = (usart.fin_l + 1) % sizeof(usart.buffer_lectura);
   }
}
```

### Loop Principal - Protocolo de Comunicación

El loop implementa un protocolo de prueba de comunicación:

```cpp
void loop() {
   static uint32_t inicio = millis();
   
   if (i < 250){
      usart.escribir_espera(i);           // Envía contador 0-249
      if (usart.pendiente_lectura())
         lectura[contador++] = usart.leer(); // Almacena respuestas
      i++;
   }
   else if (i == 250 && enviar) {
      enviar = false;
      // Verifica integridad de datos recibidos
      uint32_t tiempo = millis() - inicio;
      
      // Envía timing information
      usart.escribir_espera(' ');
      usart.escribir_espera(' ');
      usart.escribir_espera('T');
      usart.escribir_espera(tiempo & 255);        // LSB
      usart.escribir_espera((tiempo >> 8) & 255); // Byte 2
      usart.escribir_espera((tiempo >> 16) & 255);// Byte 3
      usart.escribir_espera((tiempo >> 24) & 255);// MSB
      
      if (contador == 250)
         digitalWrite(13, true);  // LED indica éxito
   }
}
```

---

## Flujo de Operación del Sistema

### 1. Inicialización
1. **Configuración ADC**: Pin A1, conversión continua, interrupción habilitada
2. **Configuración UART**: 9600 baud, buffers circulares, interrupciones
3. **Configuración Pines**: D2-D7 y B0-B1 como salida (DAC 8-bit)
4. **Configuración Timer1**: 3840 Hz, modo CTC, interrupción habilitada
5. **Inicio del Timer**: Comienzan las interrupciones periódicas

### 2. Operación en Tiempo Real

#### Cada 1/3840 segundos (Timer1):
- Se ejecuta `ISR(TIMER1_COMPA_vect)`
- Se envía el valor actual al DAC via `write(valor)`
- Se marca `beat = true` para sincronización

#### Cuando se completa conversión ADC:
- Se ejecuta `ISR(ADC_vect)`
- Se almacena el nuevo valor en el objeto ADC
- El valor queda disponible para el programa principal

#### Durante comunicación UART:
- **Transmisión**: `ISR(USART_UDRE_vect)` envía bytes del buffer
- **Recepción**: `ISR(USART_RX_vect)` almacena bytes recibidos

### 3. Protocolo de Prueba
- Envía secuencia 0-249 por UART
- Recibe y valida las respuestas
- Mide tiempo total de la prueba
- Reporta resultados y estadísticas

---

## Características Avanzadas del Diseño

### 1. Programación de Bajo Nivel
- **Manipulación directa de registros**: ADCSRA, TIMSK1, PORTD, etc.
- **Gestión manual de interrupciones**: ISR vectorizadas
- **Control preciso de timing**: Cálculo exacto de prescalers

### 2. Arquitectura Orientada a Objetos en C++
- **Encapsulación**: Clases para ADC, Timer1, USART
- **Constructores constexpr**: Cálculos en tiempo de compilación
- **RAII**: Gestión automática de recursos

### 3. Buffers Circulares
- **Eficiencia**: Operación O(1) para insertar/extraer
- **Seguridad**: Protección contra overflow
- **Concurrencia**: Seguro para uso con interrupciones

### 4. Generación de Señales
- **DAC de 8 bits**: Usando red R-2R con pines digitales
- **Múltiples formas de onda**: Senoidal, triangular, cuadrada
- **Frecuencia configurable**: Via Timer1 programable

---

## Consideraciones de Rendimiento

### Frecuencias Importantes:
- **Timer1**: 3840 Hz (período de 260.4 μs)
- **ADC**: ~9600 Hz (con prescaler 128 y 16 MHz)
- **UART**: 9600 baud (1.04 ms por byte)
- **Reloj del sistema**: 16 MHz

### Limitaciones:
- **Resolución DAC**: 8 bits (256 niveles)
- **Ancho de banda**: Limitado por frecuencia de muestreo
- **Memoria**: Arrays de tablas ocupan ~768 bytes
- **Buffers UART**: 128 bytes TX, 64 bytes RX

### Optimizaciones:
- **Cálculo de prescalers en compile-time**: constexpr
- **Interrupciones vectorizadas**: Mínima latencia
- **Buffers circulares**: Operaciones eficientes
- **Tablas precalculadas**: Evita cálculos trigonométricos

---

## Casos de Uso del Sistema

1. **Generador de funciones**: Produce ondas senoidal, triangular, cuadrada
2. **Sistema de prueba**: Valida comunicación y timing
3. **Adquisición de datos**: Muestreo continuo vía ADC
4. **Procesamiento de señales**: Base para filtros digitales
5. **Educación**: Demostración de conceptos DSP fundamentales
6. **Integración con SerialPlotter**: Sistema bidireccional ADC ↔ PC ↔ DAC

## Configuración con SerialPlotter

### Sistema DSP Bidireccional

El código DSP.ino ha sido configurado para trabajar en conjunto con la interfaz **SerialPlotter**, creando un sistema completo de procesamiento digital de señales en tiempo real:

#### Funcionamiento:
```
[ADC] → [Arduino] → [UART] → [SerialPlotter] → [Filtros IIR] → [UART] → [DAC] → [Salida]
         3840 Hz      38400 baud    Butterworth    38400 baud    3840 Hz
```

#### Parámetros de Configuración:
- **Frecuencia de muestreo**: 3840 Hz (Timer1)
- **Baudrate**: 38400 baudios (calculado: 3840 Hz × 10 bits/byte)
- **Comunicación**: Bidireccional no bloqueante
- **Latencia**: ~0.6-0.8 ms (3 muestras de retardo)

#### Características:
1. **ADC continuo**: Lectura a 3840 Hz del canal 1
2. **Transmisión**: Muestras ADC enviadas al SerialPlotter
3. **Procesamiento**: Filtros Butterworth de 8º orden (pasa-bajos/pasa-altos)
4. **Recepción**: Datos procesados recibidos desde SerialPlotter  
5. **DAC**: Salida analógica mediante red R2R

#### Fallback Inteligente:
Si SerialPlotter no responde, el Arduino usa inversión local:
```cpp
if (usart.pendiente_lectura()){
   valor = usart.leer();        // Usar datos procesados del PC
}
else {
   valor = 255 - muestra_adc;   // Fallback: inversión local
}
```

#### Ventajas del Sistema Integrado:
- ✅ **Visualización en tiempo real**: Gráficas de entrada y salida
- ✅ **FFT**: Análisis espectral instantáneo  
- ✅ **Filtros profesionales**: Butterworth IIR configurables
- ✅ **Sin pérdida de muestras**: Comunicación no bloqueante
- ✅ **Calibración**: Mapeo ADC ↔ voltaje configurable

## Posibles Extensiones

1. **Filtros digitales**: IIR, FIR
2. **FFT**: Análisis espectral en tiempo real
3. **Control adaptativo**: Parámetros dinámicos
4. **Múltiples canales**: ADC multiplexado
5. **Interfaz gráfica**: Visualización en PC
6. **Almacenamiento**: Log de datos en EEPROM/SD

---

*Este sistema representa un ejemplo completo de procesamiento digital de señales embebido, combinando hardware de bajo nivel con software estructurado para crear una plataforma versátil de experimentación y aprendizaje.*