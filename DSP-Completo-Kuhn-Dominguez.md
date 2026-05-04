DSP Completo

Lautaro Kühn y Federico Domínguez

Ingeniería en Computación, Universidad Nacional de Rafaela 

Procesamiento de Señales Digitales 

Prof. Milton Pozzo

Mayo de 2026

Índice

Introducción

El objetivo de este trabajo es profundizar en el procesamiento de señales mediante la aplicación práctica de conocimientos de ingeniería, con el fin de diseñar e implementar un sistema Digital Signal Processor (DSP) integral utilizando un dispositivo programable. El sistema debe ser capaz de cumplir con tres etapas fundamentales:

Adquisición y Acondicionamiento: Adaptar una señal analógica externa (originalmente de 6V) mediante circuitos operacionales para que sea compatible con los rangos de lectura del microcontrolador.

Procesamiento Digital: Implementar algoritmos avanzados en PC, como la Transformada Rápida de Fourier (FFT) para el análisis espectral y la detección de armónicas, junto con el diseño de filtros digitales IIR (Butterworth orden 8) para la manipulación de la señal en tiempo real.

Recreación de Señal (DAC): Generar diferentes tipos de señales de salida utilizando un hardware externo basado en una red de resistencias R2R, permitiendo modificar la forma y frecuencia de la señal procesada.

A través de esta arquitectura, se busca validar la versatilidad de plataformas como el Arduino Mega 2560 en entornos de procesamiento de señales, analizando las métricas de desempeño, latencia y las limitaciones técnicas propias de los componentes utilizados.

Desarrollo ADC

Características del Arduino Mega 2560:

Optamos por utilizar la placa de desarrollo Arduino Mega 2560 que utilizamos en un proyecto anterior y que ya la poseíamos de antemano. Las entradas que utilizaremos para censar la señal serán las analógicas, dicho micro cuenta con 16 entradas de este tipo y a su vez cada una de ellas cuenta con una resolución de 10-bit. Esto quiere decir que el rango de valores posibles que puede tomar la señal leída iría desde 0 a 1023 valores posibles. Por otro lado, su conversor ADC también cuenta con la capacidad de configurar un prescaler del conversor para modificar la velocidad de medición del conversor. 



Figura 1.Pines Arduino Mega 2560

El prescaler se encarga de dividir la frecuencia del clock interno del microcontrolador (16MHz) por un valor de potencia de dos para bajar la frecuencia de medición y así que el conversor tenga tiempo de realizar la conversión de analógico a digital. Cabe aclarar que el prescaler máximo que se puede utilizar sin afectar el tiempo de conversión de analógico a digital es 128, dándonos que la frecuencia máxima a la que se puede medir es de 125KHz (16MHz/128). Sin embargo, si se puede utilizar prescaler más chicos como 64, 32 o 16, pero esto afectará la resolución del ADC ya que este tendrá menos tiempo para realizar la conversión. Conversor ADC del Arduino tarda entre 13us a 260us ya que al utiliza la aproximación sucesiva para obtener el valor que se está leyendo.

Ahora retomando, como la velocidad de medición aceptable sin afectar la resolución del conversor es de 125KHz podemos deducir que la señal en términos aceptables que podremos medir tendrá una frecuencia de 62,5KHz, dos mediciones por ciclo de la señal, ahora si quisiéramos tener una muy buena imagen de la señal la frecuencia máxima de la misma deberá ser de 12,5KHz, para poder tener 10 mediciones por ciclo.

Ahora hay que tener en cuenta que la visualización de dichos datos en la computadora tendrá un retraso dependiendo de la velocidad a la que configuremos el puerto serie por el que nos comunicaremos. Podremos elegir entre una velocidad de comunicación de 300-bits por segundo a 115200-bits por segundo. Cabe resaltar que en un principio esto no debería afectar al conversor ya que solo se están enviando los valores obtenidos y almacenados en el Arduino, pero puede darse el caso de que se eligen velocidades de comunicación muy bajas no se lleguen a visualizar algunos datos o por el otro lado si se envían datos por el puerto seri se utilicen recursos para procesarlos y disminuya la velocidad de comunicación.

Como mencionamos anteriormente el conversor ADC trabaja utilizando la aproximación sucesiva para determinar el valor leído, pero cuenta con varias opciones de configuración para modificar ciertos parámetros en el funcionamiento del conversor.

Se puede modificar de la Referencia Analógica, esto determina el rango de voltaje que el ADC utiliza para realizar las conversiones. Por lo general, hay opciones como referencia interna, referencia externa o referencia AVCC (que toma el voltaje de alimentación del microcontrolador como referencia), por defecto el conversor operara comprando el valor leído con un rango de tensión de 0V a 5V.

Por otro lado, se pueden modificar el Modo de Conversión del ADC ya que este puede operar en diferentes modos de conversión, como el modo de conversión única (una sola conversión a la vez) o el modo de conversión libre (conversión continua).

En el Modo de Conversión Única (Single Conversion Mode) el ADC realiza una única conversión cada vez que se inicia una solicitud de conversión. El microcontrolador inicia manualmente la conversión cuando se requiere una lectura analógica. Después de que se completa la conversión, el valor digital resultante se puede leer desde el registro del ADC. Es adecuado para aplicaciones donde se necesita una lectura de ADC puntual en respuesta a algún evento o solicitud específica.

A su vez cuando trabaja en Modo de Conversión Libre (Free Running Mode) o Continuous Conversion Mode el ADC realiza conversiones continuamente a una velocidad determinada sin requerir solicitudes manuales. El ADC sigue muestreando y convirtiendo la señal de entrada a una velocidad constante hasta que se detiene explícitamente mediante la configuración del registro de control. Es útil para aplicaciones donde se necesita un monitoreo constante y continuo de una señal analógica sin la necesidad de iniciar manualmente cada conversión. También es común en aplicaciones de adquisición de datos. 

También existen el Prescaler y las Interrupciones. Como mencionamos anteriormente, con el Prescaler se puede controlar la velocidad de muestreo del ADC. Esto divide la frecuencia del reloj del sistema para establecer la frecuencia de muestreo deseada. A su vez con las Interrupciones podemos configurar el ADC para generar interrupciones cuando se completa una conversión. Esto es útil para aplicaciones en las que deseas realizar otras tareas mientras el ADC convierte señales.

Como parámetros de seguridad debemos tener en consideración el voltaje máximo de la señal de entrada esté dentro de los límites de tolerancia del dispositivo. Cada dispositivo tendrá una clasificación de voltaje máximo que no debe excederse. Si se excede este valor, puede dañar permanentemente el componente, en el caso de los pines analógicos del Arduino Mega 2560 el rango de voltaje de operación va desde los 0V a los 5V, por lo que si se necesitan medir señales mayores podemos utilizar el pin de referencia del Arduino o acondicionar la señal de modo que quede dentro del rango de trabajo del mismo.

Esto mismo aplica para señales alternas o negativas, se debe verificar la polaridad de la señal de entrada ya que Algunos dispositivos, como los ADC, pueden no manejar señales negativas si no se proporciona una referencia de voltaje adecuada. Por eso debemos de asegurarnos de que la polaridad sea compatible con las especificaciones del dispositivo o realizar previamente un proceso de acondicionamiento de la señal.

Nivel de Corriente: Limita la corriente de la señal de entrada para evitar la sobrecarga del dispositivo. Puedes utilizar resistencias limitadoras en serie con la señal para controlar la corriente.

Por último, se podrían agregar protecciones contra descargas electrostáticas (ESD) y protecciones contra sobretensión ya que la señal podría estar expuesta a ESD y cambios de tensión. Los componentes electrónicos son sensibles a estas perturbaciones y pueden dañarse fácilmente sin la protección adecuada.

Acondicionamiento de la señal:

Pare de este proyecto debemos de medir una señal senoidal que posee una amplitud pico a pico de 12V, que no cuenta con un desplazamiento vertical y nosotros podremos determinar la frecuencia de la misma. 

Por ende, investigando concluimos en que para que sea posible una correcta lectura de la señal con el Arduino deberíamos reducir la amplitud de la misma y luego agregarle un offset para que esta quede situada dentro del rango optimo del conversor del mismo. 

Reducción de amplitud: 

Teoría:

Para la reducción de la amplitud usamos un divisor resistivo donde reduciríamos la amplitud de la misma para que nos quede una amplitud pico a pico de 5V a la que le podremos agregar un offset de 2,5V capaz de desplazar la señal verticalmente hacia arriba permitiéndonos utilizar la totalidad de la resolución del conversor y a su vez nos permitirá asegurarnos de no quemar el circuito.

Considerando:                VIN=6V   VOUT=2,5V   R2=1000ohm   R1=?            

VOUT = (R2/R1+R2) * VIN   R1=(VIN/VOUT) * R2 – R2

R1=(6V/2,5V) * 1000 – 1000 = 1400ohm



Figura 2.Primer divisor resistivo

Con los valores calculados anteriormente obtendríamos una caída de 2,5V en la resistencia de 1000ohm y es la que usaremos posteriormente para seguir trabajando.

Practica:

En la práctica nos dimos cuenta de que la amplitud de la señal que caería en la R2 debería de ser de 1,5V porque más adelante el circuito que utilizaremos para añadirle un offset a la señal recorta los valores inferiores a 0,8V y superiores a 3,8V, el funcionamiento de dicho circuito se explicara más adelante. 

Debido a esta limitación debimos recalcular los valores de resistencias del divisor resistivo para obtener una caída de 1,5v V en R2.



Considerando:                VIN=6V   VOUT=1,5V   R2=500ohm   R1=?

VOUT = (R2/R1+R2) * VIN   R1=(VIN/VOUT) * R2 – R2

R1=(6V/1,5V) * 500 – 500 = 1500ohm



Figura 3.Divisor resistivo modificado

Con el cambio de estos componentes tendríamos una amplitud de 1,5V reduciendo de esta forma la resolución más adelante en la conversión realizada por el microcontrolador.

Offset a la salida:

Teoría:

Hasta ahora lo único que hicimos es reducir la amplitud de la señal a una donde la totalidad de la señal quede dentro de los parámetros de conversión del Arduino. Sin embargo, la señal sigue teniendo una componente negativa imposibilitando que se realice todavía una medición de la señal ya que se podría dañar el dispositivo. 

Para solucionar este problema decidimos implementar un circuito que utilizaría un amplificador operacional para realizar este desplazamiento de la señal ya que el Arduino cuenta con los conectores para alimentarlo. 

