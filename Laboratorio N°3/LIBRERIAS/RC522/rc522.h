#ifndef RC522_H  // Se define una directiva de inclusión condicional para evitar múltiples inclusiones del archivo
#define RC522_H  // Marca el inicio del bloque protegido de inclusión

#ifndef F_CPU  // Verifica si no se ha definido la frecuencia del microcontrolador
#define F_CPU 16000000UL  // Define la frecuencia del CPU en 16 MHz
#endif  // Fin de la comprobación de F_CPU

#include <avr/io.h>  // Se incluye la librería de acceso a los registros de E/S del microcontrolador AVR
#include <util/delay.h>  // Se incluye la librería para generar retardos con _delay_ms() y _delay_us()
#include <stdio.h>  // Se incluye para permitir funciones de impresión y depuración de texto

#define RC522_DEBUG 0  // Define si la depuración del módulo RC522 está habilitada (1) o deshabilitada (0)
#if RC522_DEBUG
  #define RC522_DBG(...) UART_IMPRIMIR(__VA_ARGS__)  // Si la depuración está activa, redirige la salida a UART_IMPRIMIR
#else
  #define RC522_DBG(...) do {} while (0)  // Si la depuración está desactivada, la macro no ejecuta ninguna acción
#endif

#define SS_LOW()   (PORTB &= ~(1<<PB2))  // Coloca la línea SS (Slave Select) en bajo para activar el dispositivo SPI
#define SS_HIGH()  (PORTB |=  (1<<PB2))  // Coloca la línea SS en alto para desactivar el dispositivo SPI

#define RST_PIN    PD4  // Define el pin físico utilizado para el reset del módulo RC522
#define RST_DDR    DDRD  // Define el registro de dirección de datos del puerto correspondiente al pin RST
#define RST_PORT   PORTD  // Define el registro de salida del puerto donde se encuentra el pin RST

#define CommandReg      0x01  // Registro de control de comandos
#define CommIEnReg      0x02  // Registro de habilitación de interrupciones
#define CommIrqReg      0x04  // Registro de interrupciones de comunicación
#define DivIrqReg       0x05  // Registro de interrupciones divisorias
#define ErrorReg        0x06  // Registro de errores del RC522
#define FIFODataReg     0x09  // Registro de datos FIFO (entrada/salida)
#define FIFOLevelReg    0x0A  // Registro de nivel de llenado del FIFO
#define ControlReg      0x0C  // Registro de control general
#define BitFramingReg   0x0D  // Registro de configuración de tramas de bits
#define ModeReg         0x11  // Registro de configuración del modo de operación
#define TxModeReg       0x12  // Registro de configuración del modo de transmisión
#define RxModeReg       0x13  // Registro de configuración del modo de recepción
#define TxControlReg    0x14  // Registro de control de la antena transmisora
#define TxASKReg        0x15  // Registro de modulación ASK
#define RFCfgReg        0x26  // Registro de configuración del receptor (ganancia)
#define TModeReg        0x2A  // Registro de modo del temporizador interno
#define TPrescalerReg   0x2B  // Registro de preescalador del temporizador
#define TReloadRegH     0x2C  // Registro alto del valor de recarga del temporizador
#define TReloadRegL     0x2D  // Registro bajo del valor de recarga del temporizador
#define VersionReg      0x37  // Registro que contiene la versión del chip RC522

#define PCD_IDLE        0x00  // Comando para poner el lector en estado inactivo
#define PCD_TRANSCEIVE  0x0C  // Comando para transmitir y recibir datos

#define PICC_REQIDL     0x26  // Comando REQA para detección de tarjetas en estado inactivo
#define PICC_ANTICOLL   0x93  // Comando ANTICOLL para evitar colisiones y obtener el UID de la tarjeta

void RFID_RESETEAR_INICIAR(void);  // Prototipo de función para realizar un reinicio físico del RC522
void RFID_ESCRIBIR(uint8_t reg, uint8_t value);  // Prototipo de función para escribir un valor en un registro del RC522
uint8_t RFID_LEER(uint8_t reg);  // Prototipo de función para leer el valor de un registro del RC522
void RFID_SET_BITMASK(uint8_t reg, uint8_t mask);  // Prototipo de función para establecer bits específicos en un registro
void RFID_LIMPIAR_BITMASK(uint8_t reg, uint8_t mask);  // Prototipo de función para limpiar bits específicos en un registro
void RFID_IMPRIMIR_REGISTRO(const char* name, uint8_t reg);  // Prototipo de función para imprimir el nombre y valor de un registro
void RFID_REINICIAR(void);  // Prototipo de función para realizar un reinicio por software del RC522
void RFID_INICIAR(void);  // Prototipo de función para inicializar el módulo RC522 con parámetros por defecto
void RFID_DEBUG_INICIAR(void);  // Prototipo de función para inicializar el módulo en modo depuración y mostrar registros
void RFID_STANDARD(uint8_t *card_uid);  // Prototipo de función estándar para detectar una tarjeta y leer su UID

#endif  // Fin de la protección contra inclusiones múltiples del archivo
