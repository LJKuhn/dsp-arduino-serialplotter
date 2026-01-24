# Análisis de Compatibilidad: DSP.ino ? SerialPlotter

## Resumen Ejecutivo

He analizado ambos códigos y encontré **incompatibilidades críticas** que impiden su funcionamiento conjunto. A continuación detallo los problemas y sus soluciones.

---

## ?? PROBLEMAS IDENTIFICADOS

### 1. **CRÍTICO: Código de Test en Loop() Principal**

**Problema**: El archivo `DSP.ino` tiene un loop principal que ejecuta un **test de comunicación** (envío de secuencia 0-249) en lugar del código DSP operativo.

```cpp
// Código actual en loop() - BLOQUEA EL DSP
void loop() {
   // Test de comunicación que envía 0-249
   if (i < 250){
      usart.escribir_espera(i);
      // ...
   }
}
```

El código DSP real está comentado o inactivo:

```cpp
// Código DSP activo - Sistema bidireccional ADC ? PC ? DAC
if (beat){
   beat = false;
   uint8_t muestra_adc = adc.get();
   usart.escribir(muestra_adc);
   
   if (usart.pendiente_lectura()){
      valor = usart.leer();
   }
   else {
      valor = 255 - muestra_adc;
   }
}
```

**Impacto**: El SerialPlotter espera recibir muestras de señal continuas, pero el Arduino está enviando una secuencia de test.

---

### 2. **CONFIGURACIÓN: Frecuencia de Muestreo y Baudrate**

#### Arduino (DSP.ino):
```cpp
Timer1 timer1(3840.0);      // 3840 Hz
usart.begin(115200);        // 115200 baudios
```

#### SerialPlotter (Settings.h):
```cpp
int sampling_rate = 3840;   // ? Coincide
int baud_rate = 38400;      // ? NO coincide (115200 vs 38400)
```

**Problema**: 
- El Arduino transmite a **115200 baudios**
- El SerialPlotter espera **38400 baudios** (sampling_rate × 10)

**Cálculo correcto**:
- Frecuencia: 3840 Hz
- Baudrate necesario: 3840 × 10 = **38400 baudios**

**Causa raíz**: El Arduino está configurado incorrectamente a 115200 baudios.

---

### 3. **PROTOCOLO: Inversión de Muestra**

#### Arduino envía:
```cpp
uint8_t muestra_adc = adc.get();  // Valor 0-255 directo del ADC
usart.escribir(muestra_adc);      // Envía sin modificar
```

#### Arduino recibe y escribe al DAC:
```cpp
if (usart.pendiente_lectura()){
   valor = usart.leer();           // Recibe valor procesado
}
else {
   valor = 255 - muestra_adc;      // ? Invierte si no hay datos
}
```

#### SerialPlotter devuelve:
```cpp
write_buffer[i] = 255 - InverseTransformSample(resultado);
```

**Problema**: Hay una **doble inversión** innecesaria:
1. SerialPlotter invierte: `255 - valor`
2. Arduino invierte: `255 - muestra` (en fallback)

**Resultado**: La señal se invierte innecesariamente dos veces.

---

### 4. **SINCRONIZACIÓN: Timer vs Comunicación Serial**

#### En Arduino:
```cpp
ISR(TIMER1_COMPA_vect) {
   write(valor);  // Escribe al DAC en cada tick del timer
   beat = true;   // Señaliza que hay que enviar/recibir
}

void loop() {
   if (beat){
      beat = false;
      uint8_t muestra_adc = adc.get();
      usart.escribir(muestra_adc);  // NO bloqueante
      
      if (usart.pendiente_lectura()){
         valor = usart.leer();
      }
   }
}
```

**Análisis**:
- ? Correcto: usa `escribir()` no bloqueante
- ? Correcto: lee solo si hay datos disponibles
- ?? Riesgo: Si el PC no responde rápido, el Arduino usa `valor = 255 - muestra_adc`

