# GUÍA DE MEDICIONES Y PRUEBAS - SISTEMA DSP ARDUINO

Este documento describe las pruebas sugeridas para validar el funcionamiento del sistema de procesamiento digital de señales implementado. Incluye metodologías, equipamiento necesario y valores esperados/óptimos para cada prueba.

---

## 1. EQUIPAMIENTO NECESARIO

### 1.1 Equipamiento Básico (Mínimo)
- **Arduino Mega 2560** con firmware DSP cargado
- **PC** con aplicación SerialPlotter compilada
- **Cable USB** para conexión Arduino-PC
- **Fuente de alimentación DC variable** (0-5V, mínimo 500mA)
- **Multímetro digital** (precisión mínima 0.1V)
- **Protoboard** y cables de conexión

### 1.2 Equipamiento Avanzado (Recomendado)
- **Generador de funciones** (10 Hz - 10 kHz, amplitud variable)
- **Osciloscopio digital** (mínimo 2 canales, 10 MHz)
- **Fuente de referencia calibrada** para pruebas de exactitud
- **Filtros RC pasivos** para validación comparativa
- **Analizador de espectro** (opcional, para validación FFT)

---

## 2. PRUEBAS DE CARACTERIZACIÓN METROLÓGICA

### 2.1 Prueba de Precisión (Repetibilidad)

**Objetivo**: Medir la dispersión de las lecturas del sistema para una entrada constante.

**Metodología**:
1. Conectar fuente DC estable configurada a **0V** (punto medio del rango ±6V)
2. Iniciar aplicación SerialPlotter
3. Capturar **1000 muestras consecutivas**
4. Registrar valores mostrados en interfaz
5. Calcular estadísticas: media, desviación estándar, mínimo, máximo

**Valores esperados** (entrada 0V):
```
Voltaje en ADC esperado: 2.30 V ± 0.05V
Media calculada (referida a entrada): 0.00 V ± 0.05V
Desviación estándar: 40-50 mV (referida a entrada ±6V)
Rango: 150-200 mV (3-4 LSB de entrada)
```

**Criterio de aceptación**:
- Desviación estándar < 50 mV ✓ Óptimo
- Desviación estándar < 70 mV ✓ Aceptable
- Desviación estándar > 100 mV ✗ Revisar conexiones/ruido

**Posibles causas de falla**:
- Ruido en fuente de alimentación → Usar filtro capacitivo
- Conexiones flojas → Revisar soldaduras/contactos
- Interferencia electromagnética → Alejar de fuentes de ruido

---

### 2.2 Prueba de Exactitud (Precisión Absoluta)

**Objetivo**: Verificar que las lecturas del sistema coincidan con valores reales conocidos.

**Metodología**:
1. Usar multímetro calibrado como referencia
2. Aplicar voltajes conocidos en el rango **-6V a +6V**
3. Comparar lectura del sistema vs multímetro
4. Calcular error absoluto y porcentual

**Puntos de prueba sugeridos**:

| Voltaje a aplicar | Voltaje esperado en ADC | Error máximo aceptable |
|-------------------|-------------------------|------------------------|
| -6.0 V | 0.80 V | ±100 mV |
| -4.0 V | 1.30 V | ±80 mV |
| -2.0 V | 1.80 V | ±60 mV |
| 0.0 V | 2.30 V | ±50 mV |
| +2.0 V | 2.80 V | ±60 mV |
| +4.0 V | 3.30 V | ±80 mV |
| +6.0 V | 3.80 V | ±100 mV |

**Tabla para completar** (ejemplo):

| V_multímetro | V_sistema | Error_absoluto | Error_% | ✓/✗ |
|--------------|-----------|----------------|---------|-----|
| -4.000 V | ______ V | ______ mV | _____ % | |
| -2.000 V | ______ V | ______ mV | _____ % | |
| 0.000 V | ______ V | ______ mV | — | |
| +2.000 V | ______ V | ______ mV | _____ % | |
| +4.000 V | ______ V | ______ mV | _____ % | |

**Criterio de aceptación**:
- Error promedio < ±50 mV → ✓ Óptimo (±0.42% del span de 12V)
- Error promedio < ±75 mV → ✓ Aceptable (±0.63% del span)
- Error promedio > ±100 mV → ✗ Revisar calibración/acondicionador

