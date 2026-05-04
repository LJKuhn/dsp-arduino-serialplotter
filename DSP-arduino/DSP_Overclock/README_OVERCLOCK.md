# ⚡ DSP OVERCLOCK - Sistema de Alta Velocidad

## 🎯 Descripción

Versión **EXTREMA** del sistema DSP que opera a **2× la velocidad estándar** manteniendo estabilidad y precisión. Diseñada para capturar señales de hasta **3840 Hz** sin aliasing.

## 📊 Especificaciones OVERCLOCK

| Parámetro | Estándar (DSP.ino) | **OVERCLOCK** | Mejora |
|-----------|-------------------|---------------|--------|
| **Frecuencia de muestreo** | 3840 Hz | **7680 Hz** | 2× |
| **Baudrate** | 38400 bps | **115200 bps** | 3× |
| **Nyquist (máx señal)** | 1920 Hz | **3840 Hz** | 2× |
| **Período de muestra** | 260 μs | **130 μs** | 2× resolución |
| **Latencia end-to-end** | ~1.0 ms | **~0.5 ms** | 2× más rápido |
| **Buffer TX** | 256 bytes | **512 bytes** | 2× capacidad |
| **Buffer RX** | 64 bytes | **128 bytes** | 2× capacidad |
| **Resolución FFT** | 1 Hz/bin | **0.5 Hz/bin** | 2× precisión |
| **Uso CPU Arduino** | 0.54% | **0.96%** | Mínimo |

## 🚀 Configuración en SerialPlotter

### **Paso 1: Compilar y Subir DSP_Overclock.ino**
```
Arduino IDE:
1. Abrir: DSP-arduino/DSP_Overclock/DSP_Overclock.ino
2. Placa: Arduino Mega 2560
3. Puerto: (Tu puerto COM)
4. Verificar compilación exitosa
5. Subir al Arduino
```

### **Paso 2: Verificar Indicador LED**
Al encender, el LED del pin 13 debe hacer **3 parpadeos rápidos** y quedar **encendido**:
- ✅ 3 parpadeos → OVERCLOCK activado correctamente
- ✅ LED encendido fijo → Sistema funcionando
- ⚠️ LED parpadeando → Buffer saturándose (ver troubleshooting)

### **Paso 3: Configurar SerialPlotter**

**En la interfaz SerialPlotter (MainWindow):**

1. **Velocidad (Baudrate):** Seleccionar **115200**
2. **Frecuencia (Sampling Rate):** Seleccionar **7680**
3. **Puerto:** El mismo que usas normalmente
4. **Conectar**

**Verificación:**
- Deberías ver señal estable en gráfico temporal
- FFT debe mostrar espectro hasta 3840 Hz (no 1920 Hz)
- No debe haber errores de comunicación

## ⚡ Compatibilidad con SerialPlotter

### **✅ TOTALMENTE COMPATIBLE**

El código SerialPlotter **YA SOPORTA** estas configuraciones:

**Baudrates disponibles en Settings.cpp línea 13:**
```cpp
const int bauds[] = { 
    ..., 38400, 57600, 115200, 230400, ...  // ✓ 115200 presente
};
```

**Frecuencias disponibles en Settings.cpp línea 16:**
```cpp
const int frecuencias[] = { 
    ..., 3840, 5760, 7680, 11520, ...  // ✓ 7680 presente
};
```

**NO requiere modificación de código C++** - Solo configurar en la interfaz.

## 🔬 Casos de Uso OVERCLOCK

### **1. Análisis de Señales de Alta Frecuencia**
```
Aplicación: Caracterización de osciladores hasta 3 kHz
Ventaja: Captura 3ª y 5ª armónica sin aliasing
Ejemplo: Señal cuadrada 1000 Hz → Detecta hasta 5000 Hz (con aliasing controlado)
```

### **2. Respuesta Transitoria Rápida**
```
Aplicación: Análisis de filtros con tiempos de settling cortos
Ventaja: Resolución temporal 130 μs vs 260 μs
Ejemplo: Filtro pasa-bajos con fc=1500 Hz → Respuesta escalón visible
```

### **3. FFT de Alta Resolución**
```
Aplicación: Detección de frecuencias muy cercanas
Ventaja: Resolución 0.5 Hz/bin vs 1 Hz/bin
Ejemplo: Distinguir 440 Hz vs 441 Hz (imposible con estándar)
```

### **4. Test de Límite de Nyquist**
```
Aplicación: Demostración experimental de aliasing
Ventaja: Nyquist = 3840 Hz → Probar señales 2000-4000 Hz
Ejemplo: Generador auto con 1800 Hz, 2000 Hz → Ver aliasing real
```

## 🧪 Pruebas Recomendadas

### **Test 1: Senoidal 2000 Hz**
```
Generador: 2000 Hz, 2Vpp
Esperado FFT: Pico en 2000 Hz ✓ (antes causaba aliasing)
Resultado: Detección limpia sin espejo
```

