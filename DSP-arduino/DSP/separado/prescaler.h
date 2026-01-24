#pragma once 

constexpr uint16_t elegir_prescaler(float frecuencia, float limite){
  constexpr uint16_t pres[] = { 1, 8, 64, 256, 1024 };
  int i = 0;
  while (i < 5 && 16e6 / (pres[i] * frecuencia) - 1 > limite)
    i++;
  return i < 5 ? pres[i] : 0;
}

constexpr uint8_t obtener_bits_prescaler(uint16_t prescaler){
  switch (prescaler){
    case 1:
      return 1;
    case 8:
      return 2;
    case 64:
      return 3;
    case 256:
      return 4;
    case 1024:
      return 5;
    default:
      return 0;
  }
}