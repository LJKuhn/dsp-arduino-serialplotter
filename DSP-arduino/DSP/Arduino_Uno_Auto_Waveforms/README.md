# Generador Automático de Formas de Onda - Arduino Uno

## Descripción

Este proyecto genera formas de onda automáticamente cambiando cada **15 segundos** entre **12 tipos diferentes**: **triangular**, **cuadrada** y **senoidal** en **4 frecuencias** (2Hz, 10Hz, 80Hz, 300Hz). Está optimizado para Arduino Uno usando un DAC R2R de 6 bits.

## Características

### **🔄 Ciclo Automático (12 estados × 15s = 3 minutos):**
1. **15s:** Triangular 2Hz (1V-4V)
2. **15s:** Triangular 10Hz (0V-5V)  
3. **15s:** Triangular 80Hz (0V-5V)
4. **15s:** Triangular 300Hz (0V-5V)
5. **15s:** Cuadrada 2Hz (1V-4V)
6. **15s:** Cuadrada 10Hz (0V-5V)
7. **15s:** Cuadrada 80Hz (0V-5V)
8. **15s:** Cuadrada 300Hz (0V-5V)
9. **15s:** Senoidal 2Hz (1V-4V)
10. **15s:** Senoidal 10Hz (0V-5V)
11. **15s:** Senoidal 80Hz (0V-5V)
12. **15s:** Senoidal 300Hz (0V-5V)
- **Repetir indefinidamente...**

### **📊 Especificaciones Técnicas:**
- **Plataforma:** Arduino Uno (ATmega328P)
- **Frecuencia de muestreo:** 3840 Hz
- **Resolución:** 6 bits (64 niveles)
- **Pines de salida:** 2-7 (PORTD)
- **Indicador:** LED en pin 13 parpadea con cada cambio

## Conexiones de Hardware

### **DAC R2R de 6 bits:**
```
Pin 2 (PORTD.2) = Bit 0 (LSB) → R de valor R
Pin 3 (PORTD.3) = Bit 1       → R de valor R  
Pin 4 (PORTD.4) = Bit 2       → R de valor R
Pin 5 (PORTD.5) = Bit 3       → R de valor R
Pin 6 (PORTD.6) = Bit 4       → R de valor R
Pin 7 (PORTD.7) = Bit 5 (MSB) → R de valor R

Todos conectados a una red R-2R hacia la salida analógica
```

### **Otros:**
- **Pin 13:** LED indicador de cambio de forma
- **Pines 0-1:** Puerto serie (115200 baud) para información de estado

## Archivos del Proyecto

### **Arduino_Uno_Auto_Waveforms.ino**
- Código principal con lógica de cambio automático
- Control del Timer1 y generación de interrupciones
- Interfaz serie para monitoreo

### **tablas_6bit.h**
- Tablas de formas de onda adaptadas a 6 bits
- Arrays de 256 muestras para cada forma
- Valores escalados de 0-63 para compatibilidad con 6 bits

### **timer1.h**
- Clase para control automático del Timer1
- Configuración de frecuencia y prescalers
- Modo CTC para interrupciones precisas

### **prescaler.h**  
- Funciones para cálculo automático de prescalers
- Optimización de configuración del timer
- Soporte para diferentes frecuencias

## Funcionamiento

### **1. Inicialización:**
```cpp
// Configurar DAC de 6 bits en pines 2-7
DDRD |= 0b11111100;

// Iniciar Timer1 a 3840 Hz  
timer1.setup();
timer1.start();
```

### **2. Generación de Señal:**
```cpp
ISR(TIMER1_COMPA_vect) {
  uint8_t muestra = formas_6bit[forma_actual][indice_tabla];
  escribir_dac(muestra);
  indice_tabla++;
}
```

### **3. Cambio Automático:**
```cpp
if (tiempo_actual - ultimo_cambio >= 30000) {
  cambiar_forma();  // Triangular → Cuadrada → Senoidal
  ultimo_cambio = tiempo_actual;
}
```

## Salida del Puerto Serie

El programa muestra información cada 5 segundos:

```
=== Generador Automático de Formas de Onda ===
Arduino Uno - DAC 6 bits (Pines 2-7)
Ciclo: Triangular (30s) -> Cuadrada (30s) -> Senoidal (30s)

Iniciando con forma: Triangular
Forma actual: Triangular | Cambio en: 25 segundos
Forma actual: Triangular | Cambio en: 20 segundos
...
Cambio a forma: Cuadrada
Forma actual: Cuadrada | Cambio en: 25 segundos
```

## Ventajas del Diseño

### **✅ Automatización Completa:**
- No requiere intervención manual
- Ciclo continuo e infinito
- Cambios precisos cada 30 segundos

### **✅ Optimizado para Arduino Uno:**
- Usa solo 6 bits = menor complejidad de hardware
- Preserva pines 0-1 para comunicación serie
- Eficiente en memoria y procesamiento

### **✅ Fácil Monitoreo:**
- LED indicador de cambios
- Información detallada por puerto serie
- Contador regresivo hasta próximo cambio

### **✅ Señales de Alta Calidad:**
- Timer1 con interrupciones precisas
- 3840 Hz = frecuencia estable
- 256 muestras por ciclo = buena resolución temporal

## Compilación

1. Abrir `Arduino_Uno_Auto_Waveforms.ino` en Arduino IDE
2. Seleccionar **Arduino Uno** como placa
3. Compilar y subir

**Uso de memoria típico:** ~2KB Flash, ~100 bytes RAM

## Modificaciones Posibles

### **Cambiar intervalos de tiempo:**
```cpp
const uint32_t INTERVALO_CAMBIO = 15000;  // 15 segundos
```

### **Cambiar frecuencia de muestreo:**
```cpp
Timer1 timer1(7680.0);  // 7680 Hz
```

### **Añadir más formas de onda:**
Agregar nuevas tablas en `tablas_6bit.h` y modificar el array `formas_6bit[]`.

---

**¡Proyecto listo para usar! Conecta el DAC R2R y disfruta de las formas de onda automáticas!** 🎵