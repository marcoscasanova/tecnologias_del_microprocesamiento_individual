#ifndef F_CPU  // Verifica si no se ha definido la frecuencia del CPU
#define F_CPU 16000000UL  // Define la frecuencia del microcontrolador en 16 MHz
#endif  // Fin de la verificación de F_CPU

#include <string.h>  // Se incluye la librería estándar para manejo de cadenas y memoria
#include <avr/io.h>  // Se incluye la librería de acceso a registros del microcontrolador AVR
#include <util/delay.h>  // Se incluye la librería para generar retardos mediante funciones _delay_ms() y _delay_us()
#include <stdio.h>  // Se incluye para manejo de impresión y depuración de texto
#include "rc522.h"  // Se incluye el archivo de cabecera del módulo RFID RC522
#include "spi.h"  // Se incluye la librería del bus SPI usada para comunicación con el RC522
#include "uart.h"  // Se incluye la librería UART para salida de texto en modo depuración

void RFID_RESETEAR_INICIAR(void) {
    RST_DDR  |= (1<<RST_PIN);  // Configura el pin RST como salida
    RST_PORT &= ~(1<<RST_PIN);  // Mantiene el pin en bajo para iniciar el reset del módulo
    _delay_ms(10);  // Espera breve para asegurar el reset físico
    RST_PORT |= (1<<RST_PIN);  // Libera el reset, poniendo el pin en alto
    _delay_ms(50);  // Espera para que el RC522 se estabilice después del reinicio
}

void RFID_ESCRIBIR(uint8_t reg, uint8_t value) {
    SS_LOW();  // Activa la línea SS (Slave Select) para iniciar la comunicación SPI
    SPI_TRANSFERIR((reg<<1) & 0x7E);  // Envía la dirección del registro con bit de escritura
    SPI_TRANSFERIR(value);  // Envía el valor que se escribirá en el registro
    SS_HIGH();  // Desactiva la línea SS para finalizar la transmisión
}

uint8_t RFID_LEER(uint8_t reg) {
    uint8_t val;  // Variable para almacenar el valor leído
    SS_LOW();  // Activa el dispositivo esclavo
    SPI_TRANSFERIR(((reg<<1)&0x7E) | 0x80);  // Envía la dirección del registro con bit de lectura
    val = SPI_TRANSFERIR(0x00);  // Lee el valor retornado por el RC522
    SS_HIGH();  // Desactiva la línea SS
    return val;  // Retorna el valor leído
}

void RFID_SET_BITMASK(uint8_t reg, uint8_t mask) {
    uint8_t tmp = RFID_LEER(reg);  // Lee el valor actual del registro
    RFID_ESCRIBIR(reg, tmp | mask);  // Escribe nuevamente el valor con los bits del mask establecidos en 1
}

void RFID_LIMPIAR_BITMASK(uint8_t reg, uint8_t mask) {
    uint8_t tmp = RFID_LEER(reg);  // Lee el valor actual del registro
    RFID_ESCRIBIR(reg, tmp & (~mask));  // Limpia los bits indicados en el mask
}

void RFID_IMPRIMIR_REGISTRO(const char* name, uint8_t reg) {
    UART_IMPRIMIR(name);  // Imprime el nombre del registro
    UART_IMPRIMIR(": ");  // Imprime un separador
    UART_IMPRIMIR_HEX(RFID_LEER(reg));  // Imprime el valor del registro en formato hexadecimal
    UART_IMPRIMIR("\r\n");  // Salto de línea
}

void RFID_REINICIAR(void) {
    RC522_DBG("Soft Reset...\r\n");  // Mensaje de depuración
    RFID_ESCRIBIR(CommandReg, (1<<4));  // Escribe en CommandReg para realizar un reinicio por software
    _delay_ms(50);  // Espera a que el reinicio se complete
}

void RFID_INICIAR(void) {
    RFID_REINICIAR();  // Reinicia el módulo RC522
    RC522_DBG("Configurando temporizadores y modulacion...\r\n");  // Mensaje de depuración
    RFID_ESCRIBIR(TModeReg, 0x8D);  // Configura el temporizador interno
    RFID_ESCRIBIR(TPrescalerReg, 0x3E);  // Establece el prescaler del temporizador
    RFID_ESCRIBIR(TReloadRegL, 30);  // Valor bajo del temporizador
    RFID_ESCRIBIR(TReloadRegH, 0);  // Valor alto del temporizador
    RFID_ESCRIBIR(TxASKReg, 0x40);  // Habilita la modulación ASK (amplitud)
    RFID_ESCRIBIR(ModeReg, 0x3D);  // Configura el modo de operación
    RFID_ESCRIBIR(RFCfgReg, 0x7F);  // Ajusta la ganancia del receptor
    RFID_ESCRIBIR(TxControlReg, 0x83);  // Habilita la antena de transmisión
    _delay_ms(5);  // Espera a que se apliquen las configuraciones
}

