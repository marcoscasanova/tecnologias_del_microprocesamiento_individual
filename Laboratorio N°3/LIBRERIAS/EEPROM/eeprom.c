#include "eeprom.h"  // Se incluye el archivo de cabecera correspondiente al manejo de la memoria EEPROM

void EEPROM_ESCRIBIR(uint16_t direccion, uint8_t dato) {
    while (EECR & (1 << EEPE));  // Se espera a que finalice cualquier escritura previa (EEPE = 0 indica que está libre)
    EEAR = direccion;  // Se carga en el registro EEAR la dirección de memoria EEPROM donde se escribirá el dato
    EEDR = dato;  // Se carga en el registro EEDR el dato que se desea almacenar en la EEPROM
    EECR |= (1 << EEMPE);  // Se habilita la escritura estableciendo el bit EEMPE (EEPROM Master Write Enable)
    EECR |= (1 << EEPE);  // Se inicia el ciclo de escritura estableciendo el bit EEPE (EEPROM Write Enable)
}

uint8_t EEPROM_LEER(uint16_t direccion) {
    while (EECR & (1 << EEPE));  // Se espera hasta que finalice cualquier operación de escritura anterior
    EEAR = direccion;  // Se carga en el registro EEAR la dirección de memoria EEPROM que se desea leer
    EECR |= (1 << EERE);  // Se inicia el ciclo de lectura estableciendo el bit EERE (EEPROM Read Enable)
    return EEDR;  // Se retorna el dato leído desde el registro EEDR
}