**Nota**: El error aumenta hacia los extremos del rango debido a la no-linealidad del acondicionador LM324.

---

## 3. PRUEBAS DE ANÁLISIS FFT

### 3.1 Señal Senoidal Pura (Validación de Frecuencia y THD)

**Objetivo**: Verificar detección correcta de frecuencia fundamental y medición de THD.

**Equipamiento**: Generador de funciones

**Configuración**:
- **Frecuencia**: 440 Hz (nota musical La, fácil de verificar auditivamente)
- **Amplitud**: 1.0 Vpp (±0.5V)
- **Offset**: 2.5 V (centrado en rango ADC)
- **Forma de onda**: Senoidal pura

**Valores esperados en la aplicación**:
```
Frecuencia detectada: 440 ± 1 Hz (error < 0.25%)
Amplitud fundamental: ~0.35 V RMS (0.5 Vpk ÷ √2)
2ª armónica (880 Hz): < 0.010 V (THD < 2%)
3ª armónica (1320 Hz): < 0.005 V
Offset DC: 2.5 ± 0.1 V
```

**Criterio de aceptación**:
- Error de frecuencia < 1 Hz ✓
- THD < 3% ✓ (señal limpia del generador)
- THD < 5% ✓ Aceptable (puede haber distorsión del acondicionador)
- THD > 10% ✗ Verificar generador o circuito

**Variaciones sugeridas**:
1. Probar otras frecuencias: 100 Hz, 220 Hz, 880 Hz, 1500 Hz
2. Variar amplitud: 0.5 Vpp, 2.0 Vpp, 4.0 Vpp
3. Verificar linealidad del sistema

---

### 3.2 Señal Cuadrada (Validación de Serie de Fourier)

**Objetivo**: Verificar que el sistema detecta correctamente las armónicas impares de una onda cuadrada.

**Configuración**:
- **Frecuencia fundamental**: 100 Hz
- **Amplitud**: 2.0 Vpp (±1.0V)
- **Duty cycle**: 50%
- **Offset**: 2.5 V

**Armónicas esperadas** (teoría de serie de Fourier):

Una onda cuadrada ideal contiene solo armónicas impares con amplitudes que decaen según:
$$A_n = \frac{4A}{\pi n}$$

Donde $A$ es la amplitud de la onda cuadrada y $n$ es el número de armónica (1, 3, 5, 7...).

**Valores teóricos para A=1.0V**:

| Armónica | Frecuencia | Amplitud teórica | Amplitud esperada* |
|----------|------------|------------------|-------------------|
| 1ª | 100 Hz | 1.273 V (4/π) | 1.20-1.30 V |
| 3ª | 300 Hz | 0.424 V (4/3π) | 0.40-0.45 V |
| 5ª | 500 Hz | 0.255 V (4/5π) | 0.23-0.27 V |
| 7ª | 700 Hz | 0.182 V (4/7π) | 0.16-0.20 V |
| 9ª | 900 Hz | 0.141 V (4/9π) | 0.12-0.16 V |

*Los valores esperados tienen tolerancia del ±10% debido a limitaciones del generador y filtrado del acondicionador.

**Criterio de aceptación**:
- Armónicas impares presentes ✓
- Amplitudes dentro del ±15% del valor teórico ✓
- Armónicas pares ausentes o < 5% de fundamental ✓
- Relación 1ª:3ª ≈ 3:1 ✓

**Nota**: Si el generador tiene tiempo de subida lento, las armónicas altas estarán atenuadas.

---

### 3.3 Señal Triangular (Opcional)

**Objetivo**: Verificar detección de armónicas con atenuación mayor (decaimiento 1/n²).

**Configuración**:
- **Frecuencia fundamental**: 200 Hz
- **Amplitud**: 2.0 Vpp
- **Offset**: 2.5 V

**Armónicas esperadas** (serie de Fourier onda triangular):
$$A_n = \frac{8A}{\pi^2 n^2} \quad \text{(solo impares)}$$

| Armónica | Frecuencia | Amplitud teórica |
|----------|------------|------------------|
| 1ª | 200 Hz | 0.811 V |
| 3ª | 600 Hz | 0.090 V (1/9 de fundamental) |
| 5ª | 1000 Hz | 0.032 V (1/25 de fundamental) |

---

## 4. PRUEBAS DE FILTROS DIGITALES

