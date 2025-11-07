#ifndef EEPROM_H  // Se define una directiva de inclusión condicional para evitar múltiples inclusiones del mismo archivo
#define EEPROM_H  // Se marca el inicio del bloque protegido de inclusión

#include <avr/io.h>  // Se incluye la librería principal de E/S del microcontrolador AVR para acceder a los registros de hardware
#include <stdint.h>  // Se incluye la librería estándar que define tipos de datos enteros con tamaño fijo (uint8_t, uint16_t, etc.)

void EEPROM_ESCRIBIR(uint16_t direccion, uint8_t dato);  // Prototipo de función para escribir un byte en una dirección específica de la EEPROM
uint8_t EEPROM_LEER(uint16_t direccion);  // Prototipo de función para leer un byte almacenado en una dirección específica de la EEPROM

#endif  // Fin de la protección contra inclusiones múltiples del archivo de cabecera