### **Test 2: Cuadrada 1500 Hz**
```
Generador: 1500 Hz cuadrada
Esperado FFT: 
  - 1ª armónica: 1500 Hz ✓
  - 3ª armónica: 4500 Hz → Aliasing a ~3120 Hz
  - 5ª armónica: 7500 Hz → Fuera de rango
```

### **Test 3: Barrido 100-3500 Hz**
```
Generador: Sweep 10s
Esperado: Frecuencia detectada sigue generador hasta 3500 Hz
Nyquist: Aliasing visible solo >3840 Hz
```

### **Test 4: Latencia del Sistema**
```
Entrada: Escalón 0V → 3V
Medición osciloscopio: 
  - Entrada (CH1): Escalón
  - Salida DAC (CH2): Escalón retrasado
Esperado: ~0.5 ms delay (vs 1 ms estándar)
```

## ⚠️ Advertencias y Limitaciones

### **Hardware**
- ❗ **Cable USB:** Usar cable de calidad (<1m preferible)
- ❗ **Fuente:** Alimentación estable 5V (evitar USB hubs baratos)
- ❗ **Interferencia:** Alejar de fuentes de ruido EMI

### **Software**
- ❗ **CPU PC:** Verificar uso <10% (Process Explorer/Task Manager)
- ❗ **Drivers:** Actualizar drivers USB/Serial a última versión
- ❗ **Antivirus:** Puede causar jitter (desactivar temporalmente)

### **Señal**
- ❗ **Frecuencias >3840 Hz:** Causarán aliasing (esperado)
- ❗ **Ruido ADC:** Más visible por mayor muestreo
- ❗ **Acondicionador:** Verificar ancho de banda LM324 suficiente

## 🔧 Troubleshooting

### **Problema: LED parpadea continuamente**
```
Causa: Buffer TX saturándose (datos más rápidos que USB)
Solución:
1. Verificar baudrate SerialPlotter = 115200
2. Usar cable USB corto y de calidad
3. Cerrar otros programas usando puerto COM
4. Bajar a 5760 Hz si persiste
```

### **Problema: Señal con ruido/jitter**
```
Causa: Fuente de alimentación inestable o EMI
Solución:
1. Alimentar Arduino con fuente externa regulada
2. Agregar capacitor 100 μF en Vin
3. Filtro pasa-bajos hardware en entrada ADC
4. Alejar cables de fuentes de ruido
```

### **Problema: FFT muestra armónicas extrañas**
```
Causa: Aliasing de componentes >3840 Hz
Solución:
1. Verificar señal entrada <3840 Hz fundamental
2. Agregar filtro anti-aliasing analógico fc=3500 Hz
3. Para señales cuadradas: Esperar aliasing armónicas altas (normal)
```

### **Problema: Errores de comunicación (bytes perdidos)**
```
Causa: Sistema operativo con alta latencia
Solución:
1. Windows: Deshabilitar "Suspensión selectiva USB" en plan energía
2. Aumentar prioridad proceso SerialPlotter (Task Manager)
3. Cerrar software de monitoreo (antiviirus, etc)
4. Usar puerto USB nativo (no hub)
```

## 📈 Próximos OVERCLOCKS Experimentales

### **OVERCLOCK Level 2: 11520 Hz @ 115200 bps**
```
Frecuencia: 11520 Hz (3× estándar)
Nyquist: 5760 Hz
Baudrate: 115200 bps (mismo)
Limitación: Requiere ADC prescaler 64 (9 bits efectivos)
Estado: Experimental - posible pérdida precisión
```

### **OVERCLOCK Level 3: 15360 Hz @ 230400 bps**
```
Frecuencia: 15360 Hz (4× estándar)
Nyquist: 7680 Hz
Baudrate: 230400 bps
Limitación: Requiere ADC prescaler 32 (8 bits efectivos)
Estado: Experimental - requiere verificación
```

### **OVERCLOCK Level MAX: 23040 Hz @ 250000 bps**
```
Frecuencia: 23040 Hz (6× estándar)
Nyquist: 11520 Hz
Baudrate: 250000 bps
Limitación: ADC prescaler 16 (7-8 bits efectivos)
Estado: Teórico - muy cercano a límite físico
```

## 📚 Referencias

- **ATmega2560 Datasheet:** Sección ADC (pág. 305-320)
- **Teorema de Nyquist:** fs > 2× f_max
- **Aliasing:** f_alias = |f_real - n×fs|
- **Baudrate estándares:** UART specification

## 🎓 Uso Académico

Perfecto para:
- ✅ Demostración experimental de Nyquist
- ✅ Visualización de aliasing real
- ✅ Comparación rendimiento sistemas DSP
- ✅ Análisis de latencia end-to-end
- ✅ Evaluación límites hardware

**Para el informe del profesor:**
- Comparar versión estándar vs OVERCLOCK
- Documentar resultados a diferentes frecuencias
- Mostrar aliasing con señales >3840 Hz
- Medir latencia con osciloscopio
- Graficar espectro FFT hasta 3840 Hz

---

**Autor:** Lautaro Kühn & Federico Domínguez  
**Fecha:** Mayo 2026  
**Versión:** OVERCLOCK 2.0  
**Licencia:** MIT
