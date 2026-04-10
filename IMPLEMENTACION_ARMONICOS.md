# INSTRUCCIONES DE IMPLEMENTACIÓN: Detección de Armónicos en SerialPlotter

## OBJETIVO

Implementar la funcionalidad de detección automática de las 3 primeras armónicas en el análisis FFT del SerialPlotter, mostrándolas en una tabla en la interfaz de usuario.

## CONTEXTO

- **Proyecto**: DSP Arduino SerialPlotter
- **Ubicación del código**: `SerialPlotter/src/`
- **Sistema actual**: Solo detecta frecuencia dominante y offset DC
- **Sistema objetivo**: Detectar y mostrar 1ª, 2ª y 3ª armónica con frecuencia y amplitud
- **NO se modifica**: Código de Arduino (permanece sin cambios)

## ARCHIVOS A MODIFICAR

1. `SerialPlotter/src/FFT.h` - Agregar estructura de datos y métodos públicos
2. `SerialPlotter/src/FFT.cpp` - Implementar lógica de detección de armónicos
3. `SerialPlotter/src/MainWindow.cpp` - Modificar interfaz para mostrar armónicos

---

## MODIFICACIÓN 1: FFT.h

### Ubicación del archivo
```
SerialPlotter/src/FFT.h
```

### Cambio 1.1: Agregar estructura Harmonic

**Ubicación**: Después de la línea 16 (`#include <vector>`)

**Código a INSERTAR**:
```cpp
// Estructura para almacenar información de armónicas detectadas
struct Harmonic {
    double frequency;   // Frecuencia en Hz
    double amplitude;   // Amplitud en Voltios
    int bin_index;      // Índice del bin en el espectro FFT
};
```

### Cambio 1.2: Agregar miembro privado en clase FFT

**Ubicación**: Dentro de la clase FFT, en la sección `private`, después de la línea 28 (`int n_frequency = 0;`)

**Código a INSERTAR**:
```cpp
	std::vector<Harmonic> detected_harmonics;  // Almacena las armónicas detectadas
```

### Cambio 1.3: Agregar métodos públicos en clase FFT

**Ubicación**: Dentro de la clase FFT, en la sección `public`, después del método `Frequency()` (línea 54)

**Código a INSERTAR**:
```cpp
	
	// Acceso al espectro completo de amplitudes
	const std::vector<double>& GetAmplitudes() const { return amplitudes; }
	int GetAmplitudesSize() const { return amplitudes_size; }
	
	// Detección de armónicas (múltiplos de la frecuencia fundamental)
	// sampling_frequency: frecuencia de muestreo en Hz
	// count: número de armónicas a detectar (por defecto 3)
	// Retorna: vector con información de cada armónica detectada
	std::vector<Harmonic> FindHarmonics(double sampling_frequency, int count = 3);
	
	// Obtener amplitud de un bin específico del espectro
	double GetAmplitudeAt(int bin) const;
```

---

## MODIFICACIÓN 2: FFT.cpp

### Ubicación del archivo
```
SerialPlotter/src/FFT.cpp
```

### Cambio 2.1: Agregar implementación de FindHarmonics()

**Ubicación**: Al final del archivo, después del método `Frequency()`

**Código a AGREGAR**:
```cpp

std::vector<Harmonic> FFT::FindHarmonics(double sampling_frequency, int count) {
    detected_harmonics.clear();
    
    // Validar que hay datos disponibles
    if (amplitudes.empty() || n_frequency == 0)
        return detected_harmonics;
    
    // Calcular frecuencia fundamental (ya detectada en Compute())
    double fundamental_freq = n_frequency * sampling_frequency / samples_size;
    
    // Buscar las 'count' primeras armónicas (1×f₀, 2×f₀, 3×f₀, ...)
    for (int n = 1; n <= count; n++) {
        // Calcular frecuencia objetivo de esta armónica
        double target_freq = fundamental_freq * n;
        
        // Convertir frecuencia objetivo a índice de bin en el espectro
        int target_bin = static_cast<int>(std::round(target_freq * samples_size / sampling_frequency));
        
        // Verificar que el bin está dentro del rango válido
        if (target_bin >= amplitudes_size)
            break;  // Frecuencia fuera del rango (> Nyquist)
        
        // Buscar el pico local alrededor del bin objetivo
        // Rango de búsqueda: ±3 bins para tolerar variaciones
        int search_start = std::max(1, target_bin - 3);
        int search_end = std::min(amplitudes_size - 1, target_bin + 3);
        
        int peak_bin = target_bin;
        double peak_amplitude = amplitudes[target_bin];
        
        // Encontrar el bin con máxima amplitud en la ventana de búsqueda
        for (int i = search_start; i <= search_end; i++) {
            if (amplitudes[i] > peak_amplitude) {
                peak_amplitude = amplitudes[i];
                peak_bin = i;
            }
        }
        
        // Calcular frecuencia exacta del pico encontrado
        double exact_freq = peak_bin * sampling_frequency / samples_size;
        
        // Crear y almacenar información de la armónica detectada
        Harmonic h;
        h.frequency = exact_freq;
        h.amplitude = peak_amplitude;
        h.bin_index = peak_bin;
        
        detected_harmonics.push_back(h);
    }
    
    return detected_harmonics;
}

double FFT::GetAmplitudeAt(int bin) const {
    if (bin >= 0 && bin < amplitudes_size)
        return amplitudes[bin];
    return 0.0;
}
```

