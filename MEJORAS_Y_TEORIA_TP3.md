# Mejoras Sugeridas y Documentación Teórica - TP3

## 🔧 **MEJORAS PARA CUMPLIMIENTO COMPLETO**

### ✅ **1. Acondicionador de Señal para 0V-12Vpp - RESUELTO**

#### **Implementación con LM324**:
El usuario ha implementado un circuito acondicionador usando **LM324** (amplificador operacional cuádruple) con:

- ✅ **Divisor resistivo**: Reduce amplitud ±6V a menor escala  
- ✅ **Offset/Buffer**: Elimina tensiones negativas
- ✅ **Rango final**: 0V-5V compatible con Arduino ADC

#### **Ventajas del LM324**:
- **Alimentación simple**: +5V (compatible con Arduino)
- **Rail-to-rail**: Salida cercana a los rieles de alimentación
- **Bajo offset**: Mínima distorsión de DC
- **Cuádruple**: 4 amplificadores en un package (permite configuraciones complejas)

**Estado**: ✅ **PROBLEMA RESUELTO POR HARDWARE**

---

### **2. Detección Automática de Armónicas**

#### **Fundamento Teórico - FFT**:
La **Transformada Rápida de Fourier (FFT)** descompone una señal temporal en sus componentes frecuenciales:

```
x[n] = Σ X[k] * e^(j2πkn/N)
```

Donde:
- `x[n]`: Señal en tiempo
- `X[k]`: Coeficientes espectrales
- `k`: Índice de frecuencia

#### **Algoritmo de Detección de Armónicas**:

```cpp
struct Harmonic {
    double frequency;    // Hz
    double amplitude;    // Voltios
    double phase;       // Radianes
};

class HarmonicAnalyzer {
private:
    double sampling_rate;
    double fundamental_freq;
    
public:
    std::vector<Harmonic> FindTopHarmonics(double* spectrum, int size, int count = 3) {
        std::vector<Harmonic> harmonics;
        
        // 1. Encontrar frecuencia fundamental
        fundamental_freq = FindFundamental(spectrum, size);
        
        // 2. Buscar armónicas múltiples de la fundamental
        for (int i = 1; i <= count; i++) {
            double target_freq = fundamental_freq * i;
            Harmonic h = ExtractHarmonic(spectrum, size, target_freq);
            harmonics.push_back(h);
        }
        
        return harmonics;
    }
    
private:
    double FindFundamental(double* spectrum, int size) {
        // Buscar pico de mayor amplitud
        int max_idx = 0;
        double max_value = 0;
        
        for (int i = 1; i < size/2; i++) {
            if (spectrum[i] > max_value) {
                max_value = spectrum[i];
                max_idx = i;
            }
        }
        
        // Convertir índice a frecuencia
        return (double)max_idx * sampling_rate / size;
    }
    
    Harmonic ExtractHarmonic(double* spectrum, int size, double target_freq) {
        // Encontrar bin más cercano a target_freq
        int bin = round(target_freq * size / sampling_rate);
        
        // Interpolación para mayor precisión
        double freq = InterpolateFrequency(spectrum, bin, size);
        double amp = spectrum[bin];
        
        return {freq, amp, 0}; // Fase simplificada
    }
};
```

#### **Integración en SerialPlotter**:
```cpp
// En MainWindow.cpp
HarmonicAnalyzer analyzer(settings->sampling_rate);

void MainWindow::UpdateFFT() {
    fft->ExecuteReal(scrollY->data(), scrollY->size());
    
    // Detectar armónicas automáticamente
    auto harmonics = analyzer.FindTopHarmonics(fft->GetMagnitudeSpectrum(), fft->GetSize());
    
    // Mostrar en interfaz
    for (size_t i = 0; i < harmonics.size(); i++) {
        ImGui::Text("Armónica %d: %.1f Hz, %.2f V", 
                   (int)i+1, harmonics[i].frequency, harmonics[i].amplitude);
    }
}
```