Decidimos utilizar un LM324 que es capaz de ser alimentado desde un rango de 0V a 3,3V hasta 36V, o por otro lado utilizar una alimentación negativa que permite alimentarlo con +-16V. Al alimentarlo con 5V desde el Arduino tendríamos el valor justo a la salida para tener la máxima resolución posible admitida por el conversor. 



Figura 4.Circuito de offset

En la entrada input iría la señal atenuada en amplitud anteriormente en el divisor resistivo, con las resistencias R5 y R6 configuraríamos la ganancia del amplificador en 1 ya que son las dos iguales mientras que con R3 y R4 haríamos un divisor resistivo para tener 2,5V en el terminal positivo del amplificador. 

Practica:

1°er prueba:

Cuando realizamos el circuito y realizamos mediciones con un osciloscopio nos encontramos de que el offset de la señal terminaba siendo de 3,8V y no de 2,5V como nosotros esperábamos. Investigando nos dimos cuenta que la configuración del amplificador no estaba siendo como un amplificador inversor partiendo de una tensión inicial de 2,5V sino que estaba trabajando como un amplificador operacional diferencial dándonos una ganancia diferente por la configuración de las resistencias.

Si rehacemos los cálculos teniendo en cuenta la ecuación de la señal de salida para dicha configuración nos queda como:

R6=R5=10Kohm   R3=R4=1Kohm   VIN=0V   VOUT=?

VOUT = [1+(R5/R6)] * [R4/(R3+R4)] * 5V – (R5/R6) * VIN

VOUT = [1+(10K/10K)] * [1K/(1K+1K)] * 5V – (10K/10K) * 0V = 5V



Figura 5.Salida del circuito de offset (Esc. V=1V/div)

Como vimos recién la señal debería de empezar en los 5V según el arreglo de las resistencias, pero en el osciloscopio vemos que empieza en los 3,8V y se satura rápidamente en este valor.

Esto se debe a que como estamos alimentando el amplificador operacional con 0V y 5V este recorta la señal por las limitaciones de operación que se le generan por utilizar un rango de alimentación tan bajo ocasionando que el rango de trabajo sea más acotado. 

Para solucionar estos problemas primero realizamos el cálculo para hacer un arreglo de resistencias que nos permita empezar desde los 2,5V y no desde los 3,8V que medimos con el osciloscopio. 

Considerando:   R6=R5=10Kohm   R4=1Kohm   VIN=0V   VOUT=2,5V   R3=?   

VOUT = [1+(R5/R6)] * [R4/(R3+R4)] * 5V – (R5/R6) * VIN

2,5V = [1+(10K/10K)] * [1K/(R3+1K)] * 5V – (10K/10K) * 0V

2,5V = 2 * [1K/(R3+1K)] * 5V

R3 = (2 * 1K * 5V)/ 2,5V - 1K = 3Kohm

Una vez que realizamos este cálculo modificamos la R3 por el valor obtenido y procedimos a verificar con el osciloscopio.

2°da prueba:

Mientras medimos con el osciloscopio nos percatamos que la señal medida no era la deseada ya que se nos recortaba la señal en el pico superior y en el pico inferior debido a la limitación de alimentación de amplificador.



Figura 6.Salida del circuito de offset 1°er modificación (Esc. V=1V/div)

Debido a este suceso y para no perder resolución de la señal cuando el conversor realice las conversiones optamos por reducir la amplitud de la misma para que quede dentro del rango de operación optimo del amplificador.

Considerando que el valor máximo era 3,8V y el mínimo era 0,8V pudimos deducir que la amplitud pico a pico de la señal entrante al amplificador deberá de ser de 3V o una amplitud normal de 1,5V. Por este motivo tuvimos que realizar cambios en el divisor resistivo como explicamos anteriormente en el apartado de “practica” de “reducción de amplitud”.

También debimos modificar el offset de la señal ya que, si partiera en 2,5V se seguiría recortando la señal por lo que procedimos a realizar el cálculo:

Cuando:

VIN=0V VOUT=0,8V + 1,5V = 2,3V (valor de mínimo inferior más la amplitud de la señal)

Entonces:   R6=R5=10Kohm   R4=1Kohm   VIN=0V   VOUT=2,3V   R3=?   

VOUT = [1+(R5/R6)] * [R4/(R3+R4)] * 5V – (R5/R6) * VIN

2,3V = [1+(10K/10K)] * [1K/(R3+1K)] * 5V – (10K/10K) * 0V 

2,3V = 2 * [1K/(R3+1K)] * 5V

R3 = (2 * 1K * 5V)/ 2,3V - 1K = 3,3Kohm

Cuando implementamos este valor de resistencia observamos que la señal quedaba dentro de los parámetros establecidos, como vemos en la imagen. 



Figura 7.Circuito de offset con modificación final (Esc. V=1V/div)

Resumen: 

Por limitaciones en la alimentación del amplificador operacional perdimos un poco de resolución del conversor, pero gracias a este circuito de acondicionamiento de señal podremos leer esta señal que contenía componentes negativas utilizando un micro controlador que no era capaz de leer este tipo de señales.



Figura 8.Circuito 'Acondicionamiento de señal' completo

Configuración del Tiempo de Muestreo y Visualización

Objetivos del Sistema

Como parámetros fundamentales para el diseño del sistema de adquisición de datos, necesitábamos cumplir con varios requisitos críticos: lograr una frecuencia de muestreo precisa y estable que fuera completamente independiente de la ejecución del código principal, ser capaces de visualizar las señales en tiempo real con capacidad de análisis avanzado, y mantener una comunicación eficiente entre el Arduino y la computadora sin pérdida de datos.

A diferencia de versiones anteriores que utilizaban el Serial Plotter básico de Arduino, este proyecto implementa una aplicación de visualización personalizada desarrollada en C++ que corre en la PC. Esta aplicación no solo grafica las señales, sino que también calcula la Transformada Rápida de Fourier (FFT) en tiempo real, detecta armónicas, aplica filtros digitales, y muestra simultáneamente la señal temporal y su espectro frecuencial. Esta solución profesional nos permite trabajar con señales mucho más complejas y realizar análisis que serían imposibles con herramientas básicas.

El desafío principal radica en garantizar que el Arduino muestree la señal a una velocidad exacta de tres mil ochocientas cuarenta muestras por segundo. Esta frecuencia no fue elegida arbitrariamente: permite capturar señales de hasta mil novecientos veinte hertz según el teorema de Nyquist, y además coincide perfectamente con un baudrate estándar de comunicación serial (treinta y ocho mil cuatrocientos bits por segundo), lo que facilita la transmisión sin saturar el puerto.

Estrategia de Muestreo Determinístico

Para lograr una frecuencia de muestreo tan precisa, exploramos varias estrategias posibles. La primera opción sería modificar únicamente el prescaler del conversor ADC, pero esto no garantiza tiempos exactos ya que depende de cuándo el programa principal solicita la conversión. La segunda opción, más robusta, consiste en utilizar un timer interno del Arduino que genere interrupciones periódicas donde se active el ADC. La tercera alternativa, que terminamos implementando, combina el timer con el modo de conversión continua del ADC (free-running mode).

En el modo de conversión continua, el ADC no espera que el microcontrolador le solicite una lectura; en cambio, automáticamente inicia una nueva conversión apenas termina la anterior. Esto es crucial porque si el programa principal se bloquea procesando datos o enviándolos por serial, el ADC seguirá tomando muestras sin perder ninguna. El timer se encarga de establecer el ritmo exacto: cada vez que el contador del timer alcanza un valor específico, dispara al ADC para que inicie una conversión. De esta manera, logramos muestreo periódico completamente independiente del flujo del programa.

Configuración del Timer1

Decidimos utilizar el Timer1 del Arduino Mega porque es un timer de dieciséis bits, lo que nos da mucha flexibilidad para configurar frecuencias precisas. El Arduino funciona a dieciséis megahertz de reloj base, pero necesitamos un prescaler (divisor de frecuencia) para poder contar a una escala manejable.

El sistema calcula automáticamente el prescaler óptimo según la frecuencia deseada. Para una frecuencia de muestreo de 3840 Hz, el algoritmo determina que se puede usar prescaler 1 (sin división), operando el timer directamente a dieciséis megahertz. Luego, configuramos el timer en modo CTC (Clear Timer on Compare Match), donde el contador se reinicia automáticamente al llegar a un valor de comparación específico (OCR1A = 4166). Este valor se calcula con la fórmula: OCR1A = (F_CPU / (prescaler × frecuencia)) - 1.

¿Cómo funciona internamente?

El microcontrolador tiene un registro contador (TCNT1) que se incrementa automáticamente con cada pulso del reloj (16 MHz). Cuando este contador alcanza el valor de comparación que configuramos (OCR1A = 4166), se genera una señal de hardware que dispara el ADC. El contador se resetea instantáneamente a cero y comienza a contar nuevamente. Este ciclo de 4167 pulsos a 16 MHz produce exactamente 260.4 microsegundos por muestra, logrando así los 3840 Hz deseados. Este proceso es completamente automático y consume recursos mínimos del procesador.

Optimización del ADC: Reducción de 10 a 8 bits

El Arduino Mega tiene un conversor ADC de diez bits, capaz de distinguir mil veinticuatro niveles diferentes de voltaje. Sin embargo, transmitir diez bits por el puerto serial requiere enviar dos bytes por cada muestra, lo que duplica el ancho de banda necesario. A tres mil ochocientas cuarenta muestras por segundo, esto sobrepasaría fácilmente la capacidad de baudrates estándares.

Descubrimos una característica poco conocida del ADC llamada ADLAR (ADC Left Adjust Result). Normalmente, los diez bits de la conversión se almacenan "justificados a la derecha" en dos registros de ocho bits: los dos bits más significativos van al registro ADCH y los ocho restantes al ADCL. Esto obliga a leer ambos registros en secuencia, y si el ADC completa una nueva conversión entre las dos lecturas, se puede corromper el dato.

Activando ADLAR, logramos que los diez bits se alineen "a la izquierda": los ocho bits más significativos quedan completos en ADCH, y los dos menos significativos (que contienen muy poca información útil) quedan en ADCL. De esta forma, podemos leer únicamente ADCH y obtener directamente un byte con los ocho bits más importantes de la conversión. Esto reduce el tráfico serial a la mitad sin perder precisión significativa, ya que el ruido inherente del circuito acondicionador ya limita la resolución efectiva.