#### En SerialPlotter:
```cpp
void MainWindow::SerialWorker() {
   while (do_serial_work) {
      int read = serial.read(read_buffer.data(), 1);  // Lee 1 byte
      
      for (size_t i = 0; i < read; i++) {
         // Procesa muestra
         double resultado = transformado;
         switch (selected_filter) {
            case Filter::LowPass:
               resultado = lowpass_filter.filter(transformado);
               break;
            // ...
         }
         write_buffer[i] = 255 - InverseTransformSample(resultado);
      }
      
      serial.write(write_buffer.data(), read);  // Responde
   }
}
```

**Análisis**:
- ? Lee 1 byte a la vez (evita latencia)
- ? Responde inmediatamente después de procesar
- ?? Si el filtro IIR toma mucho tiempo, puede haber retraso

---

## ?? SOLUCIONES REQUERIDAS

### Solución 1: Activar Código DSP en Arduino

**Cambio en `DSP.ino`**:

```cpp
void loop()
{
   // ? ELIMINAR O COMENTAR TODO EL TEST DE COMUNICACIÓN
   /*
   static uint32_t inicio = millis();
   if (i < 250){
      usart.escribir_espera(i);
      // ...
   }
   */
   
   // ? ACTIVAR ESTE CÓDIGO COMO LOOP PRINCIPAL
   if (beat){
      beat = false;
      
      // Leer ADC y enviar a PC
      uint8_t muestra_adc = adc.get();
      usart.escribir(muestra_adc);
      
      // Recibir datos procesados de PC
      if (usart.pendiente_lectura()){
         valor = usart.leer();
      }
      else {
         valor = 255 - muestra_adc;  // Fallback si no hay respuesta
      }
   }
}
```

---

### Solución 2: Corregir Baudrate en Arduino

**Cambio en `DSP.ino`**:

```cpp
void setup()
{
   adc.begin(1);
   usart.begin(38400);  // ? Cambiar de 115200 a 38400
   
   DDRA = 0xFF;
   timer1.setup();
   timer1.start();
   
   pinMode(13, OUTPUT);
   digitalWrite(13, false);
}
```

**Justificación**:
- Frecuencia de muestreo: 3840 Hz
- Baudrate = 3840 × 10 = **38400 baudios**
- Relación 10:1 asegura transmisión confiable (10 bits por byte)

---

### Solución 3: Eliminar Doble Inversión

**Opción A: Eliminar inversión en SerialPlotter**

Cambio en `MainWindow.cpp`:

```cpp
write_buffer[i] = InverseTransformSample(resultado);  // Sin invertir
```

**Opción B: Eliminar inversión en Arduino**

Cambio en `DSP.ino`:

```cpp
if (usart.pendiente_lectura()){
   valor = usart.leer();  // Sin invertir
}
else {
   valor = muestra_adc;   // Sin invertir (en lugar de 255 - muestra_adc)
}
```

**Recomendación**: **Opción B** - Es más lógico que:
- Arduino envíe ADC sin procesar
- SerialPlotter devuelva valor procesado sin procesar
- Arduino use el valor directamente

---

### Solución 4: Verificar Mapeo ADC

#### En SerialPlotter:
```cpp
Settings {
   int maximum = 49;      // Valor ADC para +6V
   int minimum = 175;     // Valor ADC para -6V
   double map_factor = 12.0 / (maximum - minimum);
}

double TransformSample(uint8_t v) {
   return (v - minimum) * map_factor - 6;
}
```

**Análisis**:
- Rango ADC: 175 a 49 (invertido)
- Rango voltaje: -6V a +6V

**Verificación necesaria**: 
¿Este mapeo coincide con el hardware del Arduino?
- ¿El ADC está realmente invertido?
- ¿Los valores 49 y 175 son correctos para tu circuito?

**Recomendación**: Calibrar usando la ventana de configuración del SerialPlotter.

---

## ? CONFIGURACIÓN FINAL RECOMENDADA

### Arduino (DSP.ino):