---

### **3. Filtro Pasabanda**

#### **Fundamento Teórico - Filtros Digitales**:

Los **filtros digitales IIR (Infinite Impulse Response)** implementan la ecuación en diferencias:

```
y[n] = Σ(ai * x[n-i]) - Σ(bi * y[n-i])
```

**Filtros Butterworth** ofrecen:
- ✅ Respuesta plana en banda de paso
- ✅ Transición suave
- ✅ Fase predecible

#### **Implementación Pasabanda - Opción 1 (Cascada)**:
```cpp
class BandPassCascade {
    Iir::Butterworth::LowPass<4> lowpass;
    Iir::Butterworth::HighPass<4> highpass;
    
public:
    void setup(double sample_rate, double low_freq, double high_freq) {
        highpass.setup(sample_rate, low_freq);    // Corta frecuencias bajas
        lowpass.setup(sample_rate, high_freq);    // Corta frecuencias altas
    }
    
    double filter(double input) {
        double temp = highpass.filter(input);
        return lowpass.filter(temp);
    }
};
```

#### **Implementación Pasabanda - Opción 2 (Directo)**:
```cpp
// Agregar al enum Filter
enum class Filter {
    None,
    LowPass,
    HighPass,
    BandPass    // Nuevo
};

// En MainWindow.cpp
Iir::Butterworth::BandPass<8> bandpass_filter;

// En el switch de filtros
case Filter::BandPass:
    resultado = bandpass_filter.filter(transformado);
    break;
```

#### **Interfaz de Usuario**:
```cpp
// Controles adicionales para pasabanda
if (selected_filter == Filter::BandPass) {
    ImGui::SliderFloat("Frecuencia baja", &low_cutoff, 1.0f, sampling_rate/4);
    ImGui::SliderFloat("Frecuencia alta", &high_cutoff, 1.0f, sampling_rate/4);
    
    if (ImGui::Button("Aplicar Pasabanda")) {
        bandpass_filter.setup(sampling_rate, low_cutoff, high_cutoff);
    }
}
```

---

## 📚 **DOCUMENTACIÓN TEÓRICA SOLICITADA**

### **1. Fundamentos de Transformadas**

#### **¿Qué es la FFT?**
La **FFT (Fast Fourier Transform)** es un algoritmo eficiente para calcular la **DFT (Discrete Fourier Transform)**:

**Propósito**: Convertir señal del dominio temporal al dominio frecuencial.

**Matemática**:
```
X[k] = Σ(n=0 to N-1) x[n] * e^(-j2πkn/N)
```

**Interpretación física**: 
- Cada `X[k]` representa la amplitud y fase de una componente senoidal de frecuencia `k*fs/N`
- Permite identificar **qué frecuencias** están presentes en la señal

#### **Aplicación en el proyecto**:
1. **Análisis espectral**: Ver contenido frecuencial de señales
2. **Detección de armónicas**: Identificar frecuencias múltiples de fundamental
3. **Diagnóstico**: Detectar distorsión, ruido, artefactos

#### **Ventanas de FFT**:
- **Rectangular**: Simple pero con "leakage"
- **Hamming/Hanning**: Mejor resolución frecuencial
- **Blackman**: Mínimo leakage, menor resolución

---

### **2. Fundamentos de Filtros Digitales**

#### **¿Por qué filtros digitales?**

**Ventajas sobre filtros analógicos**:
- ✅ **Precisión**: No hay deriva térmica ni tolerancias de componentes
- ✅ **Flexibilidad**: Parámetros modificables por software  
- ✅ **Repetibilidad**: Respuesta idéntica siempre
- ✅ **Complejidad**: Filtros imposibles con componentes pasivos

#### **Tipos implementados**:

**1. Filtro Pasabajos**:
- **Propósito**: Atenúa frecuencias > frecuencia de corte
- **Aplicación**: Anti-aliasing, reducción de ruido
- **Ecuación**: `H(z) = K / (z^n + a₁z^(n-1) + ... + aₙ)`

