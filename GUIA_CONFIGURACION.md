# Guía de Configuración: DSP.ino ? SerialPlotter

## ?? Objetivo

Configurar correctamente el sistema de procesamiento digital de señales para que Arduino Mega 2560 y SerialPlotter trabajen en conjunto.

---

## ?? CONFIGURACIÓN PASO A PASO

### PASO 1: Modificar el Arduino (DSP.ino)

#### 1.1 Cambiar Baudrate

**Archivo**: `DSP-arduino\DSP\DSP.ino`

**Buscar esta línea en `setup()`**:
```cpp
usart.begin(115200);   // ? INCORRECTO
```

**Cambiar por**:
```cpp
usart.begin(38400);    // ? CORRECTO
```

#### 1.2 Activar Código DSP

**Buscar esta línea en `loop()`**:
```cpp
void loop()
{
   static uint32_t inicio = millis();  
   
   // Fase 1: Enviar secuencia de test (0-249)
   if (i < 250){
```

**COMENTAR TODO EL CÓDIGO DE TEST**:
```cpp
void loop()
{
   /* ? COMENTAR O ELIMINAR TODO ESTE BLOQUE
   static uint32_t inicio = millis();
   
   if (i < 250){
      usart.escribir_espera(i);
      if (usart.pendiente_lectura())
         lectura[contador++] = usart.leer();
      i++;
   }
   else if (i == 250 && enviar) {
      enviar = false;
      // ... resto del test
   }
   */
   
   // ? MANTENER SOLO ESTE CÓDIGO ACTIVO
   if (beat){
      beat = false;
      
      // Enviar muestra actual por serie a la interfaz C++
      uint8_t muestra_adc = adc.get();
      usart.escribir(muestra_adc);
      
      // Recibir datos procesados desde la interfaz C++
      if (usart.pendiente_lectura()){
         valor = usart.leer();
      }
      else {
         valor = 255 - muestra_adc;  // Fallback si no hay respuesta
      }
   }
}
```

#### 1.3 Eliminar Inversión (Opcional pero Recomendado)

**Cambiar**:
```cpp
valor = 255 - muestra_adc;  // ? Invierte señal
```

**Por**:
```cpp
valor = muestra_adc;        // ? Mantiene señal original
```

#### 1.4 Compilar y Subir

1. Abrir Arduino IDE
2. Abrir archivo `DSP-arduino\DSP\DSP.ino`
3. Seleccionar **Herramientas ? Placa ? Arduino Mega or Mega 2560**
4. Seleccionar **Herramientas ? Puerto ? COMx** (tu puerto)
5. Hacer clic en **Subir** (flecha derecha)
6. Esperar mensaje: "Subido con éxito"

---

### PASO 2: Verificar SerialPlotter

#### 2.1 Confirmar Settings.h

**Archivo**: `SerialPlotter\src\Settings.h`

**Verificar que tenga**:
```cpp
struct Settings {
    int sampling_rate = 3840;           // ? 3840 Hz
    int baud_rate = sampling_rate * 10; // ? 38400 baudios
    int samples = sampling_rate;        // ? 3840 muestras
    
    int maximum = 49;     // ?? Calibrar según hardware
    int minimum = 175;    // ?? Calibrar según hardware
    
    int stride = 4;
    int byte_stride = sizeof(double) * stride;
    double map_factor = 12.0 / (maximum - minimum);
    
    bool show_frame_time = false;
    bool open = false;
};
```

**IMPORTANTE**: Si `baud_rate` no se calcula automáticamente, cambiar a:
```cpp
int baud_rate = 38400;  // Forzar 38400 baudios
```

#### 2.2 Eliminar Inversión en MainWindow.cpp (Opcional)

**Archivo**: `SerialPlotter\src\MainWindow.cpp`

**Buscar en `SerialWorker()`**:
```cpp
write_buffer[i] = 255 - InverseTransformSample(resultado);  // ? Invierte
```

**Cambiar por**:
```cpp
write_buffer[i] = InverseTransformSample(resultado);        // ? Sin invertir
```

**?? NOTA**: Solo hacer este cambio si también eliminaste la inversión en Arduino (Paso 1.3).

#### 2.3 Compilar SerialPlotter

**Opción A: CMake + Ninja (Recomendado)**
```bash
cd SerialPlotter
cmake -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

**Opción B: Visual Studio**
1. Abrir `SerialPlotter.sln` (si existe)
2. Seleccionar **Release** en configuración
3. Build ? Rebuild Solution

---

### PASO 3: Conectar y Configurar

#### 3.1 Conexión Física

1. **Conectar Arduino Mega 2560** al puerto USB
2. **Verificar driver**: Debe aparecer como COMx en Windows
3. **Anotar número de puerto**: Ej. COM3, COM4, etc.

#### 3.2 Hardware del DAC R2R

**Verificar conexiones** en pines 22-29 del Arduino Mega:

```
Pin 22 (PA0) ? R (2R) ? Bit 0 (LSB)
Pin 23 (PA1) ? R (2R) ? Bit 1
Pin 24 (PA2) ? R (2R) ? Bit 2
Pin 25 (PA3) ? R (2R) ? Bit 3
Pin 26 (PA4) ? R (2R) ? Bit 4
Pin 27 (PA5) ? R (2R) ? Bit 5
Pin 28 (PA6) ? R (2R) ? Bit 6
Pin 29 (PA7) ? R (2R) ? Bit 7 (MSB)
                ?
           Salida DAC
