/* 
este programa demuestra el uso del las interrupciones generadas por un timer para manejar el 
conversor analógico digital manejando los registros del microcontrolador
*/

#define pulsadorPin 2 // Pin del pulsador de habilitar y desabilitar muestreo
#define LED 13        // Led de control
volatile boolean habilitarADC = true; // Variable para habilitar o deshabilitar el ADC
int result_adc;  //resultado del conversor analógico digital.

void imprime_ADC(void);   //funcion encargada de enviar por el puerto seriel los datos obtenidos
void config_ADC(void);    //función de configuracion del conversor ADC
void config_timer1(void); //funcion de configuración del timer, el cual controla los disparos del ADC

void SPlotter() { // Cambia el estado de habilitarADC cuando se produce una interrupción
  habilitarADC =! habilitarADC;
}

void setup(){
  pinMode(pulsadorPin, INPUT); //defino modo de trabajo de los pines digitales
  pinMode(LED, OUTPUT); 
  attachInterrupt(digitalPinToInterrupt(pulsadorPin), SPlotter, RISING); //defino interrupcion externa para frenar o iniciar el muestreo
  config_ADC();
  config_timer1();
  Serial.begin(115200);
}
  
void loop(){
  //La función que detecta si se ha presionado una tecla se ejecuta continuamente
  imprime_ADC();
}

void config_ADC(void){
  DIDR0 = 0x3F;            // deshabilito las entradas digitales A0-A5 (solo 6 canales en Arduino Uno)
  ADMUX = 0x40;            // 0100 0001 voltaje de ref = 5v, justificación derecha, medida por el canal 0
  ADCSRA = 0xAC;           // 1010 1100 Habilito el conversor ADC, Habilita la interrupcion, INicia el preecalador en 16
  ADCSRB = 0x45;           // 0100 0101 AD channels MUX on, free running mode
  sei();                   // set interrupt flag
  }

void config_timer1(void){
  TCCR1A = TCCR1B = 0;//ELIMINAR CUALQUIE CONF PREVIA DEL TIMER
  TCNT1 = 0;//Borro cualquier conteo o valor previo

  OCR1B = 1332;//  SE COMPARA el timer 1 CONTRA 1332 CONTEOS...con el fin de obtener 1500 Hz, OCRB=(16MHz/prescaler * frecuencia maxima querida) - 1
  TCCR1B |= (1 << WGM12);  //se configuro el timer 1 en modo de comparación CTC
  TIMSK1 |= (1 << OCIE1B); //habilito la interrupción por comparación del timer 1 en el modo B.
  TCCR1B |= (1<<CS11);  //preescalador de 8
  }
 
  /*** Interrupt routine ADC ***/
ISR(TIMER1_COMPB_vect) //Interrupt Service Routine, es la interrupcion generada por el timer
{
  TCNT1=0;
}
  
ISR(ADC_vect) {       //interrupcion del ADC disparada por el timer
  result_adc = ADCL;        // guarda los bytes bajo del ADC
  result_adc += ADCH << 8;  // guarda los bytes altos ADC
}


void imprime_ADC(void){//solo imprime cuando esta habilitado el conversor
  if(habilitarADC == true){
  digitalWrite(LED, HIGH); //cambia de estado la salida del led
    Serial.println(result_adc);
  }
  else{
    digitalWrite(LED, LOW);
    }
}
