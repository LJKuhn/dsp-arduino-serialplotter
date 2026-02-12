# Generador Automático de Formas de Onda - Arduino Uno

## 🎯 **Descripción**

Sistema avanzado que genera **12 tipos diferentes de señales** de forma automática, cambiando cada **15 segundos** en un ciclo repetitivo. Cada señal tiene **frecuencia y rango de voltaje específicos** para pruebas completas de sistemas DSP.

## 📊 **Ciclo de Señales (12 estados × 15s = 3 minutos)**

| Estado | Forma      | Frecuencia | Rango Voltaje | Offset | Amplitud | Descripción |
|--------|------------|------------|---------------|---------|----------|-------------|
| **0**  | Triangular | **2 Hz**   | **1V - 4V**   | 1V     | 3V       | Señal lenta, rango medio |
| **1**  | Triangular | **10 Hz**  | **0V - 5V**   | 0V     | 5V       | Señal media, rango completo |
| **2**  | Triangular | **80 Hz**  | **0V - 5V**   | 0V     | 5V       | Señal media-alta, rango completo |
| **3**  | Triangular | **300 Hz** | **0V - 5V**   | 0V     | 5V       | Señal rápida, rango completo |
| **4**  | Cuadrada   | **2 Hz**   | **1V - 4V**   | 1V     | 3V       | Señal lenta, rango medio |
| **5**  | Cuadrada   | **10 Hz**  | **0V - 5V**   | 0V     | 5V       | Señal media, rango completo |
| **6**  | Cuadrada   | **80 Hz**  | **0V - 5V**   | 0V     | 5V       | Señal media-alta, rango completo |
| **7**  | Cuadrada   | **300 Hz** | **0V - 5V**   | 0V     | 5V       | Señal rápida, rango completo |
| **8**  | Senoidal   | **2 Hz**   | **1V - 4V**   | 1V     | 3V       | Señal lenta, rango medio |
| **9**  | Senoidal   | **10 Hz**  | **0V - 5V**   | 0V     | 5V       | Señal media, rango completo |
| **10** | Senoidal   | **80 Hz**  | **0V - 5V**   | 0V     | 5V       | Señal media-alta, rango completo |
| **11** | Senoidal   | **300 Hz** | **0V - 5V**   | 0V     | 5V       | Señal rápida, rango completo |

**Después del estado 11, vuelve automáticamente al estado 0.**

## ⚡ **Especificaciones Técnicas**

- **Plataforma**: Arduino Uno
- **DAC**: R2R 6 bits (pines 2-7) = 64 niveles
- **Resolución**: ~78.7 mV por nivel (5V ÷ 64)
- **Frecuencia de muestreo**: 3840 Hz
- **Puerto serie**: 9600 baudios para información
- **Indicador**: LED pin 13 parpadea en cada cambio

## 🔌 **Conexiones Hardware**

### **DAC R2R de 6 bits:**
```
Pin 2 (PORTD.2) → Resistor 2R → Bit 0 (LSB)
Pin 3 (PORTD.3) → Resistor 2R → Bit 1  
Pin 4 (PORTD.4) → Resistor 2R → Bit 2
Pin 5 (PORTD.5) → Resistor 2R → Bit 3
Pin 6 (PORTD.6) → Resistor 2R → Bit 4
Pin 7 (PORTD.7) → Resistor 2R → Bit 5 (MSB)
                      ↓
              Salida analógica 0V-5V
```

### **Valores típicos resistores:**
- **R**: 10kΩ (resistores de precisión 1%)
- **2R**: 20kΩ (o dos de 10kΩ en serie)

## 🚀 **Modo de Uso**

1. **Compilar y subir** el código al Arduino Uno
2. **Conectar DAC R2R** a los pines 2-7
3. **Conectar osciloscopio** a la salida del DAC
4. **Abrir monitor serie** (9600 baudios) para ver estado actual
5. **Observar señales** que cambian automáticamente cada 30 segundos

## 📈 **Ventajas para Testing DSP**

### **Señales de 2 Hz** (Estados 0, 2, 4):
- ✅ **Visualización fácil**: Se puede ver forma completa en osciloscopio
- ✅ **Calibración**: Permite ajustar ganancia y offset de sistemas
- ✅ **Debugging**: Ideal para verificar funcionamiento básico

### **Señales de 300 Hz** (Estados 1, 3, 5):  
- ✅ **Test de ancho de banda**: Verifica respuesta en frecuencia  
- ✅ **Test de slew rate**: Evalúa velocidad de cambio máxima
- ✅ **Test de distorsión**: Detecta no-linealidades del sistema

### **Rangos de voltaje**:
- **1V-4V**: Test de rango dinámico parcial
- **0V-5V**: Test de rango completo del sistema

## 📊 **Información de Debug**

El monitor serie muestra cada 5 segundos:
```
Estado actual: 2 - Cuadrada 2Hz 1V-4V | Cambio en: 18 segundos
```

## 🔧 **Modificaciones Posibles**

### **Cambiar intervalos de tiempo:**
```cpp
const uint32_t INTERVALO_CAMBIO = 15000;  // 15 segundos en lugar de 30
```

### **Agregar nuevo estado:**
```cpp
const ConfigEstado configuraciones[7] = {
  // ... estados existentes ...
  {2, 1, 32, 31, "Senoidal 1kHz 2.5V±1.25V"}  // Nuevo estado
};
```

### **Cambiar frecuencias:**
```cpp
const uint16_t incrementos_freq[3] = {
  1,    // 2Hz
  20,   // 300Hz  
  64    // 960Hz (nuevo)
};
```

## ⚠️ **Limitaciones**

1. **Frecuencias aproximadas**: Los valores son aproximados debido a la discretización
2. **Resolución**: 6 bits = solo 64 niveles de voltaje
3. **Impedancia de salida**: Depende de los resistores del DAC R2R
4. **Jitter**: Posible variación en tiempo entre Arduino Uno (no atómico)

## 🎓 **Aplicaciones Educativas**

- **Estudio de formas de onda básicas**
- **Análisis de respuesta en frecuencia**  
- **Test de sistemas de adquisición**
- **Calibración de instrumentos**
- **Demostración de conceptos DSP**

---

**Nota**: Este generador es ideal para usar junto con el **sistema DSP.ino + SerialPlotter** para testing completo de la cadena de procesamiento digital de señales.