### 4.1 Filtro Pasa Bajos (Atenuación de Altas Frecuencias)

**Objetivo**: Verificar que el filtro atenúa correctamente frecuencias superiores a fc.

**Equipamiento**: Generador de funciones capaz de generar **chirp** (barrido de frecuencias) o múltiples tonos.

**Configuración del filtro**:
- **Tipo**: Pasa Bajos
- **Frecuencia de corte**: 500 Hz
- **Orden**: 8

**Metodología**:
1. Aplicar señal senoidal de amplitud constante (1.0 Vpp)
2. Variar frecuencia desde 10 Hz hasta 1500 Hz
3. Medir amplitud en salida para cada frecuencia
4. Calcular atenuación: $A_{dB} = 20 \log_{10}(V_{out}/V_{in})$

**Frecuencias de prueba y atenuación esperada**:

| Frecuencia | Relación con fc | Atenuación teórica | Rango aceptable |
|------------|-----------------|-------------------|-----------------|
| 50 Hz | 0.1×fc | 0 dB (sin atenuación) | -0.5 a +0.5 dB |
| 250 Hz | 0.5×fc | -1 dB | -0.5 a -2 dB |
| **500 Hz** | **1.0×fc** | **-3 dB** | **-2.5 a -3.5 dB** |
| 1000 Hz | 2.0×fc | -48 dB | -45 a -51 dB |
| 1500 Hz | 3.0×fc | -72 dB | -68 a -76 dB |

**Tabla para completar**:

| Frecuencia | V_entrada | V_salida | Atenuación (dB) | ✓/✗ |
|------------|-----------|----------|-----------------|-----|
| 50 Hz | 1.00 V | _____ V | _____ dB | |
| 250 Hz | 1.00 V | _____ V | _____ dB | |
| 500 Hz | 1.00 V | _____ V | _____ dB | |
| 1000 Hz | 1.00 V | _____ V | _____ dB | |
| 1500 Hz | 1.00 V | _____ V | _____ dB | |

**Criterio de aceptación**:
- Atenuación a fc = -3 dB ± 0.5 dB ✓
- Pendiente en banda de rechazo ≈ -48 dB/octava ✓ (para orden 8)
- Sin oscilaciones/ringing en banda de paso ✓

**Cálculo de pendiente**:
```
Pendiente = (Atten_1000Hz - Atten_500Hz) / log2(1000/500)
          = (Atten_1000Hz - Atten_500Hz) / 1 octava
Esperado: ≈ -48 dB/octava
```

---

### 4.2 Filtro Pasa Altos (Atenuación de Bajas Frecuencias)

**Objetivo**: Verificar comportamiento complementario al pasa bajos.

**Configuración del filtro**:
- **Tipo**: Pasa Altos
- **Frecuencia de corte**: 1000 Hz
- **Orden**: 8

**Frecuencias de prueba y atenuación esperada**:

| Frecuencia | Relación con fc | Atenuación teórica | Rango aceptable |
|------------|-----------------|-------------------|-----------------|
| 250 Hz | 0.25×fc | -72 dB | -68 a -76 dB |
| 500 Hz | 0.5×fc | -48 dB | -45 a -51 dB |
| **1000 Hz** | **1.0×fc** | **-3 dB** | **-2.5 a -3.5 dB** |
| 1500 Hz | 1.5×fc | -1 dB | -0.5 a -2 dB |
| 1800 Hz | 1.8×fc | 0 dB | -0.5 a +0.5 dB |

**Validación cruzada**:
- Pasa Bajos (fc=1000Hz) + Pasa Altos (fc=1000Hz) deberían ser complementarios
- Suma de atenuaciones en frecuencia lejana ≈ -∞ dB (señal bloqueada)

---

### 4.3 Respuesta a Escalón (Análisis Temporal)

**Objetivo**: Verificar el comportamiento transitorio del filtro.

**Metodología**:
1. Aplicar señal cuadrada de baja frecuencia (1 Hz)
2. Observar forma de onda filtrada en visualización temporal
3. Medir parámetros de respuesta transitoria

**Parámetros a observar**:
- **Overshoot** (sobrepaso): Debe ser < 5% para Butterworth
- **Settling time** (tiempo de establecimiento): ~8 períodos del filtro
- **Ringing** (oscilaciones): Mínimo en Butterworth (respuesta maximimally flat)