Análisis de pérdida:

Los dos bits que descartamos representan aproximadamente doce milivoltios de resolución en el rango del ADC. Sin embargo, el ruido del amplificador operacional LM324 que usamos para acondicionar la señal ya introduce variaciones de alrededor de cuarenta milivoltios RMS. Por lo tanto, esos dos bits extra solo estarían capturando ruido, no información útil de la señal. La decisión de usar ocho bits es un equilibrio inteligente entre precisión y eficiencia.

Comunicación Serial de Alta Velocidad

Con ocho bits por muestra y tres mil ochocientas cuarenta muestras por segundo, necesitamos transmitir aproximadamente treinta mil setecientos veinte bits de datos puros. Sin embargo, cada byte transmitido por serial lleva bits adicionales (un bit de inicio y uno de parada), multiplicando el requerimiento por diez. Esto nos da trescientos siete mil doscientos bits por segundo como mínimo necesario.

El baudrate estándar más cercano es treinta y ocho mil cuatrocientos baudios, que en realidad nos da capacidad para transmitir tres mil ochocientos cuarenta bytes por segundo, justo lo que necesitamos. Esta sincronización perfecta no es casualidad: diseñamos la frecuencia de muestreo precisamente para que encajara con un baudrate estándar, evitando así configuraciones especiales o problemas de compatibilidad.

Problemas iniciales encontrados:

Al principio intentamos trabajar con nueve mil seiscientos baudios porque era más común y simple. Sin embargo, nos encontramos con un problema grave: los datos llegaban a la PC pero se veían símbolos extraños y caracteres incomprensibles en lugar de números. El Arduino estaba enviando datos más rápido de lo que la velocidad serial permitía transmitir, por lo que el buffer de transmisión se llenaba y el programa se bloqueaba esperando espacio libre. Esto causaba que se perdieran muestras y el timing del ADC se desincronizaba.

Subimos la velocidad a treinta y ocho mil cuatrocientos y el problema desapareció completamente. Ahora cada byte se transmite en aproximadamente doscientos sesenta microsegundos, exactamente el tiempo que tarda el ADC en tomar la siguiente muestra. El flujo es perfecto: mientras el ADC está convirtiendo, el byte anterior se está transmitiendo por serial. No hay cuellos de botella.

Deshabilitación de Entradas Digitales

Una optimización importante pero fácil de pasar por alto: el Arduino Mega tiene pines que pueden funcionar tanto como entradas analógicas (ADC) como digitales. Cuando un pin está configurado para ADC pero su circuito digital sigue activo, esto introduce ruido y consumo innecesario de corriente.

Los registros DIDR0 y DIDR2 (Digital Input Disable Register) permiten desconectar completamente los buffers digitales de los pines analógicos. Al escribir unos en estos registros, deshabilitamos las entradas digitales de todos los canales del ADC que no vamos a usar. Esto reduce el ruido de conversión y mejora ligeramente la precisión del ADC.

Inicialmente no lo implementamos y notábamos un nivel de ruido base más alto de lo esperado. Después de investigar en la datasheet del microcontrolador ATmega2560, descubrimos esta característica. Al habilitar DIDR, el ruido se redujo aproximadamente un treinta por ciento, mejorando la calidad de la señal capturada.

Arquitectura de Visualización Avanzada

A diferencia de sistemas simples que solo grafican valores en el Serial Plotter de Arduino, nuestra aplicación implementa una arquitectura de doble ventana con procesamiento en tiempo real:

Ventana de dominio temporal: Muestra la señal tal como llega del ADC, punto por punto, en un gráfico que se desplaza continuamente. Podemos ver la forma de onda completa con un segundo de historia (tres mil ochocientas cuarenta puntos). Esto es útil para verificar que la señal esté limpia, sin distorsiones, y para observar fenómenos transitorios.

Ventana de dominio frecuencial: Mientras los datos llegan, un hilo paralelo de procesamiento va acumulando muestras en un buffer circular. Cuando se completa un segundo de datos, se ejecuta la FFT que convierte esos puntos del dominio temporal al espectro frecuencial. El resultado se muestra en un gráfico donde el eje horizontal representa frecuencias (de cero a mil novecientos veinte hertz) y el eje vertical la amplitud de cada componente frecuencial.

Detección automática de armónicas: Un algoritmo busca picos en el espectro y automáticamente identifica las cinco primeras armónicas de la señal. Calcula la frecuencia fundamental, la amplitud de cada armónica, y el THD (Distorsión Armónica Total), que es un indicador de cuánta energía hay en las armónicas en comparación con la fundamental. Esto es crucial para análisis de calidad de señal.

Flujo Completo del Sistema

En el Arduino:

El Timer1 cuenta continuamente y cada doscientos sesenta microsegundos genera una señal

Esta señal activa el ADC que inicia una conversión de diez bits

Aproximadamente cien microsegundos después, el ADC completa la conversión

Se genera una interrupción que lee el registro ADCH (ocho bits MSB)

El byte se coloca en el buffer de transmisión serial

El hardware serial transmite el byte automáticamente en segundo plano

El proceso se repite, mientras el programa principal puede hacer otras tareas

En la PC:

Un hilo lee continuamente el puerto serial, byte por byte

Cada byte se convierte de valor ADC a voltaje real (invirtiendo la transformación del LM324)

El voltaje se almacena en dos buffers circulares: uno para visualización temporal, otro para FFT

La interfaz gráfica dibuja la señal temporal sesenta veces por segundo (actualización fluida)

Cada segundo, cuando el buffer FFT está completo, se ejecuta la transformada

El espectro resultante se grafica y se buscan automáticamente picos (armónicas)

Si hay filtros activos, se aplican muestra por muestra y los resultados se envían de vuelta al Arduino para salida DAC



Figura 9. Circuito final

Desarrollo DAC

Investigación:

Como en trabajos anteriores utilizamos la placa de desarrollo Arduino Mega 2560 decidimos seguir utilizándola pero que sin embargo está limitada a la hora de cumplir con el objetivo. Los módulos o dispositivos que permiten recrear señales son conversores digitales analógicos y esta placa no cuenta con este sistema por lo que para que pueda cumplir con esta tarea se necesita complementarlo con algún hardware externo.

En el caso de la plataforma de desarrollo de Arduino existen módulos externos que permiten la conversión digital a analógico mediante una correcta conexión a la placa de desarrollo y una implementación de un código acorde.

En primera instancia investigamos sobre los diferentes módulos disponibles, estos nos permitían generar señales con un límite de frecuencia muy elevado y con formas muy diversas pero el problema que nos encontramos es que eran muy caros. Por ende, decidimos que no era viable realizar un gasto tan elevado para cumplir con los requisitos de la actividad y que debíamos buscar otras alternativas más asequibles donde. Es así como consideramos la posibilidad de confeccionar nosotros mismos en el protoboard un circuito que nos permita cumplir con el objetivo con el fin de abaratar costos.

En la búsqueda de circuitos capaces de cumplir nos encontramos con diferentes tipos que utilizaban deferentes componentes y circuitos integrados, es allí donde nos topamos con un circuito muy interesante que nos permitía convertir una señal digital a una analógica utilizando solamente resistencias lo que lo hacía sumamente asequible.

El nombre de dicho circuito era R2R y mediante la suma de la caída de tensiones en las resistencias podemos recrear la señal que queramos.



Ilustración 1. Circuito R2R con resolución de 8 bits

El circuito ira conectado a salidas digitales del Arduino y utilizara cuantos pines digitales como cuanta resolución quiera el usuario a la hora de recrear las señales. Cuantos más pines utilicemos a la salida mejor se vera la señal de salida sin embargo si embargo esto puede ser una desventaja en placas de desarrollo que no cuente con una cantidad de pines digitales y que tengan que realizar otras tareas además de la recreación de señales. También en cuanto a amplitud nos encontramos que la máxima de la señal generada serán 5V que es a tensión máxima de los pines del Arduino por lo que si se querrá generar señales con una amplitud mayor a 5V se deberá realizar modificaciones en el circuito o directamente construir uno diferente. A su vez como alimentamos el circuito R2R con la tensión de los pines digitales también no podremos generar señales negativas o que tengan una parte en tensión negativa.

Funcionamiento Circuito R2R:

Cuando analizamos la señal que tenemos a la salida nos encontramos que la señal es la suma de las caídas de tensiones en las resistencias y que cada una es la mitad de que la anterior.



Ilustración 2. Circuito original

Para realizar la explicación de cómo funciona en detalle el circuito vamos a utilizar el teorema de Thévenin y el teorema de superposición ya que nos interesa la tensión en los puntos A, B, C y D.



Ilustración 3. Circuito de 1°er análisis del bit menos significativo

Procedemos a realizar la simplificación a Thévenin y nos encontramos con la simplificación del circuito donde: 





Ilustración 4. Circuito de 1°era simplificación

Después de realizar la primera simplificación observamos que la tención es la mitad que la original pero todavía quedan realizar tres simplificaciones mas para obtener la tensión de salida.



Ilustración 5. Circuito de 2°do análisis

Aislamos nuevamente la sección remarcada en rojo y procedemos a realizar los cálculos donde obtenemos que:





Ilustración 6. Circuito de 2°da simplificación

Al realizar un segundo calculo ya vemos que la tensión es una cuarto de la original porque lo que ya podemos ver la tensión seguirá disminuyendo hasta llegar al punto de salida (OUTPUT) al final del circuito.



Ilustración 7. Circuito de 3°er anáisis



Ilustración 8. Circuito de 3°era simplificación



Ilustración 9. Circuito de 4°to análisis



Ilustración 10. Circuito de 4°ta simplificación



Ilustración 11. Circuito final equivalente con bit menos significativo en alto

Como se vio en todo el proceso desde la ilustración 3 a la ilustración 11 la tensión original va disminuyendo hasta ser 1/16 de la original. Este proceso es parecido en el caso de las otras tensiones lo único que cambia es la atenuación que se sufre. 

A continuación, mostramos de ejemplo el caso de V3 para ilustrar otra variante del proceso:



Ilustración 12. Circuito de análisis bit más significativo en alto



Ilustración 13. Circuito de la Ilustración 12 simplificado

Desarrollo DSP completo:

Una vez que adaptamos la señal de entrada a un rango aceptable para que el Arduino lo pueda leer con el ADC y que después la pueda recrear con el circuito R2R que funcionaria como DAC procedemos a realizar la programación para que funcione en conjunto con la PC, agregando funciones de filtrado y muestreo de datos en una interface para poder interpretar los datos de la señal y recrearla o modificarla a la salida.

Arquitectura General del Sistema:



Especificaciones:

Conversor Analógico-Digital (ADC)

El Arduino Mega 2560 incorpora un ADC de aproximaciones sucesivas (SAR) con las siguientes características:

Especificaciones técnicas:

Resolución nominal: 10 bits (1024 niveles)

Resolución de transmisión: 8 bits (256 niveles) - optimización de baudrate

Rango de entrada ADC: 0V - 5V (referencia AVCC)

Rango efectivo de señal: 0.8V - 3.8V (3.0V span, acondicionado por LM324)

Frecuencia de muestreo: 3840 Hz

Tiempo de conversión: ~104 μs por muestra

Configuración del prescaler: 128 (balance velocidad/precisión)

Configuración de registros:

// Configuración básica del ADC (ver adc.cpp para detalles completos)

ADMUX = (1 << REFS0) | (1 << ADLAR) | canal;    // Ref: AVcc (5V), ADLAR, canal A1

ADCSRA = (1 << ADEN) | (7 << ADPS0);  // Enable ADC, prescaler /128

Cálculo de frecuencia del ADC:

F_ADC = F_CPU / Prescaler = 16 MHz / 128 = 125 kHz

Tiempo de conversión = 13 ciclos × (1/125kHz) = 104 μs

Frecuencia máxima teórica = 125kHz / 13 ≈ 9.6 kHz

Conversión de datos 10→8 bits mediante ADLAR (Left Adjust):

Resultado del ADC Ajustado a la Izquierda es una técnica eficiente de conversión a nivel de software para adaptar la lectura de 10bits a 8bits y que sea compatible a la hora de utilizarla en otros procesos como su recreación con el DAC:

// Configuración del ADC con ADLAR (alineación izquierda)

void ADCController::begin(int pin) {

    ADMUX = AVcc | AJUSTAR_IZQUIERDA | pin;  // ADLAR = 1

    // ...

}



// Lectura directa de 8 bits más significativos

void ADCController::conversion_complete() {

    uint8_t high = ADCH;  // Solo lee registro alto (8 bits MSB)

    data = high;          // Descarta automáticamente 2 bits LSB

}



uint8_t ADCController::get() {

    return data;  // Retorna [0-255] directamente

}

Distribución de bits con ADLAR:

Conversión 10 bits: 0b11_1010_0101 (933 decimal)



Registro ADCH:  [b9][b8][b7][b6][b5][b4][b3][b2] = 233 (8 bits altos)

Registro ADCL:  [b1][b0][0][0][0][0][0][0] = ignorado



Resultado: 933 >> 2 = 233 ✓

Conversor Digital-Analógico (DAC)

La salida utiliza un DAC R2R de 8 bits mediante salidas digitales paralelas:

Especificaciones:

Resolución: 8 bits (256 niveles)

Tipo: DAC resistivo R-2R ladder network

Pines utilizados: 22-29 (PORTA completo del Arduino Mega 2560)

Rango de salida: 0V - 5V

Tiempo de establecimiento: ~1 μs (escritura atómica)

Métricas de Desempeño

Precisión (Resolución)

La precisión se define como la menor variación de entrada que el sistema puede detectar.

ADC (10 bits nativos sobre rango efectivo):

Resolución_ADC = Rango_efectivo / (2^n) = 3.0V / 1024 = 2.93 mV por LSB

Transmisión (8 bits efectivos sobre rango efectivo):

Resolución_transmisión = 3.0V / 256 = 11.72 mV por LSB

DAC R2R (8 bits, rango completo 0-5V):

Resolución_DAC = 5V / 256 = 19.53 mV por LSB

Tabla Resumen de Especificaciones

Parámetro

Valor

Unidad

Observaciones

Resolución ADC

10

bits

1024 niveles nativos

Resolución efectiva

8

bits

Limitada por serial y DAC

Rango de entrada

-6 a +6

V

Señal original

Rango ADC efectivo

0.8 - 3.8

V

Post-acondicionamiento LM324

Precisión (en ADC)

11.72

mV

Sobre rango efectivo

Frec. muestreo

3840

Hz

Nyquist: 1920 Hz

Latencia total

~1.0

ms

ADC + serial + filtro

1. FUNDAMENTOS TEÓRICOS - TRANSFORMADAS DE FOURIER 

1.1.1 Del Dominio Temporal al Dominio Frecuencial

Una señal en el tiempo x(t) puede representarse como la suma de componentes sinusoidales de diferentes frecuencias. La Transformada de Fourier es la herramienta matemática que realiza esta descomposición.

Transformada de Fourier Continua:



Interpretación física:

X(f) representa la amplitud y fase de cada componente frecuencial

Permite identificar qué frecuencias están presentes en la señal

Fundamental para análisis espectral, filtrado y compresión

1.1.2 Transformada Discreta de Fourier (DFT)

Para señales digitales muestreadas, se utiliza la DFT:



Donde:

x[n]: Muestras temporales (N puntos)

X[k]: Coeficientes espectrales (N/2 frecuencias útiles)

k: Índice de frecuencia (0 a N-1)

N: Número total de muestras

Conversión índice → frecuencia:



Donde fs​ es la frecuencia de muestreo. 

Explicación paso a paso de cómo funciona la DFT:

Hipotéticamente tenemos una señal digital con N = 8 muestras tomadas a fs = 3840 Hz:

Paso 1: Tomar las muestras en el tiempo

x[0] = 1.2 V  (t = 0.00 ms)

x[1] = 0.8 V  (t = 0.26 ms)

x[2] = -0.3 V (t = 0.52 ms)

... y así sucesivamente

Paso 2: Aplicar la fórmula DFT para cada frecuencia k

Para k = 0 (componente DC):

X[0] = x[0]×e^0 + x[1]×e^0 + ... + x[7]×e^0

     = x[0] + x[1] + ... + x[7]  // Suma simple

     = Promedio × N = Offset DC

Para k = 1 (frecuencia fundamental = fs/N = 480 Hz):

X[1] = x[0]×e^(-j2π×1×0/8) + x[1]×e^(-j2π×1×1/8) + ...

     = x[0]×1 + x[1]×e^(-jπ/4) + x[2]×e^(-jπ/2) + ...

Cada término e^(-j2πkn/N) es un "factor de rotación" que compara la señal con senos y cosenos de frecuencia k.

Paso 3: Convertir resultado complejo a magnitud

Cada X[k] es un número complejo con parte real e imaginaria:

X[k] = a + jb  // a = parte real, b = parte imaginaria



Magnitud = √(a² + b²)  // Amplitud de la frecuencia k

Fase = arctan(b/a)     // Fase de la frecuencia k

Paso 4: Mapear índice k a frecuencia real

k = 0  →  f = 0 Hz      (DC)

k = 1  →  f = 480 Hz    (fundamental para N=8, fs=3840)

k = 2  →  f = 960 Hz

k = 3  →  f = 1440 Hz

k = 4  →  f = 1920 Hz   (Nyquist)

Interpretación física: La DFT descompone la señal temporal en N/2 componentes sinusoidales de diferentes frecuencias, calculando cuánta "energía" hay en cada frecuencia.

1.1.3 FFT (Fast Fourier Transform)

La FFT es un algoritmo eficiente para calcular la DFT, desarrollado por Cooley y Tukey (1965).

Complejidad computacional:

DFT directa: O(N2) operaciones

FFT: O(NlogN) operaciones

Ejemplo: Para N = 1024 muestras:

DFT: ~1,048,576 operaciones

FFT: ~10,240 operaciones (~100× más rápida)

Algoritmo FFT por decimación en frecuencia:

1. Dividir la secuencia en pares e impares

2. Calcular FFT de cada mitad recursivamente

3. Combinar resultados mediante factores de rotación (twiddle factors)

   W_N^k = e^(-j2πk/N)

¿Cómo funciona la optimización FFT?

La FFT aprovecha simetrías matemáticas para evitar cálculos redundantes:

Ejemplo con N = 8 muestras:

Método directo (DFT):

Para calcular X[0]: 8 multiplicaciones complejas

Para calcular X[1]: 8 multiplicaciones complejas

...

Total: 8 × 8 = 64 multiplicaciones

Método FFT:

División: Separar en pares e impares

Pares:   x[0], x[2], x[4], x[6]  → FFT de 4 puntos

Impares: x[1], x[3], x[5], x[7]  → FFT de 4 puntos

Conquista: Resolver dos FFT de N/2 = 4 puntos

FFT de 4 puntos requiere 4×4 = 16 operaciones cada una

Total: 2 × 16 = 32 operaciones

Combinación: Unir resultados con N = 8 multiplicaciones

Total, final: 32 + 8 = 40 operaciones (vs 64 de DFT)

Recursión continua: Para N = 1024, la FFT divide hasta llegar a pares de 1 elemento:

1024 → 512 → 256 → 128 → 64 → 32 → 16 → 8 → 4 → 2 → 1

        log₂(1024) = 10 niveles de recursión

Resultado: En lugar de N² = 1,048,576 operaciones, solo necesita N×log₂(N) = 10,240 operaciones.

Analogía: Es como buscar un nombre en una agenda:

DFT: Revisar página por página (lento)

FFT: Abrir por la mitad y decidir si está antes o después (rápido)

1.2 Teorema de Muestreo de Nyquist

Teorema: Una señal con frecuencia máxima fmax​ puede reconstruirse perfectamente si se muestreo a: 



En nuestro sistema:

Frecuencia de muestreo: fs=3840 Hz

Frecuencia máxima útil: fmax=1920 Hz (frecuencia de Nyquist)

Frecuencias por encima de 1920 Hz aparecerán como aliasing

Explicación intuitiva del Teorema de Nyquist:

Imagina que quieres dibujar una onda senoidal a mano, pero solo puedes marcar puntos discretos en el papel.

Ejemplo 1: Muestreo adecuado (fs = 4×f), señal de 1 Hz, muestreada a 4 Hz (4 puntos por ciclo):



Resultado: Puedes reconstruir la onda conectando los puntos.

Ejemplo 2: Muestreo mínimo (fs = 2×f) - Límite de Nyquis, señal de 1 Hz, muestreada a 2 Hz (2 puntos por ciclo):