void RFID_DEBUG_INICIAR(void) {
    RFID_REINICIAR();  // Reinicia el módulo
    RC522_DBG("Configurando temporizadores y modulacion...\r\n");  // Mensaje informativo
    RFID_ESCRIBIR(TModeReg, 0x8D);  // Configura el temporizador interno
    RFID_ESCRIBIR(TPrescalerReg, 0x3E);  // Establece el prescaler del temporizador
    RFID_ESCRIBIR(TReloadRegL, 30);  // Valor bajo del temporizador
    RFID_ESCRIBIR(TReloadRegH, 0);  // Valor alto del temporizador
    RFID_ESCRIBIR(TxASKReg, 0x40);  // Activa modulación ASK
    RFID_ESCRIBIR(ModeReg, 0x3D);  // Configura el modo de operación
    RFID_ESCRIBIR(RFCfgReg, 0x7F);  // Ajusta la ganancia del receptor
    RFID_ESCRIBIR(TxControlReg, 0x83);  // Activa la antena
    _delay_ms(5);  // Retardo breve
    RC522_DBG("Registros clave despues de init:\r\n");  // Mensaje de depuración
    RFID_IMPRIMIR_REGISTRO("VersionReg", VersionReg);  // Muestra el valor del registro VersionReg
    RFID_IMPRIMIR_REGISTRO("TxControlReg", TxControlReg);  // Muestra el valor del registro TxControlReg
    RFID_IMPRIMIR_REGISTRO("TxASKReg", TxASKReg);  // Muestra el valor del registro TxASKReg
    RFID_IMPRIMIR_REGISTRO("ModeReg", ModeReg);  // Muestra el valor del registro ModeReg
    RFID_IMPRIMIR_REGISTRO("TModeReg", TModeReg);  // Muestra el valor del registro TModeReg
    RFID_IMPRIMIR_REGISTRO("TPrescalerReg", TPrescalerReg);  // Muestra el valor del registro TPrescalerReg
    RFID_IMPRIMIR_REGISTRO("TReloadRegH", TReloadRegH);  // Muestra el valor del registro TReloadRegH
    RFID_IMPRIMIR_REGISTRO("TReloadRegL", TReloadRegL);  // Muestra el valor del registro TReloadRegL
    RFID_IMPRIMIR_REGISTRO("RFCfgReg", RFCfgReg);  // Muestra el valor del registro RFCfgReg
}

void RFID_STANDARD(uint8_t *card_uid) {
    uint8_t req[1] = {PICC_REQIDL};  // Comando para detectar una tarjeta en estado inactivo
    uint8_t buffer[16];  // Buffer de almacenamiento temporal
    uint8_t bufferLength = sizeof(buffer);  // Longitud del buffer

    RFID_ESCRIBIR(BitFramingReg, 0x07);  // Configura el envío de solo 7 bits del último byte
    RFID_ESCRIBIR(CommIrqReg, 0x7F);  // Limpia las interrupciones anteriores
    RFID_ESCRIBIR(FIFOLevelReg, 0x80);  // Limpia el buffer FIFO interno

    RC522_DBG("\r\n=== Enviando REQA ===\r\n");  // Mensaje de depuración
    for (uint8_t i=0; i<1; i++) RFID_ESCRIBIR(FIFODataReg, req[i]);  // Escribe el comando REQA en el FIFO

    RFID_ESCRIBIR(CommandReg, PCD_TRANSCEIVE);  // Envía el comando de transmisión y recepción
    RFID_SET_BITMASK(BitFramingReg, 0x80);  // Inicia la transmisión

    uint16_t count = 1000;  // Contador de espera
    uint8_t irq;  // Variable para almacenar el estado de interrupciones
    do {
        irq = RFID_LEER(CommIrqReg);  // Lee las interrupciones
        count--;  // Decrementa el contador
    } while (!(irq & 0x30) && count);  // Espera a que ocurra la interrupción o se agote el tiempo

    RFID_LIMPIAR_BITMASK(BitFramingReg, 0x80);  // Limpia el bit de transmisión

    if (count == 0) {
        RC522_DBG("Timeout REQA, tarjeta no detectada\r\n");  // Mensaje de fallo de detección
        memset(card_uid, 0, 16);  // Limpia el UID
    } else {
        uint8_t fifoLevel = RFID_LEER(FIFOLevelReg);  // Lee cuántos bytes hay disponibles en FIFO
        for (uint8_t i=0; i<fifoLevel; i++) {
            uint8_t val = RFID_LEER(FIFODataReg);  // Extrae los datos recibidos del FIFO
            if (i < bufferLength) buffer[i] = val;  // Los almacena en el buffer si hay espacio
        }
        if (fifoLevel > 0) {
            RC522_DBG("Tarjeta detectada! Intentando leer UID...\r\n");  // Mensaje de éxito parcial
            RFID_ESCRIBIR(BitFramingReg, 0x00);  // Configura el envío completo de bytes
            RFID_ESCRIBIR(CommIrqReg, 0x7F);  // Limpia las interrupciones
            RFID_ESCRIBIR(FIFOLevelReg, 0x80);  // Limpia el FIFO
            RFID_ESCRIBIR(FIFODataReg, PICC_ANTICOLL);  // Comando anticollision
            RFID_ESCRIBIR(FIFODataReg, 0x20);  // Parámetro del comando
            RFID_ESCRIBIR(CommandReg, PCD_TRANSCEIVE);  // Envía la orden de transmisión
            RFID_SET_BITMASK(BitFramingReg, 0x80);  // Inicia la transmisión
            count = 1000;  // Reinicia el contador
            do {
                irq = RFID_LEER(CommIrqReg);  // Verifica si la comunicación finalizó
                count--;
            } while (!(irq & 0x30) && count);  // Espera hasta que haya respuesta o timeout
            RFID_LIMPIAR_BITMASK(BitFramingReg, 0x80);  // Limpia el bit de transmisión
            if (count == 0) {
                RC522_DBG("Timeout Anticollision\r\n");  // Falla en la lectura del UID
            } else {
                fifoLevel = RFID_LEER(FIFOLevelReg);  // Lee el número de bytes en FIFO
                RC522_DBG("UID leido!\r\n");  // Mensaje indicando éxito
                for (uint8_t i=0; i<fifoLevel; i++) {
                    uint8_t val = RFID_LEER(FIFODataReg);  // Extrae cada byte del UID
                    card_uid[i] = val;  // Lo almacena en el arreglo recibido
                }
            }
        }
    }
}