**Comparación con teoría**:
- Butterworth: Sin ripple en banda de paso, overshoot mínimo
- Chebyshev: Mayor overshoot pero transición más abrupta (no implementado)

---

## 5. ANÁLISIS DE LATENCIA

### 5.1 Latencia Total del Sistema

**Objetivo**: Medir el retardo total desde entrada analógica hasta salida DAC.

**Equipamiento**: Osciloscopio de 2 canales

**Metodología**:
1. Aplicar señal impulso o flanco abrupto en entrada
2. Conectar CH1 del osciloscopio a entrada del Arduino (antes del ADC)
3. Conectar CH2 del osciloscopio a salida del DAC R2R
4. Medir diferencia temporal entre flancos

**Componentes de latencia esperados**:

| Componente | Tiempo esperado | Notas |
|------------|-----------------|-------|
| Conversión ADC | 100-120 μs | Dependiente de prescaler |
| Transmisión serial (ida) | ~260 μs | 10 bits @ 38400 bps |
| Procesamiento filtro | 10-20 μs | Depende de CPU |
| Transmisión serial (vuelta) | ~260 μs | 10 bits @ 38400 bps |
| Escritura DAC | < 5 μs | Escritura atómica PORTA |
| **Total** | **0.8-1.2 ms** | **Aceptable para audio** |

**Criterio de aceptación**:
- Latencia total < 2 ms ✓ Excelente
- Latencia total < 5 ms ✓ Aceptable para audio
- Latencia total < 10 ms ✓ Imperceptible humano
- Latencia total > 20 ms ✗ Problemático

**Nota**: La latencia puede aumentar si se procesa en bloques grandes o si hay sobrecarga en el PC.

---

### 5.2 Jitter y Estabilidad Temporal

**Objetivo**: Verificar que la frecuencia de muestreo sea estable.

**Metodología**:
1. Aplicar señal senoidal de frecuencia conocida (ej: 1000 Hz)
2. Capturar FFT durante 10 segundos
3. Observar ancho del pico fundamental
4. Verificar ausencia de picos espurios

**Valores esperados**:
- Ancho de pico a -3dB: 1-2 Hz (limitado por resolución 1 Hz/bin)
- Ausencia de sidebands (bandas laterales)
- Pico estable sin fluctuaciones

**Posibles problemas**:
- Picos anchos → Jitter en muestreo
- Sidebands → Modulación espuria
- Deriva lenta → Frecuencia no estable

---

## 6. PRUEBAS DE ESTRÉS Y LÍMITES

### 6.1 Prueba de Sobrecarga (Clipping)

**Objetivo**: Verificar comportamiento con señales que exceden el rango del ADC.

**Metodología**:
1. Aplicar señal senoidal con amplitud creciente
2. Observar punto de saturación
3. Verificar que no haya daño al circuito

**Valores esperados**:
- Saturación suave alrededor de ±6V (entrada)
- Correspondiente a ~0.8V y ~3.8V en ADC
- Sin distorsión excesiva antes de saturación

---

### 6.2 Prueba de Rango Dinámico

**Objetivo**: Medir el rango entre señal mínima detectable y saturación.

**Metodología**:
1. Aplicar señal senoidal con amplitud mínima detectable
2. Incrementar hasta saturación
3. Calcular rango dinámico en dB

**Cálculo**:
```
Rango dinámico (dB) = 20 × log10(V_max / V_min)

V_max ≈ 6.0 V (saturación)
V_min ≈ 0.05 V (limitado por ruido ~40-50 mV)

Rango esperado: 20 × log10(6.0 / 0.05) ≈ 42 dB
```

**Criterio de aceptación**:
- Rango dinámico > 40 dB ✓ Aceptable
- Rango dinámico > 50 dB ✓ Óptimo

---

## 7. VALIDACIÓN FUNCIONAL COMPLETA

### 7.1 Caso de Uso: Filtrado de Señal de Audio

**Escenario**: Eliminar ruido de alta frecuencia de señal de audio.

**Procedimiento**:
1. Generar señal de audio (ej: 440 Hz) con ruido superpuesto (ej: 5 kHz)
2. Configurar filtro pasa bajos con fc = 1000 Hz
3. Observar señal filtrada en visualización temporal y espectral
4. Verificar eliminación del ruido y preservación de señal útil