Resultado: Justo alcanza para reconstruir (2 puntos/ciclo) 

Ejemplo 3: Submuestreo (fs < 2×f) - ¡ALIASING! Señal de 3 Hz, muestreada a 4 Hz (menos de 2 puntos por ciclo): 



Resultado: ¡Se ve como 1 Hz en lugar de 3 Hz! (aliasing)

En el sistema DSP-Arduino: Con fs = 3840 Hz:

Frecuencias válidas:  0 Hz ─────────────────► 1920 Hz

                          │                       │

                          DC                   Nyquist              

Frecuencias que causan aliasing: > 1920 Hz

Ejemplo de aliasing:

Señal real a 2500 Hz → Se ve como 1380 Hz en el espectro

Cálculo: |2500 - 3840| = 1340 Hz (alias)

¿Por qué importa para nuestro proyecto?

Si analizamos una señal que contiene frecuencias > 1920 Hz (como armónicas altas), esas frecuencias aparecerán "reflejadas" en el espectro FFT, contaminando el análisis.

1.3 Armónicas y Análisis Espectral

1.3.1 Concepto de Armónicas

Para una señal periódica con frecuencia fundamental f0​, las armónicas son componentes frecuenciales a múltiplos enteros:

Fundamental: f1​=f0​ (1ª armónica)

Segunda armónica: f2​=2⋅f0​

Tercera armónica: f3​=3⋅f0​

n-ésima armónica: fn​=n⋅f0​

Ejemplo: Para una señal con fundamental a 440 Hz:

f1​=440 Hz (fundamental)

f2​=880 Hz (octava)

f3​=1320 Hz (quinta + octava)

1.3.2 Series de Fourier

Cualquier señal periódica puede expresarse como suma de senos y cosenos:



Donde:

A0​: Componente DC (offset)

An Bn​: Amplitudes de cada armónica

Forma compleja (más compacta):



Explicación intuitiva: Construyendo una onda cuadrada

Las Series de Fourier nos dicen que cualquier forma de onda se puede construir sumando senos y cosenos de diferentes frecuencias.

Ejemplo paso a paso: Onda cuadrada a 100 Hz

Una onda cuadrada perfecta se construye sumando solo armónicas impares:

Paso 1: Solo la fundamental (1ª armónica)

Amplitud: A₁ = 4/π ≈ 1.273 V

Frecuencia: 100 Hz

Resultado: senoidal pura, no se parece a cuadrada.

Paso 2: Fundamental + 3ª armónica

A₁ = 4/π       @ 100 Hz

A₃ = (4/π)/3   @ 300 Hz

Resultado:  empieza a "aplanar" arriba y abajo.

Paso 3: Hasta 5ª armónica

A₁ = 1.273 V @ 100 Hz

A₃ = 0.424 V @ 300 Hz

A₅ = 0.255 V @ 500 Hz

Resultado:  cada vez más cuadrada

Paso 4: Hasta 15ª armónica

Suma de armónicas impares: 1, 3, 5, 7, 9, 11, 13, 15

Resultado: empieza a verse la cuadrada con ligeras distorsiones en las puntas.



Verificación matemática:

x(t) = (4/π) × [sin(2π×100t) 

              + (1/3)×sin(2π×300t) 

              + (1/5)×sin(2π×500t) 

              + (1/7)×sin(2π×700t) 

              + ...]

¿Por qué solo impares?

Las armónicas pares (2, 4, 6...) se cancelan por simetría

Las armónicas impares (1, 3, 5...) suman constructivamente

Aplicación en nuestro proyecto:

Cuando el sistema detecta estas armónicas en el espectro FFT:

Espectro de onda cuadrada @ 100 Hz:

  100 Hz → 1.273 V  ← Fundamental

  200 Hz → 0.000 V  ← Par (ausente)

  300 Hz → 0.424 V  ← 3ª armónica

  400 Hz → 0.000 V  ← Par (ausente)

  500 Hz → 0.255 V  ← 5ª armónica

  ...

Esta "huella digital" de armónicas permite identificar la forma de onda sin verla directamente.

1.3.3 Distorsión Armónica Total (THD)

La Distorsión Armónica Total (THD) mide la pureza de una señal:



Donde An​ es la amplitud de la n-ésima armónica.

Interpretación:

THD < 1%: Señal muy pura (audio Hi-Fi)

THD 1-5%: Calidad aceptable

THD > 10%: Distorsión audible

¿Qué significa realmente el THD?

El THD responde a la pregunta: "¿Qué porcentaje de la energía de la señal NO está en la frecuencia fundamental?"

Ejemplo numérico completo:

Supongamos que medimos una señal de 440 Hz:

A₁ = 1.000 V  @ 440 Hz   (fundamental)

A₂ = 0.100 V  @ 880 Hz   (2ª armónica)

A₃ = 0.050 V  @ 1320 Hz  (3ª armónica)

A₄ = 0.020 V  @ 1760 Hz  (4ª armónica)

Cálculo paso a paso:

Paso 1: Calcular la energía de las armónicas no fundamentales

Energía_armónicas = √(A₂² + A₃² + A₄²)

                  = √(0.100² + 0.050² + 0.020²)

                  = √(0.01 + 0.0025 + 0.0004)

                  = √0.0129

                  = 0.1136 V

Paso 2: Dividir por la fundamental

THD = Energía_armónicas / A₁ × 100%

    = 0.1136 / 1.000 × 100%

    = 11.36%

Interpretación del resultado:

El 11.36% de la "energía" de la señal está en armónicas indeseadas

Un generador de tonos puro debería tener THD < 1%

Este valor (11.36%) indica distorsión moderada

Comparación de formas de onda típicas:

Forma de onda

THD teórico

Armónicas presentes

Senoidal perfecta

0%

Solo fundamental

Senoidal con ruido

0.5-2%

Todas (bajo nivel)

Triangular

12%

Impares (decaen rápido)

Cuadrada 50%

48.3%

Impares (1/n)

Diente de sierra

30%

Todas (1/n)

Aplicación práctica en el proyecto:

// El código calcula THD automáticamente:

std::vector<Harmonic> harmonics = fft->FindHarmonics(3);



double sum_squares = 0;

for (int i = 1; i < harmonics.size(); i++) {  // i=1 salta la fundamental

    sum_squares += harmonics[i].amplitude * harmonics[i].amplitude;

}



double thd = sqrt(sum_squares) / harmonics[0].amplitude * 100.0;

¿Por qué es importante?

Audio: THD > 1% es audible como "dureza" o "distorsión"

Comunicaciones: THD alto causa interferencia en canales adyacentes

Control de calidad: Los generadores de señales especifican THD < 0.05%

Diagnóstico: THD alto indica problemas en amplificadores o fuentes

2. IMPLEMENTACIÓN FFT Y DETECCIÓN DE ARMÓNICAS 

2.1 Biblioteca FFTW3

El sistema utiliza FFTW3 (Fastest Fourier Transform in the West), una biblioteca especializada que calcula la Transformada Rápida de Fourier con máxima eficiencia.

¿Qué es FFTW3?

FFTW3 es una implementación optimizada del algoritmo FFT que convierte señales del dominio temporal (voltaje vs tiempo) al dominio frecuencial (amplitud vs frecuencia). Es utilizada internamente por software profesional como MATLAB, Octave, NumPy/SciPy, y muchas aplicaciones de audio.

Características principales:

Optimización SIMD: Utiliza instrucciones especiales del procesador (SSE, AVX) que procesan múltiples datos simultáneamente, acelerando cálculos hasta 8× respecto a código normal.

Planes precomputados: Analiza el tamaño de datos una sola vez y genera una estrategia optimizada ("plan") que reutiliza en cálculos posteriores.

Simetría de Hermite: Para señales reales (como voltajes), aprovecha que el espectro es simétrico. Solo calcula mitad de las frecuencias, ahorrando 50% de tiempo y memoria.

Inicialización del sistema FFT:

Cuando se crea el objeto FFT, se reserva memoria y se genera el plan de ejecución:

FFT::FFT(int sample_count) {

    // 1. Calcular cuántos bins de frecuencia necesitamos

    amplitudes_size = sample_count / 2 + 1;  



    // 2. Reservar memoria para resultados complejos

    complex = (fftw_complex*)fftw_malloc(amplitudes_size * sizeof(fftw_complex));



    // 3. Crear plan de ejecución

    p = fftw_plan_dft_r2c_1d(sample_count, samples.data(), complex, FFTW_ESTIMATE);

}

Explicación línea por línea:

Línea 1: Cálculo del tamaño del espectro

amplitudes_size = sample_count / 2 + 1;

Si capturamos N=3840 muestras temporales, solo necesitamos calcular 3840/2 + 1 = 1921 frecuencias

¿Por qué? Por el teorema de Nyquist: solo podemos representar frecuencias hasta fs/2

El "+1" incluye la frecuencia 0 Hz (componente DC o promedio de la señal)

Línea 2: Reserva de memoria alineada

complex = (fftw_complex*)fftw_malloc(amplitudes_size * sizeof(fftw_complex));

fftw_malloc: Reserva memoria con alineación especial (16 bytes) requerida por instrucciones SIMD

fftw_complex: Cada frecuencia se representa como número complejo (parte real + parte imaginaria)

Tamaño: 1921 × 16 bytes = ~31 KB de memoria

Línea 3: Creación del plan de ejecución

p = fftw_plan_dft_r2c_1d(sample_count, samples.data(), complex, FFTW_ESTIMATE);

dft: Discrete Fourier Transform (transformada discreta de Fourier)

r2c: Real to Complex (entrada real → salida compleja)

1d: Unidimensional (para señales de audio/voltaje; existe 2D para imágenes)

FFTW_ESTIMATE: Modo rápido que usa heurísticas en lugar de benchmarks

Resultado: Un "plan" optimizado que se guardará para reutilizar en cada análisis

2.2 Proceso de Análisis FFT - Paso a Paso

El análisis FFT transforma señales temporales (voltios medidos cada fracción de segundo) en espectros frecuenciales (qué frecuencias existen en la señal y con qué amplitud). Este proceso se realiza en 5 pasos secuenciales:

PASO 1: Adquisición y Almacenamiento de Muestras

El Arduino captura voltajes con su ADC a una tasa constante (3840 muestras por segundo). Estas muestras llegan al PC vía puerto serial y se almacenan en un buffer circular que funciona como una "ventana deslizante" de 1 segundo.