```

**Valores típicos**: R = 10k? (resistencias de precisión 1%)

#### 3.3 Entrada ADC

**Verificar conexión** al canal 1 del ADC (Pin A1):

```
Señal de entrada ? Pin A1 (ADC1)
```

**Rango recomendado**: 0V a 5V (máximo tolerado)

---

### PASO 4: Iniciar SerialPlotter

#### 4.1 Ejecutar Aplicación

```bash
cd SerialPlotter\build-release
.\SerialPlotter.exe
```

#### 4.2 Seleccionar Puerto

1. En la barra superior, hacer clic en **"Puerto"**
2. Seleccionar el puerto COM del Arduino (ej. COM3)

#### 4.3 Abrir Configuración

1. Hacer clic en **"Configuración"**
2. Verificar los siguientes valores:

```
Frecuencia de muestreo: 3840 Hz
Velocidad (Baudios):    38400
Puerto COM:             COMx (tu puerto)
```

#### 4.4 Calibrar Mapeo (IMPORTANTE)

En la ventana de configuración:

1. Expandir sección **"Mapeo"**
2. Ajustar sliders:
   - **Máximo**: Valor ADC para voltaje máximo (+6V)
   - **Mínimo**: Valor ADC para voltaje mínimo (-6V)

**Método de calibración**:

a) **Señal conocida**: Conectar un generador de señales calibrado
   - Aplicar +5V ? Anotar valor ADC ? Ajustar "Máximo"
   - Aplicar 0V ? Anotar valor ADC ? Ajustar "Mínimo"

b) **Observación**: Conectar señal real y ajustar hasta que:
   - La gráfica muestre amplitud correcta
   - La forma de onda se vea centrada

**Valores por defecto**:
- Máximo = 49
- Mínimo = 175

?? **Nota**: Estos valores parecen invertidos (menor valor ? mayor voltaje), lo cual sugiere que el ADC podría estar configurado con referencia invertida.

#### 4.5 Conectar

1. Hacer clic en **"Conectar"**
2. Deberías ver la gráfica empezar a moverse
3. El LED del pin 13 debería encenderse si hay comunicación exitosa

---

### PASO 5: Verificación

#### 5.1 Prueba Básica

**Sin filtro aplicado**:
1. Conectar una señal de prueba al ADC (ej. señal senoidal 100 Hz)
2. Verificar que aparezca en la gráfica "Entrada"
3. La señal debe ser continua y estable

#### 5.2 Prueba con Filtro

**Filtro Pasa-Bajos**:
1. Expandir sección **"Filtro"**
2. Seleccionar **"Pasa bajos"**
3. Ajustar frecuencia de corte (ej. 200 Hz)
4. Verificar en gráfica "Salida" que:
   - Frecuencias bajas pasan sin atenuación
   - Frecuencias altas se atenúan

**Filtro Pasa-Altos**:
1. Seleccionar **"Pasa altos"**
2. Ajustar frecuencia de corte (ej. 1000 Hz)
3. Verificar comportamiento opuesto

#### 5.3 Prueba FFT

1. Expandir sección **"Análisis"**
2. Verificar espectro de frecuencias
3. Confirmar que se detecte la frecuencia dominante correctamente

**Ejemplo con señal 100 Hz**:
```
Frecuencia: 100 Hz
Desplazamiento: 0 V (si está centrada)
```

---

## ?? SOLUCIÓN DE PROBLEMAS

### Problema 1: No se visualiza nada

**Causas posibles**:
1. ? Baudrate incorrecto
2. ? Puerto COM equivocado
3. ? Código de test activo en Arduino

**Soluciones**:
1. Verificar Paso 1.1 (baudrate = 38400)
2. Seleccionar puerto correcto en SerialPlotter
3. Verificar Paso 1.2 (código DSP activo)

---

### Problema 2: La señal se visualiza pero invertida

**Causa**: Inversión de señal en uno o ambos lados

**Soluciones**:

**Opción A**: Eliminar ambas inversiones (recomendado)
- Seguir Paso 1.3 en Arduino
- Seguir Paso 2.2 en SerialPlotter
- Recompilar ambos

**Opción B**: Ajustar mapeo
- En Configuración ? Mapeo
- Intercambiar valores Máximo ? Mínimo

---

### Problema 3: Señal con mucho ruido/jitter

**Causas posibles**:
1. Baudrate muy alto para la velocidad real
2. Cable USB de mala calidad
3. Interferencia electromagnética

**Soluciones**:
1. Confirmar baudrate = 38400 en ambos lados
2. Usar cable USB corto y de calidad
3. Alejar cables de fuentes de interferencia
4. Agregar capacitor de desacople (100nF) en entrada ADC

---

### Problem 4: FFT muestra frecuencias incorrectas

**Causa**: Sampling rate incorrecto

**Solución**:
1. Verificar que `Timer1 timer1(3840.0)` en Arduino
2. Verificar que `sampling_rate = 3840` en SerialPlotter
3. Reiniciar ambos programas

---

### Problema 5: Latencia visible entre entrada y salida

**Causa**: Procesamiento del filtro IIR

**Soluciones**:
1. Es normal, latencia ~0.6-0.8 ms (3 muestras)
2. Reducir orden del filtro (cambiar `<8>` a `<4>` en código)
3. Usar filtro FIR en lugar de IIR (requiere código nuevo)

---

### Problema 6: SerialPlotter se congela

**Causas posibles**:
1. Stride muy bajo (renderiza demasiados puntos)
2. FFT calculándose muy frecuentemente

**Soluciones**:
1. En Configuración ? Rendimiento ? Aumentar Stride
2. Cerrar sección "Análisis" cuando no se use
3. Reducir ventana de visualización

---

## ?? PARÁMETROS DE REFERENCIA

### Configuración Estándar

| Parámetro | Valor | Unidad |
|-----------|-------|--------|
| Frecuencia de muestreo | 3840 | Hz |
| Baudrate | 38400 | baudios |
| Muestras FFT | 3840 | samples |
| Ventana visible | 30 | segundos |
| Stride | 4 | - |
| Orden filtro | 8 | - |

### Rangos de Frecuencia

| Filtro | Mínimo | Máximo | Unidad |
|--------|--------|--------|--------|
| Pasa-bajos | 1 | 960 | Hz |
| Pasa-altos | 960 | 1919 | Hz |

### Latencia del Sistema

| Componente | Latencia | Unidad |
|------------|----------|--------|
| Transmisión serial (Tx) | 0.26 | ms |
| Filtro IIR | 0.05 | ms |
| Transmisión serial (Rx) | 0.26 | ms |
| **Total** | **~0.6** | **ms** |

---

## ?? CONCEPTOS IMPORTANTES

### Baudrate vs Sampling Rate

**Relación**:
```
Baudrate mínimo = Sampling Rate × 10
```

**Explicación**:
- Cada byte necesita 10 bits (1 start + 8 data + 1 stop)
- Por cada muestra (1 byte), se transmiten 10 bits

**Ejemplo**:
- 3840 muestras/segundo × 10 bits/muestra = 38400 bits/segundo

### Mapeo ADC ? Voltaje

**Fórmula**:
```
Voltaje = (ADC - mínimo) × factor - 6
factor = 12.0 / (máximo - mínimo)
```

**Ejemplo con valores por defecto**:
```
factor = 12.0 / (49 - 175) = 12.0 / (-126) = -0.0952
Voltaje = (ADC - 175) × (-0.0952) - 6
```

**Para ADC = 49** (máximo):
```
Voltaje = (49 - 175) × (-0.0952) - 6 = 126 × 0.0952 - 6 = 6V ?
```

**Para ADC = 175** (mínimo):
```
Voltaje = (175 - 175) × (-0.0952) - 6 = 0 - 6 = -6V ?
```

### Filtros Butterworth

**Características**:
- Orden 8: Muy selectivo (48 dB/octava)
- Respuesta plana en banda de paso
- Introduce retardo de fase

**Trade-offs**:
- Mayor orden ? Mayor selectividad ? Mayor latencia
- Menor orden ? Menor selectividad ? Menor latencia

---

## ?? CHECKLIST FINAL

Antes de dar por completada la configuración, verificar:

- [ ] Arduino compilado con baudrate 38400
- [ ] Código DSP activo (sin test)
- [ ] SerialPlotter compilado en Release
- [ ] Puerto COM correcto seleccionado
- [ ] Baudrate 38400 en ambos lados
- [ ] Sampling rate 3840 Hz en ambos lados
- [ ] Mapeo calibrado (máximo/mínimo)
- [ ] Señal de entrada conectada al ADC
- [ ] DAC R2R conectado a pines 22-29
- [ ] Visualización funcionando
- [ ] Filtros funcionando correctamente
- [ ] FFT mostrando frecuencias correctas

---

## ?? SOPORTE

Si después de seguir esta guía sigues teniendo problemas:

1. Verificar versión de Arduino IDE (mínimo 1.8.x)
2. Verificar drivers CH340/FTDI instalados
3. Probar con otro cable USB
4. Verificar que no haya otro programa usando el puerto COM
5. Revisar documentación detallada en `ANALISIS_COMPATIBILIDAD.md`

---

**Última actualización**: 2024  
**Versión**: 1.0  
**Compatible con**: Arduino Mega 2560 + SerialPlotter v1.0