```cpp
void setup() {
   adc.begin(1);
   usart.begin(38400);     // ? 38400 baudios
   
   DDRA = 0xFF;            // Pines 22-29 como salida (DAC)
   timer1.setup();
   timer1.start();
   
   pinMode(13, OUTPUT);
   digitalWrite(13, LOW);
}

void loop() {
   if (beat){
      beat = false;
      
      // Enviar muestra ADC a PC
      uint8_t muestra_adc = adc.get();
      usart.escribir(muestra_adc);
      
      // Recibir valor procesado de PC
      if (usart.pendiente_lectura()){
         valor = usart.leer();        // ? Sin invertir
      }
      else {
         valor = muestra_adc;         // ? Sin invertir (fallback)
      }
   }
}
```

### SerialPlotter (Settings.h):

```cpp
struct Settings {
   int sampling_rate = 3840;        // ? 3840 Hz
   int baud_rate = 38400;           // ? 38400 baudios
   int samples = 3840;              // ? 3840 muestras para FFT
   
   int maximum = 49;                // ?? Verificar con hardware real
   int minimum = 175;               // ?? Verificar con hardware real
   
   int stride = 4;                  // Renderiza 1 de cada 4 puntos
};
```

### SerialPlotter (MainWindow.cpp):

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
         
         // ? Sin invertir - enviar valor directo
         write_buffer[i] = InverseTransformSample(resultado);
      }
      
      serial.write(write_buffer.data(), read);
   }
}
```

---

## ?? CHECKLIST DE CONFIGURACIÓN

### Paso 1: Modificar Arduino
- [ ] Cambiar baudrate a 38400 en `setup()`
- [ ] Comentar/eliminar código de test en `loop()`
- [ ] Activar código DSP bidireccional
- [ ] Eliminar inversión `255 -` en fallback
- [ ] Compilar y subir a Arduino Mega 2560

### Paso 2: Verificar SerialPlotter
- [ ] Confirmar `baud_rate = 38400` en Settings
- [ ] Verificar `sampling_rate = 3840`
- [ ] Eliminar inversión en `write_buffer[i]`
- [ ] Compilar SerialPlotter

### Paso 3: Calibración
- [ ] Conectar Arduino Mega 2560 al puerto USB
- [ ] Abrir SerialPlotter
- [ ] Seleccionar puerto COM correcto
- [ ] Ir a "Configuración" ? "Mapeo"
- [ ] Ajustar valores `maximum` y `minimum` según hardware
- [ ] Conectar señal de entrada al ADC
- [ ] Verificar que la forma de onda se visualice correctamente

### Paso 4: Pruebas
- [ ] Test sin filtro: Señal entrada = señal salida
- [ ] Test filtro pasa-bajos: Frecuencias altas atenuadas
- [ ] Test filtro pasa-altos: Frecuencias bajas atenuadas
- [ ] Test FFT: Verificar frecuencia dominante
- [ ] Test latencia: Señal de salida no debe tener retraso visible

---

## ?? ANÁLISIS TÉCNICO ADICIONAL

### Baudrate vs Frecuencia de Muestreo

**Relación teórica**:
- Cada byte serial necesita 10 bits (1 start + 8 data + 1 stop)
- Baudrate mínimo = frecuencia × 10

**Tu configuración**:
- 3840 Hz × 10 = **38400 baudios mínimo**
- 115200 baudios daría: 115200 / 10 = **11520 Hz máximo**

**Conclusión**: 115200 es excesivo para 3840 Hz y causa desincronización.

---

### Latencia del Sistema

**Fuentes de latencia**:

1. **Arduino**:
   - Conversión ADC: ~100 µs (insignificante)
   - Transmisión serial: 10 bits / 38400 = **260 µs por byte**

2. **SerialPlotter**:
   - Lectura serial: negligible (API de Windows)
   - Filtro IIR Butterworth 8° orden: **~10-50 µs**
   - Transformación: negligible
   - Transmisión serial: 260 µs

3. **Arduino (recepción)**:
   - Buffer USART: negligible
   - Escritura DAC: < 1 µs

**Latencia total estimada**: ~600-800 µs = **0.6-0.8 ms**

A 3840 Hz, cada muestra tarda: 1/3840 = **0.26 ms**

**Conclusión**: Latencia ? 3 muestras de retraso (aceptable para DSP en tiempo real).

---

### Filtros IIR

**Configuración actual**:
```cpp
Iir::Butterworth::LowPass<8> lowpass_filter;
Iir::Butterworth::HighPass<8> highpass_filter;
```

**Características**:
- Orden 8: Pendiente ~48 dB/octava
- Tipo Butterworth: Respuesta plana en banda de paso
- Fase no lineal (introduce retardo de grupo)

**Rangos recomendados**:
- **Pasa-bajos**: 1 Hz - 960 Hz (fs/4)
- **Pasa-altos**: 960 Hz - 1919 Hz (fs/2 - 1)

---

## ?? POSIBLES PROBLEMAS ADICIONALES

### 1. Hardware Mega 2560

El código usa `PORTA` completo (pines 22-29):

```cpp
void write(uint8_t valor){
   PORTA = valor;  // ? Correcto para Mega 2560
}
```

**Verificar**:
- ¿Tienes un **Arduino Mega 2560** o un **Arduino Uno**?
- Arduino Uno requeriría usar `PORTD` y `PORTB` (código diferente)

---

### 2. Buffer Overflow

Si el SerialPlotter no responde lo suficientemente rápido:

```cpp
if (usart.pendiente_lectura()){
   valor = usart.leer();
}
else {
   valor = muestra_adc;  // Fallback
}
```

**Impacto**: El DAC genera la señal sin filtrar cuando hay atraso.

**Solución**: El código ya maneja esto correctamente con fallback.

---

### 3. ADC Timing

El ADC no está sincronizado explícitamente con el timer:

```cpp
ISR(TIMER1_COMPA_vect) {
   write(valor);  // Escribe valor ANTERIOR
   beat = true;
}