Funcionamiento del buffer circular:

El sistema mantiene siempre 3840 muestras disponibles (exactamente 1 segundo de señal). Cuando llega una nueva muestra, entra por un extremo y desaloja la más antigua por el otro, como una cinta transportadora.

Código de almacenamiento:

void SerialWorker() {

    while (lectura_activa) {

        uint8_t valor_adc = serial.read();  // Leer 1 byte desde Arduino

        double voltaje = TransformarADC_a_Voltaje(valor_adc);  

        scrollY->push(voltaje);  // Agregar al buffer circular

    }

}

PASO 2: Preparación de Datos para FFT

Antes de calcular la FFT, copiamos los datos del buffer circular al array de entrada de FFTW3. Esta copia es necesaria porque FFTW3 necesita un array continuo en memoria y puede modificar los datos durante el cálculo.

void FFT::SetData(const double* data, uint32_t count) {

    // Copiar datos del buffer temporal al array de la FFT

    std::copy(data, data + count, samples.begin());



    // Si hay menos de N muestras, rellenar con ceros (zero-padding)

    if (count < samples_size) {

        std::fill(samples.begin() + count, samples.end(), 0);

    }

}

¿Qué hace esta función?

Toma las 3840 muestras más recientes del buffer circular

Las copia al array samples[] que usa FFTW3

Si faltaran muestras (poco común), rellena con ceros

PASO 3: Ejecución de la Transformada de Fourier

Este es el corazón del análisis. FFTW3 toma las 3840 muestras temporales y calcula el espectro de frecuencias.

void FFT::Compute() {

    // Ejecutar la transformada de Fourier

    fftw_execute(p);  // p es el "plan" creado en el constructor

}

¿Qué sucede internamente en fftw_execute(p)?

FFTW3 calcula para cada frecuencia k (de 0 a 1920 Hz) cuánta energía hay en la señal a esa frecuencia específica. Matemáticamente, aplica esta fórmula para cada k:



Donde:

x[n] = muestra temporal n (el voltaje en el instante n)

X[k] = coeficiente complejo de la frecuencia k

j = unidad imaginaria (√-1)

Resultado: Un array complex[] con 1921 números complejos. Cada número tiene:

Parte real: Cuánto de la frecuencia k está "en fase" con la señal

Parte imaginaria: Cuánto de la frecuencia k está "desfasada 90°"

PASO 4: Conversión a Magnitudes (Amplitudes)

Los números complejos son difíciles de interpretar directamente. Lo que nos interesa es la amplitud (qué tan fuerte es cada frecuencia), independientemente de su fase.

Para calcular la amplitud desde un número complejo, usamos el teorema de Pitágoras:



Código de conversión:

void FFT::Compute() {

    fftw_execute(p);  // Ya ejecutada en paso 3



    // Convertir números complejos a magnitudes

    for (int k = 0; k < amplitudes_size; k++) {

        double parte_real = complex[k][0];

        double parte_imag = complex[k][1];



        // Calcular magnitud (hipotenusa del triángulo)

        double magnitud = sqrt(parte_real * parte_real + parte_imag * parte_imag);



        // Normalizar dividiendo por N para obtener amplitud en voltios

        amplitudes[k] = magnitud / samples_size;

    }

}

¿Por qué dividir por samples_size (N=3840)?

La FFT suma todas las contribuciones de cada muestra. Si la señal es constante 1V, la suma sería 3840V. Dividiendo entre 3840 obtenemos el promedio verdadero: 1V.

Interpretación del array resultante:

amplitudes[0]   = Componente DC (promedio de la señal)      → 0 Hz

amplitudes[1]   = Amplitud a 1 Hz (un ciclo por segundo)    → 1 Hz

amplitudes[2]   = Amplitud a 2 Hz                           → 2 Hz

...

amplitudes[440] = Amplitud a 440 Hz (nota La musical)       → 440 Hz

...

amplitudes[1920] = Amplitud a 1920 Hz (frecuencia de Nyquist) → 1920 Hz

PASO 5: Detección de Frecuencia Dominante

Una vez calculadas todas las amplitudes, buscamos cuál frecuencia tiene la mayor amplitud (excluyendo la componente DC).

void FFT::Compute() {

    // ... (pasos anteriores)



    // Guardar componente DC

    offset = amplitudes[0];



    // Buscar la frecuencia con máxima amplitud (empezando desde k=1 para ignorar DC)

    n_frequency = 1;

    double amplitud_maxima = amplitudes[1];



    for (int k = 2; k < amplitudes_size; k++) {

        if (amplitudes[k] > amplitud_maxima) {

            amplitud_maxima = amplitudes[k];

            n_frequency = k;  // Guardar índice del pico

        }

    }

}

Conversión de índice a frecuencia en Hz:

El índice k corresponde a una frecuencia específica:



En nuestro caso particular, el índice coincide con la frecuencia en Hz porque fs = N = 3840.

Ejemplo completo con señal sinusoidal de 440 Hz:

Entrada: Tono puro 440 Hz, amplitud 1.0V

         ┌─┐     ┌─┐     ┌─┐

    1V  ─┤ └─────┘ └─────┘ └──

         └───────────────────────

    0V   



FFT calcula espectro:

    Amplitud

    │

    │             █  ← Pico en k=440

 1.0V│            ███

    │           █████

 0.5V│          ███████

    │      ░░░░███████░░░░

    └─────┴─────┴─────┴─────┴─────> Frecuencia (Hz)

          0    220   440   660   880



Resultado detectado:

  - Frecuencia dominante: 440 Hz

  - Amplitud: 0.98V (cercano al teórico 1.0V)

  - Offset DC: 0.00V (señal centrada en cero)

Resumen del flujo completo:

Muestras temporales (3840 valores de voltaje)

            ↓

   [Paso 1] Almacenar en buffer circular

            ↓

   [Paso 2] Copiar a array samples[]

            ↓

   [Paso 3] fftw_execute() → array complex[]

            ↓

   [Paso 4] Calcular magnitudes → array amplitudes[]

            ↓

   [Paso 5] Buscar pico máximo → frecuencia dominante

            ↓

Resultado: Espectro de frecuencias listo para visualizar

2.3 Detección de Frecuencia Dominante

Una vez calculado el espectro completo de frecuencias, el sistema identifica automáticamente cuál es la frecuencia principal (dominante) en la señal. Esta es típicamente la frecuencia fundamental de la señal que estamos midiendo.

Proceso de detección:

El algoritmo busca el bin con mayor amplitud en todo el espectro, excluyendo la componente DC (frecuencia 0 Hz):

// Código simplificado de detección

int indice_pico = 1;

double amplitud_maxima = amplitudes[1];



for (int k = 1; k < amplitudes_size; k++) {

    if (amplitudes[k] > amplitud_maxima) {

        amplitud_maxima = amplitudes[k];

        indice_pico = k;

    }

}



// Convertir índice a frecuencia real

frecuencia_dominante = indice_pico * fs / N;

Explicación del proceso:

Iniciar búsqueda en k=1: Se ignora k=0 porque corresponde a la componente DC (promedio de la señal), no a una frecuencia oscilante.

Recorrer todo el espectro: Se compara cada bin con el máximo actual encontrado.

Guardar el pico: Cuando se encuentra una amplitud mayor, se actualiza tanto la amplitud como el índice.

Convertir a Hz: El índice k se convierte a frecuencia multiplicando por la resolución frecuencial (fs/N).

Ejemplo práctico:

Supongamos que estamos midiendo una señal de 440 Hz:

Espectro calculado:

  amplitudes[0]   = 0.001 V  (DC, casi cero)

  amplitudes[1]   = 0.002 V

  amplitudes[2]   = 0.003 V

  ...

  amplitudes[439] = 0.025 V

  amplitudes[440] = 0.950 V  ← ¡MÁXIMO!

  amplitudes[441] = 0.018 V

  ...

  amplitudes[1920] = 0.001 V



Resultado:

  indice_pico = 440

  frecuencia_dominante = 440 * 3840 / 3840 = 440 Hz

  amplitud = 0.950 V

2.4 Detección de Armónicas

El sistema detecta automáticamente las 5 primeras armónicas de la señal de entrada. Las armónicas son frecuencias que son múltiplos enteros de la frecuencia fundamental.

¿Qué son las armónicas?

Las armónicas son componentes frecuenciales que aparecen naturalmente en señales no sinusoidales. Si la frecuencia fundamental es f₀, las armónicas aparecen en:

1ª armónica (fundamental): f₀

2ª armónica: 2 × f₀

3ª armónica: 3 × f₀

4ª armónica: 4 × f₀

5ª armónica: 5 × f₀

Ejemplo: Si f₀ = 100 Hz, las armónicas están en 100, 200, 300, 400 y 500 Hz.

Estructura de datos:

Cada armónica detectada contiene tres datos:

struct Harmonic {

    double frequency;   // Frecuencia en Hz

    double amplitude;   // Amplitud en Voltios  

    int bin_index;      // Posición en el espectro

};

Algoritmo de detección - Explicación paso a paso:

El algoritmo utiliza la frecuencia dominante como referencia y busca picos en las posiciones donde se esperan las armónicas.

PASO 1: Obtener frecuencia fundamental

Después de ejecutar la FFT, ya tenemos detectada la frecuencia dominante. Esta será nuestra frecuencia fundamental f₀.

Ejemplo: f₀ = 440 Hz (detectada como pico máximo)

PASO 2: Calcular frecuencias esperadas

Para cada armónica n (de 1 a 5), calculamos dónde deberíamos encontrar el pico:

1ª armónica: f₁ = 1 × 440 = 440 Hz   (fundamental)

2ª armónica: f₂ = 2 × 440 = 880 Hz

3ª armónica: f₃ = 3 × 440 = 1320 Hz

4ª armónica: f₄ = 4 × 440 = 1760 Hz

5ª armónica: f₅ = 5 × 440 = 2200 Hz  (si no excede Nyquist)

PASO 3: Convertir frecuencias a índices (bins)

Cada frecuencia esperada se convierte a un índice del array de amplitudes:

bin_esperado = round(frecuencia × N / fs)



Para f₂ = 880 Hz:

  bin₂ = round(880 × 3840 / 3840) = 880

PASO 4: Búsqueda con tolerancia (±3 bins)

En lugar de buscar exactamente en el bin calculado, el algoritmo busca el pico máximo en una ventana de ±3 bins alrededor. Esto compensa:

Resolución finita: Si la frecuencia real es 880.4 Hz pero la resolución es 1 Hz/bin, el pico puede estar ligeramente desplazado.

Spectral leakage: La energía puede distribuirse en bins vecinos.

// Código simplificado de búsqueda con tolerancia

int bin_objetivo = 880;  // Para 2ª armónica



int mejor_bin = bin_objetivo;

double mejor_amplitud = amplitudes[bin_objetivo];



// Buscar en ventana [877, 878, 879, 880, 881, 882, 883]

for (int bin = bin_objetivo - 3; bin <= bin_objetivo + 3; bin++) {

    if (amplitudes[bin] > mejor_amplitud) {

        mejor_amplitud = amplitudes[bin];

        mejor_bin = bin;

    }

}



// Guardar resultado

armonica.frequency = mejor_bin * fs / N;  // Frecuencia exacta encontrada

armonica.amplitude = mejor_amplitud;       // Amplitud del pico

armonica.bin_index = mejor_bin;            // Posición en el espectro

Visualización del proceso de búsqueda:

Buscando 2ª armónica (esperada en 880 Hz):



  Amplitud

    │

    │        877 878 879 880 881 882 883

    │         ↓   ↓   ↓   ↓   ↓   ↓   ↓

 0.4│         █   █   █   █   █   █   █

    │        ░░░░░░░░░░░░███░░░░░░░░░░░   ← Pico real en 881

 0.3│                  ███████

    │                ███████████

 0.2│              █████████████████

    │          ░░███████████████████░░

    └──────────────────────────────────> Frecuencia

                    Ventana de búsqueda



Resultado: Armónica encontrada en 881 Hz con amplitud 0.42V

PASO 5: Límite de Nyquist

Si una armónica esperada excede la frecuencia de Nyquist (fs/2 = 1920 Hz), el algoritmo detiene la búsqueda:

if (bin_objetivo >= amplitudes_size) {

    break;  // Armónica fuera del rango representable

}

Ejemplo: Con fs=3840 Hz, solo podemos detectar hasta 1920 Hz. Si f₀=500 Hz:

1ª armónica: 500 Hz ✓

2ª armónica: 1000 Hz ✓

3ª armónica: 1500 Hz ✓

4ª armónica: 2000 Hz ✗ (excede Nyquist)

5ª armónica: no se calcula

PASO 6: Cálculo de THD (Distorsión Armónica Total)

Una vez detectadas todas las armónicas, se calcula el THD como medida de qué tan "pura" es la señal:



Donde:

A₁ = amplitud de la fundamental

A₂, A₃, A₄, A₅ = amplitudes de las armónicas superiores

Ejemplo numérico completo:

Señal medida: onda cuadrada de 440 Hz

Armónicas detectadas:

  1ª: 440.1 Hz → 0.950 V  (fundamental)

  2ª: 880.3 Hz → 0.012 V  (muy débil, casi cero)

  3ª: 1320.2 Hz → 0.315 V (33% de la fundamental, típico en onda cuadrada)

  4ª: 1760.1 Hz → 0.008 V (muy débil)

  5ª: 2200.4 Hz → Fuera de rango (>1920 Hz)



Cálculo de THD:

  THD = √(0.012² + 0.315² + 0.008²) / 0.950 × 100%

      = √(0.000144 + 0.099225 + 0.000064) / 0.950 × 100%

      = 0.3153 / 0.950 × 100%

      = 33.2%

Visualización en la interfaz:

El sistema muestra una tabla estructurada con los resultados para una señal con frecuencia fundamental en 296Hz:

Interpretación del THD:

THD < 1%: Señal muy pura (casi senoidal)

THD ≈ 5-10%: Señal con ligera distorsión

THD ≈ 30-50%: Señal con distorsión significativa (típico en ondas cuadradas/triangulares)

THD > 50%: Señal muy distorsionada o ruidosa

3. FUNDAMENTOS TEÓRICOS - FILTROS DIGITALES

3.1 Introducción a Filtros Digitales

3.1.1 Definición y Propósito

Un filtro digital es un sistema que modifica selectivamente el contenido frecuencial de una señal digital, atenuando o eliminando componentes de frecuencia no deseadas.

Aplicaciones:

Eliminación de ruido

Separación de señales

Ecualización de audio

Anti-aliasing

Conformación espectral

5.1.2 Ventajas sobre Filtros Analógicos

Aspecto

Filtros Analógicos

Filtros Digitales

Precisión

Limitada por tolerancias (±5-10%)

Exacta (limitada por precisión numérica)

Deriva térmica

Significativa (~100 ppm/°C)

Nula

Repetibilidad

Variable entre unidades

Perfecta

Complejidad

Limitada (orden alto = costoso)

Arbitraria (solo aumenta cómputo)

Reconfigurabilidad

Requiere cambio físico

Cambio de coeficientes por software

Linealidad de fase

Difícil de lograr (filtros Bessel)

Posible (filtros FIR)

Costo

Aumenta con orden

Constante en hardware

3.2 Tipos de Filtros Digitales

3.2.1 Clasificación por Respuesta Impulsional

1. Filtros FIR (Finite Impulse Response):



Características:

Respuesta impulsional de duración finita

Siempre estables

Fase lineal posible

Requieren más coeficientes para misma selectividad

2. Filtros IIR (Infinite Impulse Response):



Características:

Respuesta impulsional de duración infinita (teórica)

Requieren análisis de estabilidad

Fase no-lineal

Mayor eficiencia computacional (menos coeficientes)

3.2.2 Clasificación por Respuesta en Frecuencia

1. Filtro Pasabajos (Low-Pass):

Deja pasar frecuencias por debajo de fc​

Atenúa frecuencias por encima de fc​

Aplicación: Anti-aliasing, suavizado

2. Filtro Pasaaltos (High-Pass):

Atenúa frecuencias por debajo de fc​

Deja pasar frecuencias por encima de fc​

Aplicación: Eliminación de DC, detección de bordes

3.3 Filtros Butterworth

3.3.1 Características Teóricas

Los filtros Butterworth se diseñan para tener respuesta en amplitud máximamente plana en la banda de paso.

Función de transferencia (analógica):



Donde:

ωc​: Frecuencia de corte (donde |H|=1)=-3 dB)

n: Orden del filtro

Propiedades:

Respuesta plana en banda de paso (sin ripple)

Transición monotónica

Atenuación fuera de banda: −20n−20n dB/década

Fase no-lineal (retardo de grupo variable)

3.3.2 Diseño Digital - Transformación Bilineal

Para convertir un filtro analógico a digital se usa la transformación bilineal:



Donde T=1/fs​ es el período de muestreo.

Pre-warping de frecuencia:



Compensa la distorsión no-lineal de la transformación bilineal.

3.3.3 Orden del Filtro y Selectividad

Comparación de órdenes:

Orden

Atenuación

Ancho transición

Retardo grupo

n=2

-40 dB/dec

Muy ancho

Bajo

n=4

-80 dB/dec

Ancho

Medio-bajo

n=8

-160 dB/dec

Medio

Medio

n=16

-320 dB/dec

Estrecho

Alto

Justificación de orden 8:

Atenuación suficiente para aplicaciones de audio (-160 dB/década)

Latencia aceptable para procesamiento en tiempo real (~0.6 ms)

Balance óptimo selectividad/complejidad computacional

3.3.4 Respuesta en Frecuencia

Magnitud en dB:



En la frecuencia de corte:



Atenuación en stopband: Para f=2fc​:



Con n=8:



3.4 Estabilidad de Filtros IIR

Un filtro IIR es estable si todos los polos de su función de transferencia están dentro del círculo unitario en el plano z:



Consecuencias de inestabilidad:

Oscilaciones divergentes

Overflow numérico

Salida no acotada para entrada acotada

Los filtros Butterworth diseñados correctamente mediante transformación bilineal son inherentemente estables.

4. IMPLEMENTACIÓN DE FILTROS DIGITALES

4.1 Biblioteca IIR1

El sistema utiliza la biblioteca IIR1 de Bernd Porr, una implementación eficiente de filtros IIR (Infinite Impulse Response) digitales. Esta biblioteca proporciona filtros clásicos como Butterworth, Chebyshev y Bessel.

¿Qué es un filtro IIR?

Los filtros IIR (Respuesta Impulsional Infinita) son filtros digitales que utilizan retroalimentación: la salida actual depende no solo de las entradas actuales y pasadas, sino también de las salidas pasadas. Esto los hace muy eficientes computacionalmente, requiriendo menos operaciones que filtros FIR equivalentes.

Filtros implementados en el proyecto:

Se han configurado dos filtros Butterworth de orden 8:

Filtro Pasa Bajos: Deja pasar frecuencias bajas y atenúa frecuencias altas.

Filtro Pasa Altos: Deja pasar frecuencias altas y atenúa frecuencias bajas.

Declaración en el código:

// Filtros globales de orden 8

Iir::Butterworth::LowPass<8> lowpass_filter;

Iir::Butterworth::HighPass<8> highpass_filter;

¿Qué significa "orden 8"?

El orden de un filtro determina cuán "abrupta" es su respuesta frecuencial. Un filtro de orden 8 significa que:

Tiene 8 polos en su función de transferencia

Proporciona una atenuación de 48 dB/octava en la banda de rechazo

Requiere almacenar 8 valores de entrada y 8 valores de salida anteriores

Es más selectivo que un filtro de orden 2, pero menos que uno de orden 16

4.2 Configuración de Filtros

Antes de usar un filtro, es necesario configurarlo con la frecuencia de muestreo y la frecuencia de corte deseada.

Frecuencia de corte:

La frecuencia de corte (fc) es el punto donde el filtro atenúa la señal en -3 dB (aproximadamente 70.7% de la amplitud original).

Pasa Bajos: Frecuencias menores a fc pasan, mayores a fc se atenúan

Pasa Altos: Frecuencias mayores a fc pasan, menores a fc se atenúan

Método de configuración:

// Configurar filtro pasa bajos

lowpass_filter.setup(frecuencia_muestreo, frecuencia_corte);



// Ejemplo: fs=3840 Hz, fc=500 Hz

lowpass_filter.setup(3840, 500);

// Resultado: Deja pasar 0-500 Hz, atenúa 500-1920 Hz

¿Qué hace setup() internamente?

La función setup() calcula automáticamente los coeficientes del filtro digital mediante la transformación bilineal:

Diseña un filtro analógico Butterworth con la frecuencia de corte especificada

Aplica la transformación bilineal para convertirlo a digital

Calcula los coeficientes de la ecuación en diferencias

Almacena estos coeficientes para usar en cada muestra

Reseteo del filtro:

Cuando se cambia la configuración o se inicia una nueva captura, es importante limpiar los estados internos:

lowpass_filter.reset();  // Limpia historial de entradas/salidas

Esto elimina los valores almacenados de muestras anteriores, evitando transitorios al procesar una nueva señal.

4.3 Aplicación en Tiempo Real

Pipeline de procesamiento:



Pipeline de procesamiento (ver MainWindow.cpp para detalles completos):

void SerialWorker() {  // Hilo de procesamiento en tiempo real

    while (do_serial_work) {

        int read = serial.read(read_buffer, 128);  // Leer bloque



        for (int i = 0; i < read; i++) {

            double voltaje = TransformSample(read_buffer[i]);  // ADC→V

            scrollY->push(voltaje);  // Almacenar original



            // Aplicar filtro

            double filtrado = (selected_filter == LowPass) ? 

                             lowpass_filter.filter(voltaje) : voltaje;



            filter_scrollY->push(filtrado);

            write_buffer[i] = InverseTransformSample(filtrado);  // V→DAC

        }



        serial.write(write_buffer, read);  // Enviar procesado

    }

}

Funciones de transformación (compensan acondicionamiento LM324):

// ADC [0-255] → Voltaje [-6V a +6V]

double TransformSample(uint8_t adc_value) {

    double v_adc = (adc_value / 255.0) * 5.0;         // Voltaje en ADC

    double v_in = ((v_adc - 0.8) / 0.25) - 6.0;      // Invertir LM324

    return clamp(v_in, -6.0, 6.0);

}



// Voltaje [-6V a +6V] → DAC [0-255]

uint8_t InverseTransformSample(double voltage) {

    double v_adc = (voltage + 6.0) * 0.25 + 0.8;     // Aplicar LM324

    return (uint8_t)((v_adc / 5.0) * 255.0);

}

Transformación LM324: V_salida = (V_entrada / 4) + 2.3V• -6V → 0.8V  |  0V → 2.3V  |  +6V → 3.8V

4.4 Visualización Dual

La interfaz gráfica muestra simultáneamente dos gráficos temporales superpuestos verticalmente, ambos con el mismo eje de tiempo, lo que permite comparar visualmente el efecto del filtro.

Gráfico superior: Señal de entrada (original del ADC) Gráfico inferior: Señal de salida (después del filtro)

Ambos gráficos comparten la misma escala temporal, facilitando la observación de:

El retardo introducido por el filtro (group delay)

La atenuación de frecuencias específicas

La forma de onda antes y después del procesamiento

4.5 Controles de Configuración

El usuario puede controlar el filtrado mediante la interfaz, seleccionando:

1. Tipo de filtro:

Ninguno: La señal pasa sin procesar (bypass)

Pasa Bajos: Atenúa frecuencias superiores a fc

Pasa Altos: Atenúa frecuencias inferiores a fc

2. Frecuencia de corte (fc): Ajustable dinámicamente en tiempo real, típicamente en el rango de 10 Hz hasta 1500 Hz.

Cuando se modifica cualquier parámetro, el filtro se reconfigura automáticamente llamando a setup() con los nuevos valores, y luego se ejecuta reset() para limpiar los estados internos y evitar transitorios al aplicar la nueva configuración.

4.6 Análisis de Latencia

Componentes de latencia total:

Conversión ADC: ~104 μs

Transmisión serial (1 byte): T_serial = 10 bits / 38400 bps = 260 μs

Procesamiento filtro: ~15 μs (orden 8)

Retardo de grupo filtro: ~0.4 ms

Transmisión vuelta: ~260 μs

Latencia total:

T_total = 104 + 260 + 15 + 400 + 260 ≈ 1040 μs ≈ 1.04 ms

En muestras:

Latencia = 1.04 ms × 3840 Hz ≈ 4 muestras

Conclusión: La latencia es aceptable para aplicaciones de audio en tiempo real (imperceptible por debajo de 10 ms).

5. JUSTIFICACIONES DE DISEÑO

5.1 Elección de Frecuencia de Muestreo

Decisión: fs=3840fs​=3840 Hz

Análisis basado en Teorema de Nyquist:



Justificación:

Rango útil: Cubre ampliamente señales de audio típicas en osciloscopios didácticos (20 Hz - 1 kHz)

Compatibilidad con baudrate:

Baudrate requerido = 10 bits/muestra × 3840 muestras/s = 38400 bps

Baudrate estándar disponible: 38400 bps 

Balance carga computacional:

FFT de 3840 muestras: ~0.15 ms

Filtro IIR orden 8: ~15 μs por muestra

CPU total: <5% en PC moderna

Compromiso resolución temporal/frecuencial:

Ventana de 1 segundo: 3840 muestras

Resolución frecuencial: 3840 Hz / 3840 = 1 Hz por bin

Alternativas descartadas:

f_s = 1920 Hz: Resolución frecuencial pobre

f_s = 7680 Hz: Requiere baudrate 76800 (no estándar en muchos sistemas)

f_s = 15360 Hz: Overhead excesivo para aplicación.

5.2 Arquitectura ADC→PC→DAC

Decisión: Procesamiento en PC en lugar de procesamiento embebido en Arduino

Ventajas:

Poder computacional

Visualización:

PC: Gráficos HD en tiempo real con ImPlot/OpenGL

Arduino: Limitado a comunicación serial → software externo

Flexibilidad:

Cambio de parámetros sin recompilar firmware

Experimentación con diferentes filtros instantáneamente

Logging y análisis post-procesamiento

Escalabilidad:

Fácil agregar nuevos análisis (THD, SNR, correlación)

Múltiples canales sin saturar Arduino

5.3 Uso de FFTW3

Decisión: Biblioteca FFTW3 en lugar de implementación propia

Ventajas:

Rendimiento: Hasta 100× más rápida que DFT naive

Speedup = O(N²) / O(N log N) = N / log N

Para N=1024: Speedup ≈ 100×

Optimizaciones:

Instrucciones SIMD (SSE4, AVX2)

Cache-friendly memory access

Planes pre-computados

Validación: Ampliamente probada en producción (MATLAB, SciPy la usan internamente)

5.4 Reducción 10→8 bits

Decisión: Transmitir 8 bits en lugar de 10 bits nativos del ADC

Análisis:

Opción 1: Transmitir 10 bits (2 bytes por muestra)

Baudrate requerido = (2 bytes × 8 bits) × 3840 Hz = 61440 bps

Baudrate estándar cercano: 115200 bps 

Opción 2: Transmitir 8 bits (1 byte por muestra)

Baudrate requerido = 8 bits × 3840 Hz = 30720 bps

Baudrate estándar cercano: 38400 bps ✓

Pérdida de resolución (sobre rango efectivo 0.8V-3.8V):

Resolución 10 bits: 3.0V / 1024 = 2.93 mV (en ADC)

                   → 11.72 mV referido a entrada ±6V



Resolución 8 bits: 3.0V / 256 = 11.72 mV (en ADC)

                  → 46.88 mV referido a entrada ±6V



Pérdida: 46.88 - 11.72 = 35.16 mV en términos de entrada

Justificación: La pérdida de resolución es aceptable considerando:

Ruido del acondicionador (~10 mV RMS en salida LM324, ~40 mV en entrada)

SNR del sistema (~50 dB)

Simplificación de protocolo serial

Reducción de 37% en ancho de banda

El rango efectivo (0.8V-3.8V) ya limita el uso del ADC al 60% de su capacidad

El circuito R2R que cumple la función de DAC tiene una resolución de 8 bits.

Conclusión:

Tras la implementación y validación del sistema, se concluye que es posible desarrollar un procesador digital de señales funcional utilizando componentes de bajo costo y hardware limitado como el Arduino Mega 2560. El proyecto demuestra que la combinación de un microcontrolador para la adquisición/generación y una PC para el cálculo intensivo es una arquitectura eficiente para aplicaciones educativas y de desarrollo.

Evaluación del Desempeño y Limitaciones

Acondicionamiento de Señal: Se logró adaptar señales de 6V al rango del ADC, aunque con una pérdida del 40% de la resolución total debido a las limitaciones de excursión del amplificador LM324 al ser alimentado con solo 5V. Para futuras iteraciones, el uso de una fuente simétrica externa permitiría aprovechar el rango completo de 0 a 5V del conversor.

Procesamiento Digital: La implementación de la FFT mediante la biblioteca FFTW3 y los filtros IIR de orden 8 permitió un análisis espectral preciso y una manipulación de la señal con una latencia mínima de 1.04 ms, lo cual es imperceptible para aplicaciones de audio en tiempo real.

Recreación Analógica: El circuito R2R de 8 bits demostró ser una alternativa sumamente asequible y efectiva para la función de DAC, permitiendo reconstruir las señales procesadas con una fidelidad aceptable para el propósito del trabajo.

Aprendizajes Clave

El desarrollo de este sistema permitió integrar conceptos críticos de ingeniería como el Teorema de Nyquist, la transformación bilineal para el diseño de filtros y la optimización de protocolos de comunicación serie para sistemas de tiempo real. A pesar de las restricciones económicas del hardware, la versatilidad de la plataforma permitió cumplir con todos los objetivos planteados, validando la capacidad del sistema para visualizar y procesar señales de forma similar a instrumentos de laboratorio más sofisticados

Bibliografía:

Placa de desarrollo Arduino Mega 2560 datasheet:

<>

<>



Microcontrolador ATmega-2560 datasheet:

<>



Funcionamiento circuito R2R:

<>



Imágenes hechas en Proteus 8 profesional:

<>



“Amplificadores y Comparadores” datasheet:

<>



“Amplificadores Operacionales” formulas:

<>



Sergio Andrés Castaño Giraldo, “Timer Arduino”:

<>



Vladimir Trujillo, “Conversión ADC con registros e interrupciones”:

<>



FFTW3 Documentation:

<>



IIR1 Library:

<https://github.com/berndporr/iir1>



Software Utilizado:

Arduino IDE: v1.8.19

C++ Compiler: GCC 11.2.0

FFTW3: v3.3.10

ImGui: v1.89

ImPlot: v0.14

Visual Studio 2022: Desarrollo de SerialPlotter