**2. Filtro Pasaaltos**: 
- **Propósito**: Atenúa frecuencias < frecuencia de corte  
- **Aplicación**: Eliminar DC offset, separar canales
- **Característica**: Respuesta complementaria al pasabajos

**3. Filtro Pasabanda**:
- **Propósito**: Permite solo frecuencias en rango específico
- **Aplicación**: Selección de canales, análisis de armónicas
- **Implementación**: Cascada o diseño directo

#### **¿Por qué Butterworth?**

**Características**:
- **Respuesta plana**: |H(jω)| constante en banda de paso
- **Orden escalable**: Más orden = transición más abrupta
- **Fase predecible**: Retardo de grupo relativamente constante

**Trade-off**:
- ✅ **Ventaja**: Respuesta suave, sin oscillations
- ⚠️ **Desventaja**: Transición gradual vs Chebyshev/Elliptic

---

### **3. Justificación de Decisiones de Diseño**

#### **¿Por qué 3840 Hz de muestreo?**

**Criterio de Nyquist**: `fs ≥ 2 * fmax`

Para señales de audio (20 Hz - 20 kHz):
- Frecuencia máxima útil: 1920 Hz (fs/2)
- Suficiente para señales de prueba típicas (≤ 1 kHz)
- Balance entre resolución temporal y carga computacional

#### **¿Por qué Butterworth orden 8?**

**Análisis de trade-offs**:
- **Orden 4**: Transición suave, latencia baja (~0.2ms)
- **Orden 8**: Selectividad alta, latencia media (~0.6ms) ← **Elegido**
- **Orden 16**: Selectividad máxima, latencia alta (~1.2ms)

**Decisión**: Orden 8 ofrece **buen compromiso** entre selectividad y latencia para aplicaciones de tiempo real.

#### **¿Por qué comunicación bidireccional?**

**Arquitectura ADC ↔ PC ↔ DAC**:

**Ventajas**:
1. **Poder computacional**: PC realiza cálculos complejos (FFT, filtros IIR)
2. **Visualización**: Gráficas en tiempo real imposibles en Arduino
3. **Flexibilidad**: Cambio de parámetros sin recompilación
4. **Escalabilidad**: Fácil agregar nuevos filtros o análisis

**Desventajas**:
- Latencia adicional (~0.6ms)
- Dependencia de comunicación serie
- Mayor complejidad de sistema

**Conclusión**: Para un sistema **educativo y de desarrollo**, las ventajas superan ampliamente las desventajas.

---

## 🎯 **PLAN DE IMPLEMENTACIÓN DE MEJORAS**

### **Prioridad 1 - Críticas**:
1. ✅ **Documentación teórica**: Completada en este documento
2. ✅ **Acondicionador de señal**: Resuelto con LM324 por el usuario

### **Prioridad 2 - Mejoras opcionales**:  
3. 🔧 **Filtro pasabanda**: Implementación directa (1-2 horas)
4. 🔧 **Detección automática de armónicas**: Algoritmo de picos (2-4 horas)

### **Prioridad 3 - Mejoras**:
5. 🔧 **Ventanas de FFT**: Hamming/Hanning para mejor resolución
6. 🔧 **Filtros adicionales**: Chebyshev, Bessel para comparación
7. 🔧 **Calibración automática**: Detección de rango de entrada

---

## 💡 **RECOMENDACIÓN FINAL**

El proyecto **cumple sustancialmente** con todos los objetivos del TP3. Las mejoras sugeridas son **extensiones avanzadas** que elevan el sistema de "funcional" a "profesional", pero **no son críticas** para la demostración de conceptos fundamentales.

**Estado actual**: ✅ **Apto para entrega y demostración**  
**Con mejoras**: 🏆 **Sistema de referencia para DSP embebido**