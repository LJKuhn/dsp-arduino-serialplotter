# Generador Automático de Formas de Onda - Arduino Uno

## 🎯 **Descripción**

Sistema avanzado que genera **18 tipos diferentes de señales** de forma automática, cambiando cada **15 segundos** en un ciclo repetitivo. Cada señal tiene **frecuencia y rango de voltaje específicos** para pruebas completas de sistemas DSP, incluyendo **test de aliasing** con frecuencias superiores al límite de Nyquist (1920 Hz).

## 📊 **Ciclo de Señales (18 estados × 15s = 4.5 minutos)**

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
| **12** | Senoidal   | **500 Hz** | **0V - 5V**   | 0V     | 5V       | **Test Nyquist** (< 1920 Hz) |
| **13** | Senoidal   | **900 Hz** | **0V - 5V**   | 0V     | 5V       | **Test Nyquist** (< 1920 Hz) |
| **14** | Senoidal   | **1200 Hz** | **0V - 5V**  | 0V     | 5V       | **Test Nyquist** (< 1920 Hz) |
| **15** | Senoidal   | **1500 Hz** | **0V - 5V**  | 0V     | 5V       | **Test Nyquist** (< 1920 Hz) |
| **16** | Senoidal   | **1800 Hz** | **0V - 5V**  | 0V     | 5V       | ⚠️ **Test aliasing** (> 1920 Hz) |
| **17** | Senoidal   | **2000 Hz** | **0V - 5V**  | 0V     | 5V       | ⚠️ **Test aliasing** (> 1920 Hz) |

**Después del estado 17, vuelve automáticamente al estado 0.**

### 🔬 **Frecuencias para Test de Aliasing**

Las frecuencias **1800 Hz y 2000 Hz** superan el límite de Nyquist (1920 Hz) del sistema receptor que muestrea a 3840 Hz. Estas señales producirán **aliasing** visible:

- **1800 Hz real** → Detectado como **~2040 Hz** (espejo: 3840 - 1800)
- **2000 Hz real** → Detectado como **~1840 Hz** (espejo: 3840 - 2000)

Esto permite **verificar experimentalmente** el fenómeno de aliasing documentado en el informe.

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

### **Señales de 2 Hz** (Estados 0, 4, 8):
- ✅ **Visualización fácil**: Se puede ver forma completa en osciloscopio
- ✅ **Calibración**: Permite ajustar ganancia y offset de sistemas
- ✅ **Debugging**: Ideal para verificar funcionamiento básico

### **Señales de 300 Hz** (Estados 3, 7, 11):  
- ✅ **Test de ancho de banda**: Verifica respuesta en frecuencia  
- ✅ **Test de slew rate**: Evalúa velocidad de cambio máxima
- ✅ **Test de distorsión**: Detecta no-linealidades del sistema

### **Señales de 500-1500 Hz** (Estados 12-15):
- ✅ **Test de límite Nyquist**: Frecuencias bajo el límite de 1920 Hz
- ✅ **Verificación FFT**: Deben aparecer en bins correctos
- ✅ **Respuesta en frecuencia**: Evaluar filtros pasa-bajos/altos

### **Señales de 1800-2000 Hz** (Estados 16-17):
- ⚠️ **Demostración de aliasing**: Superan límite de Nyquist (1920 Hz)
- ⚠️ **Validación experimental**: Verificar teoría de muestreo
- ⚠️ **Frecuencias esperadas en receptor**:
  - 1800 Hz → Detectado como ~2040 Hz
  - 2000 Hz → Detectado como ~1840 Hz

### **Rangos de voltaje**:
- **1V-4V**: Test de rango dinámico parcial
- **0V-5V**: Test de rango completo del sistema

## 📊 **Información de Debug**

El monitor serie muestra cada 5 segundos:
```
Estado actual: 16 - Senoidal 1800Hz 0V-5V | Cambio en: 8 segundos
```

Al cambiar de estado:
```
Cambiando a estado 17: Senoidal 2000Hz 0V-5V
```

**Nota:** Los estados 16 y 17 mostrarán aliasing en el receptor (frecuencias > 1920 Hz Nyquist).

## 🔧 **Modificaciones Posibles**

### **Cambiar intervalos de tiempo:**
```cpp
const uint32_t INTERVALO_CAMBIO = 15000;  // 15 segundos en lugar de 30
```

### **Agregar nuevo estado:**
```cpp
const ConfigEstado configuraciones[19] = {
  // ... 18 estados existentes ...
  {2, 10, 32, 31, "Senoidal 440Hz 2.5V±1.25V"}  // Nuevo estado (La musical)
};
```
Nota: También necesitas agregar el incremento correspondiente en `incrementos_freq[]` y divisor en `divisores_freq[]`.

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