void loop() {
   if (beat){
      uint8_t muestra_adc = adc.get();  // Lee ADC AHORA
   }
}
```

**Análisis**: Hay un desfase de 1 muestra entre ADC y DAC (aceptable).

---

## ?? DIAGRAMA DE FLUJO

```
???????????????
?  ARDUINO    ?
?  Timer1     ?
?  (3840 Hz)  ?
???????????????
       ?
       ?
???????????????
?  ADC Read   ?
?  (0-255)    ?
???????????????
       ?
       ?
???????????????
?  USART TX   ?
?  38400 baud ?
???????????????
       ?
       ?
???????????????
? SerialPort  ?
?  (Windows)  ?
???????????????
       ?
       ?
???????????????
?  Transform  ?
?  ADC ? V    ?
???????????????
       ?
       ?
???????????????
? IIR Filter  ?
?(Butterworth)?
???????????????
       ?
       ?
???????????????
?  Transform  ?
?  V ? ADC    ?
???????????????
       ?
       ?
???????????????
?  USART RX   ?
?  38400 baud ?
???????????????
       ?
       ?
???????????????
?  DAC Write  ?
?  (PORTA)    ?
???????????????
```

---

## ?? NOTAS FINALES

### Ventajas del Sistema

1. **Arquitectura limpia**: Separación clara ADC ? PC ? DAC
2. **Filtros profesionales**: Butterworth de 8° orden
3. **Visualización en tiempo real**: FFT y gráficas
4. **Bajo jitter**: Uso de PORTA completo en Mega 2560

### Limitaciones

1. **Latencia inevitable**: ~0.6-0.8 ms (3 muestras)
2. **Baudrate limitante**: Máximo ~11520 Hz con 115200 baud
3. **Solo Windows**: API de Windows para serial
4. **Sin sincronización de fase**: Filtros IIR introducen retardo

---

## ?? REFERENCIAS

- [Arduino Mega 2560 Pinout](https://docs.arduino.cc/hardware/mega-2560)
- [AVR USART](https://ww1.microchip.com/downloads/en/DeviceDoc/ATmega640-1280-1281-2560-2561-Datasheet-DS40002211A.pdf)
- [Butterworth Filters](https://en.wikipedia.org/wiki/Butterworth_filter)
- [DSP Latency Analysis](https://www.dspguide.com/)

---

**Creado**: 2024
**Autor**: Análisis automático de compatibilidad
**Versión**: 1.0