---

## MODIFICACIÓN 3: MainWindow.cpp

### Ubicación del archivo
```
SerialPlotter/src/MainWindow.cpp
```

### Cambio 3.1: Reemplazar visualización de frecuencia actual

**Ubicación**: Aproximadamente línea 828-831, dentro de la función `Draw()`

**BUSCAR este código**:
```cpp
            // Mostrar información de frecuencia dominante y offset DC
            if (scrollY && scrollY->count() > 0) {
                ImGui::Text("Frecuencia: %s\tDesplazamiento %s",
                            MetricFormatter(fft->Frequency(settings->sampling_rate), "Hz").data(),
                            MetricFormatter(fft->Offset(), "V").data()
                );
            }
```

**REEMPLAZAR con**:
```cpp
            // Mostrar información de frecuencia dominante y offset DC
            if (scrollY && scrollY->count() > 0) {
                ImGui::Text("Frecuencia dominante: %s\tOffset DC: %s",
                            MetricFormatter(fft->Frequency(settings->sampling_rate), "Hz").data(),
                            MetricFormatter(fft->Offset(), "V").data()
                );
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Text("ARMÓNICAS DETECTADAS:");
                ImGui::Spacing();
                
                // Detectar las 3 primeras armónicas
                auto harmonics = fft->FindHarmonics(settings->sampling_rate, 3);
                
                // Mostrar tabla con formato estructurado
                if (!harmonics.empty()) {
                    // Configurar tabla de 3 columnas
                    ImGui::Columns(3, "harmonics_table");
                    ImGui::Separator();
                    
                    // Encabezados de tabla
                    ImGui::Text("Armónica"); ImGui::NextColumn();
                    ImGui::Text("Frecuencia"); ImGui::NextColumn();
                    ImGui::Text("Amplitud"); ImGui::NextColumn();
                    ImGui::Separator();
                    
                    // Datos de cada armónica detectada
                    for (size_t i = 0; i < harmonics.size(); i++) {
                        // Columna 1: Número de armónica (1ª, 2ª, 3ª)
                        ImGui::Text("%dª", static_cast<int>(i + 1)); 
                        ImGui::NextColumn();
                        
                        // Columna 2: Frecuencia en Hz con formato métrico
                        ImGui::Text("%s", 
                                   MetricFormatter(harmonics[i].frequency, "Hz").data());
                        ImGui::NextColumn();
                        
                        // Columna 3: Amplitud en Voltios con formato métrico
                        ImGui::Text("%s", 
                                   MetricFormatter(harmonics[i].amplitude, "V").data());
                        ImGui::NextColumn();
                    }
                    
                    // Volver a 1 columna
                    ImGui::Columns(1);
                    ImGui::Separator();
                    
                    // OPCIONAL: Calcular y mostrar THD (Total Harmonic Distortion)
                    if (harmonics.size() >= 3) {
                        double fundamental = harmonics[0].amplitude;
                        double thd_sum = 0;
                        
                        // THD = sqrt(A₂² + A₃² + ...) / A₁
                        for (size_t i = 1; i < harmonics.size(); i++) {
                            thd_sum += harmonics[i].amplitude * harmonics[i].amplitude;
                        }
                        
                        double thd = (fundamental > 0) ? 
                                     (std::sqrt(thd_sum) / fundamental * 100.0) : 0;
                        
                        ImGui::Text("Distorsión Armónica Total (THD): %.2f%%", thd);
                    }
                } else {
                    // Mensaje cuando no hay datos suficientes
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
                                      "  No hay datos suficientes para análisis");
                }
            }
```

---

## INSTRUCCIONES DE COMPILACIÓN

Después de realizar las modificaciones, recompilar el proyecto SerialPlotter:

### Visual Studio 2022 (Windows)
```powershell
# Abrir solución
start SerialPlotter.sln

# En Visual Studio:
# Build > Rebuild Solution
# O presionar Ctrl+Shift+B
```

### CMake (Linux/Mac)
```bash
cd SerialPlotter
mkdir -p build
cd build
cmake ..
make
```