**Resultado esperado**:
- Componente 440 Hz preservada (atenuación < 1 dB)
- Componente 5 kHz atenuada > 50 dB
- THD de salida similar a entrada

---

### 7.2 Caso de Uso: Análisis de Distorsión Armónica

**Escenario**: Medir THD de un amplificador bajo prueba.

**Procedimiento**:
1. Aplicar señal senoidal 1 kHz pura al amplificador
2. Conectar salida del amplificador al sistema DSP
3. Observar armónicas en FFT
4. Leer valor de THD calculado por el sistema

**Interpretación**:
- THD < 1% → Amplificador Hi-Fi de calidad
- THD 1-3% → Amplificador de consumo aceptable
- THD > 5% → Distorsión audible, revisar amplificador

---

## 8. TROUBLESHOOTING (RESOLUCIÓN DE PROBLEMAS)

### 8.1 Problemas Comunes y Soluciones

| Problema | Posible Causa | Solución |
|----------|---------------|----------|
| Lecturas inestables/ruidosas | Fuente de alimentación ruidosa | Agregar capacitor 100μF en Vin |
| | Conexiones flojas | Verificar soldaduras |
| | Interferencia EMI | Alejar de motores/fuentes conmutadas |
| Offset DC incorrecto | Error en calibración LM324 | Verificar resistencias divisor (R1/R2) |
| | Voltaje referencia incorrecto | Medir Vref con multímetro |
| FFT muestra picos espurios | Aliasing (señal > fs/2) | Agregar filtro antialiasing RC |
| | Armónicas del generador | Verificar THD del generador |
| Filtro no atenúa correctamente | Configuración incorrecta | Verificar fc y tipo de filtro |
| | Bug en código | Verificar llamada a setup() |
| Saturación prematura | Acondicionador mal calibrado | Verificar ganancia LM324 |
| | Señal de entrada muy grande | Reducir amplitud generador |
| Comunicación serial falla | Baudrate incorrecto | Verificar 38400 bps en ambos lados |
| | Cable USB defectuoso | Probar otro cable |
| | Drivers USB no instalados | Instalar drivers CH340/FTDI |

---

## 9. REGISTRO DE PRUEBAS (PLANTILLA)

### Información General
- **Fecha**: ___________
- **Operador**: ___________
- **Versión firmware Arduino**: ___________
- **Versión aplicación PC**: ___________
- **Temperatura ambiente**: _____°C

### Checklist Inicial
- [ ] Arduino conectado y energizado
- [ ] Aplicación SerialPlotter iniciada correctamente
- [ ] Puerto COM detectado
- [ ] Visualización temporal funcionando
- [ ] Visualización FFT funcionando

### Resultados de Pruebas
- [ ] Prueba de precisión: APROBADA / RECHAZADA
- [ ] Prueba de exactitud: APROBADA / RECHAZADA
- [ ] FFT señal senoidal: APROBADA / RECHAZADA
- [ ] FFT señal cuadrada: APROBADA / RECHAZADA
- [ ] Filtro pasa bajos: APROBADO / RECHAZADO
- [ ] Filtro pasa altos: APROBADO / RECHAZADO
- [ ] Análisis de latencia: APROBADO / RECHAZADO

### Observaciones
```
[Espacio para notas del operador]
```

### Firma
Operador: ___________________ Fecha: ___________

---

## 10. REFERENCIAS Y RECURSOS ADICIONALES

### 10.1 Herramientas de Software Útiles
- **Audacity**: Generación de señales de prueba y análisis de audio
- **REW (Room EQ Wizard)**: Medición de respuesta en frecuencia
- **MATLAB/Octave**: Análisis avanzado de datos exportados

### 10.2 Calculadoras Online
- **Butterworth Filter Calculator**: http://www.analog.com/designtools/en/filterwizard/
- **THD Calculator**: Diversas herramientas online disponibles
- **dB Converter**: Conversión entre voltaje y dB

### 10.3 Documentación Relacionada
- `INFORME_TP3_COMPLETO.md`: Teoría y diseño del sistema
- `README.md`: Instrucciones de compilación e instalación
- `GUIA_CONFIGURACION.md`: Configuración inicial del entorno

---

**Última actualización**: Abril 2026
**Versión**: 1.0
