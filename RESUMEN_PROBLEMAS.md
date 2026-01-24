# Resumen de Problemas de Compatibilidad

## ?? PROBLEMAS CRÍTICOS (Impiden funcionamiento)

### 1. Baudrate Incorrecto ?? CRÍTICO

**Arduino**: `usart.begin(115200);` ? Transmite a 115200 baudios  
**SerialPlotter**: `baud_rate = 38400;` ? Espera 38400 baudios

**Impacto**: Sin comunicación o datos corruptos  
**Solución**: Cambiar Arduino a `usart.begin(38400);`

---

### 2. Código de Test Activo ?? CRÍTICO

**Problema**: El loop() principal ejecuta un test de comunicación que envía secuencia 0-249 en lugar de muestras ADC reales.

**Código actual**:
```cpp
void loop() {
   if (i < 250){
      usart.escribir_espera(i);  // Envía 0,1,2...249
   }
}
```

**Código esperado**:
```cpp
void loop() {
   if (beat){
      uint8_t muestra_adc = adc.get();
      usart.escribir(muestra_adc);  // Envía muestra ADC real
   }
}
```

**Impacto**: SerialPlotter recibe datos de test en lugar de señal real  
**Solución**: Comentar test y activar código DSP bidireccional

---

## ?? PROBLEMAS MODERADOS (Causan errores de señal)

### 3. Doble Inversión de Señal

**Arduino envía**: Valor ADC directo (0-255)  
**SerialPlotter devuelve**: `255 - valor` (invertido)  
**Arduino escribe al DAC**: `255 - muestra_adc` en fallback (invertido)

**Resultado**: Doble inversión ? señal correcta por casualidad  
**Problema**: Si Arduino recibe datos del PC, la señal queda invertida

**Solución recomendada**:
- Arduino: `valor = muestra_adc;` (sin invertir)
- SerialPlotter: `write_buffer[i] = InverseTransformSample(resultado);` (sin invertir)

---

## ?? PROBLEMAS MENORES (Requieren calibración)

### 4. Mapeo ADC no Calibrado

**Valores por defecto**:
```cpp
int maximum = 49;   // +6V
int minimum = 175;  // -6V
```

**Problema**: Estos valores son específicos del hardware y pueden no coincidir con tu circuito

**Impacto**: Amplitud incorrecta en visualización  
**Solución**: Calibrar usando ventana de configuración

---

### 5. Inversión de Rango ADC

**Valores actuales**: `maximum = 49 < minimum = 175`

**Indica**: ADC configurado con referencia invertida o circuito inversor

**Impacto**: Ninguno si se calibra correctamente  
**Nota**: Verificar que esta inversión sea intencional

---

## ?? CHECKLIST DE CORRECCIONES

### Cambios en Arduino (DSP.ino)

```cpp
void setup() {
   adc.begin(1);
   usart.begin(38400);     // ? Cambiar de 115200
   
   DDRA = 0xFF;
   timer1.setup();
   timer1.start();
   
   pinMode(13, OUTPUT);
   digitalWrite(13, LOW);
}

void loop() {
   // ? Comentar todo el código de test
   
   // ? Activar este código
   if (beat){
      beat = false;
      
      uint8_t muestra_adc = adc.get();
      usart.escribir(muestra_adc);
      
      if (usart.pendiente_lectura()){
         valor = usart.leer();         // ? Sin invertir
      }
      else {
         valor = muestra_adc;          // ? Sin invertir (cambio opcional)
      }
   }
}
```

### Cambios en SerialPlotter (MainWindow.cpp)

```cpp
void MainWindow::SerialWorker() {
   while (do_serial_work) {
      int read = serial.read(read_buffer.data(), 1);
      
      for (size_t i = 0; i < read; i++) {
         double transformado = TransformSample(read_buffer[i]);
         scrollY->push(transformado);
         scrollX->push(next_time);
         
         double resultado = transformado;
         switch (selected_filter) {
            case Filter::LowPass:
               resultado = lowpass_filter.filter(transformado);
               break;
            case Filter::HighPass:
               resultado = highpass_filter.filter(transformado);
               break;
            case Filter::None:
               break;
         }
         
         filter_scrollY->push(resultado);
         next_time += 1.0 / settings->sampling_rate;
         
         // ? Sin invertir (cambio opcional, coordinado con Arduino)
         write_buffer[i] = InverseTransformSample(resultado);
      }
      
      serial.write(write_buffer.data(), read);
   }
}
```

---

## ?? PRIORIDAD DE CORRECCIONES

### Prioridad 1 (Obligatorio):
1. ? Cambiar baudrate a 38400 en Arduino
2. ? Activar código DSP en Arduino (comentar test)

### Prioridad 2 (Recomendado):
3. ? Eliminar doble inversión (coordinar Arduino + SerialPlotter)
4. ? Calibrar mapeo ADC según hardware real

### Prioridad 3 (Opcional):
5. ? Ajustar orden de filtros si hay mucha latencia
6. ? Optimizar stride si hay problemas de rendimiento

---

## ?? TABLA COMPARATIVA

| Aspecto | Arduino (actual) | SerialPlotter (actual) | Estado |
|---------|------------------|------------------------|---------|
| Baudrate | 115200 | 38400 | ? No coincide |
| Sampling rate | 3840 Hz | 3840 Hz | ? Correcto |
| Transmisión | Test (0-249) | Espera ADC real | ? Incompatible |
| Inversión Tx | No | Sí (255-x) | ?? Asimétrico |
| Inversión Rx | Sí (255-x) | - | ?? Depende de Tx |
| Mapeo ADC | N/A | 49-175 (invertido) | ?? Calibrar |

---

## ?? DOCUMENTOS RELACIONADOS

- **Análisis completo**: `ANALISIS_COMPATIBILIDAD.md`
- **Guía paso a paso**: `GUIA_CONFIGURACION.md`
- **Documentación técnica**: `SerialPlotter\DOCUMENTACION.md`
- **README Arduino**: `DSP-arduino\DSP\README_Explicacion_Detallada.md`

---

## ?? EXPLICACIÓN TÉCNICA

### ¿Por qué baudrate = sampling_rate × 10?

Cada byte serial necesita:
- 1 bit de start
- 8 bits de datos
- 1 bit de stop
- **Total: 10 bits por byte**

Para transmitir 3840 muestras/segundo:
```
3840 muestras/s × 10 bits/muestra = 38400 bits/s = 38400 baudios
```

### ¿Por qué el código de test impide el funcionamiento?

El test envía una secuencia predefinida (0-249) que:
1. **Bloquea** el envío de muestras ADC reales
2. **Usa escritura bloqueante** (`escribir_espera()`) que detiene el programa
3. **Solo se ejecuta una vez** y luego enciende LED y se detiene

El código DSP necesita:
1. **Envío continuo** de muestras ADC a 3840 Hz
2. **Escritura no bloqueante** (`escribir()`) para no perder muestras
3. **Recepción asíncrona** de datos procesados del PC

---

**Creado**: 2024  
**Versión**: 1.0