---

## VALIDACIÓN

### Verificar cambios aplicados correctamente

1. **FFT.h**: Debe tener estructura `Harmonic` y 3 métodos nuevos
2. **FFT.cpp**: Debe tener implementación de `FindHarmonics()` (~60 líneas)
3. **MainWindow.cpp**: La sección de análisis debe mostrar tabla de armónicos

### Prueba funcional

1. Compilar sin errores
2. Ejecutar SerialPlotter
3. Conectar Arduino
4. Generar señal senoidal (ejemplo: 440 Hz)
5. Verificar que aparece tabla:
   ```
   ARMÓNICAS DETECTADAS:
   Armónica | Frecuencia | Amplitud
   ─────────┼────────────┼──────────
      1ª    |   440 Hz   |  1.00 V
      2ª    |   880 Hz   |  0.05 V  (ruido)
      3ª    |  1320 Hz   |  0.03 V  (ruido)
   ```

6. Con señal cuadrada (100 Hz) debe mostrar armónicas impares:
   ```
   ARMÓNICAS DETECTADAS:
   Armónica | Frecuencia | Amplitud
   ─────────┼────────────┼──────────
      1ª    |   100 Hz   |  1.27 V
      2ª    |   200 Hz   |  0.02 V  (casi nula)
      3ª    |   300 Hz   |  0.42 V  (1/3 de fundamental)
   ```

---

## TROUBLESHOOTING

### Error de compilación: "Harmonic was not declared"
**Solución**: Verificar que la estructura `Harmonic` está definida ANTES de la clase `FFT` en FFT.h

### Error de compilación: "std::round is not a member of std"
**Solución**: Agregar `#include <cmath>` al inicio de FFT.cpp (ya debería estar)

### Error de linkeo: "undefined reference to FFT::FindHarmonics"
**Solución**: Verificar que FFT.cpp contiene la implementación completa de `FindHarmonics()`

### No aparece tabla de armónicos en interfaz
**Solución**: 
1. Verificar que hay señal conectada
2. Verificar que la sección "Análisis" está expandida
3. Revisar que el reemplazo en MainWindow.cpp se hizo correctamente

### Las armónicas muestran valores incorrectos
**Causas posibles**:
- Frecuencia fundamental mal detectada (revisar FFT::Compute())
- Rango de búsqueda muy estrecho (ajustar ±3 bins si es necesario)
- Señal muy ruidosa (armónicas se confunden con ruido)

---

## RESULTADO ESPERADO

### Antes (estado actual):
```
═══════════════════════════════════════
Frecuencia: 440.2 Hz    Desplazamiento: 2.50 V
═══════════════════════════════════════
```

### Después (con implementación):
```
═══════════════════════════════════════
Frecuencia dominante: 440.2 Hz    Offset DC: 2.50 V
───────────────────────────────────────
ARMÓNICAS DETECTADAS:

Armónica | Frecuencia | Amplitud
─────────┼────────────┼──────────
   1ª    |  440.2 Hz  |  0.950 V
   2ª    |  880.5 Hz  |  0.420 V
   3ª    | 1320.1 Hz  |  0.180 V
───────────────────────────────────────
Distorsión Armónica Total (THD): 48.32%
═══════════════════════════════════════
```

---

## ARCHIVOS MODIFICADOS - RESUMEN

| Archivo | Líneas agregadas | Líneas modificadas | Líneas eliminadas |
|---------|------------------|-------------------|-------------------|
| FFT.h | ~30 | 0 | 0 |
| FFT.cpp | ~60 | 0 | 0 |
| MainWindow.cpp | ~55 | 0 | ~7 |
| **TOTAL** | **~145** | **0** | **~7** |

---

## NOTAS PARA AGENTE DE IA

### Formato de código
- Usar **tabs** para indentación (como en el código existente)
- Mantener estilo de comentarios existente
- Respetar encoding UTF-8 con BOM

### Dependencias
- No se requieren librerías adicionales
- Usa solo STL estándar: `<vector>`, `<cmath>`, `<algorithm>`
- ImGui ya está incluido en el proyecto

### Testing
- Probar con señales sintéticas: senoidal pura, cuadrada, triangular
- Validar THD con valores teóricos conocidos
- Verificar comportamiento con señales ruidosas

### Extensiones futuras sugeridas
1. Agregar selector de cantidad de armónicos (3, 5, 7)
2. Resaltar armónicos en el gráfico espectral
3. Exportar armónicos a archivo CSV
4. Calcular SINAD, SFDR además de THD
5. Detección automática de señales no-armónicas

---

**Documento generado**: Abril 2026  
**Versión**: 1.0  
**Proyecto**: DSP Arduino SerialPlotter  
**Autor**: Sistema de documentación automática
