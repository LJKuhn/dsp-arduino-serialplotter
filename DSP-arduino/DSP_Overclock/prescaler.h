#pragma once 

/**
 * Selecciona automáticamente el prescaler óptimo del Timer1
 * 
 * @param frecuencia Frecuencia deseada para la interrupción del timer (Hz)
 * @param limite     Valor máximo permitido para OCR1A (65535 para 16 bits)
 * @return           Valor de prescaler (1, 8, 64, 256, 1024) o 0 si es imposible
 * 
 * Fórmula: OCR1A = (F_CPU / (prescaler * frecuencia)) - 1
 * Donde F_CPU = 16,000,000 Hz (cristal Arduino)
 */
uint16_t elegir_prescaler(float frecuencia, float limite){
  // Prescalers disponibles del Timer1 en AVR
  const uint16_t pres[] = { 1, 8, 64, 256, 1024 };
  int i = 0;
  
  // Buscar el prescaler más pequeño que produzca un OCR1A válido
  while (i < 5 && 16e6 / (pres[i] * frecuencia) - 1 > limite)
    i++;
    
  return i < 5 ? pres[i] : 0;
}

/**
 * Convierte el valor del prescaler a bits para el registro TCCR1B
 * 
 * @param prescaler Valor del prescaler (1, 8, 64, 256, 1024)
 * @return          Valor de los bits CS12:CS10 para TCCR1B
 * 
 * Mapeo de prescalers a bits CS12:CS10:
 * - 1    → 001 (CS=1)
 * - 8    → 010 (CS=2)
 * - 64   → 011 (CS=3)
 * - 256  → 100 (CS=4)
 * - 1024 → 101 (CS=5)
 * - otros → 000 (Timer parado)
 */
uint8_t obtener_bits_prescaler(uint16_t prescaler){
  switch (prescaler){
    case 1:    return 1;  // CS = 001
    case 8:    return 2;  // CS = 010
    case 64:   return 3;  // CS = 011
    case 256:  return 4;  // CS = 100
    case 1024: return 5;  // CS = 101
    default:   return 0;  // CS = 000 (timer parado)
  }
}