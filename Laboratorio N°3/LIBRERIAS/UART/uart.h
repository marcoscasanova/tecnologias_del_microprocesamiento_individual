#ifndef UART_H  // Se define una directiva de inclusión condicional para evitar múltiples inclusiones del archivo
#define UART_H  // Marca el inicio del bloque protegido de inclusión

#ifndef F_CPU  // Verifica si no se ha definido la frecuencia del microcontrolador
#define F_CPU 16000000UL  // Define la frecuencia del reloj del sistema en 16 MHz
#endif  // Fin de la comprobación de F_CPU

#include <avr/io.h>  // Se incluye la librería para acceder a los registros del periférico UART del microcontrolador AVR
#include <util/delay.h>  // Se incluye para permitir retardos de tiempo mediante las funciones _delay_ms() o _delay_us()
#include <stdio.h>  // Se incluye para habilitar funciones de formato y manejo de cadenas como sprintf()

void UART_INICIAR(unsigned int ubrr);  // Prototipo de función para inicializar la UART con un divisor de baud rate específico
char UART_RECIBIR(void);  // Prototipo de función para recibir un carácter desde el puerto UART
void UART_ENVIAR(char c);  // Prototipo de función para enviar un carácter a través del puerto UART
void UART_IMPRIMIR(const char *s);  // Prototipo de función para enviar una cadena de texto completa por UART
void UART_IMPRIMIR_HEX(uint8_t val);  // Prototipo de función para imprimir un valor en formato hexadecimal
void UART_IMPRIMIR_HEX_ARRAY(const uint8_t *arr, uint8_t len);  // Prototipo de función para imprimir un arreglo de bytes en formato hexadecimal
void UART_LEER_CADENA(char *buffer, uint8_t max_len);  // Prototipo de función para leer una cadena de texto ingresada desde UART
uint8_t UART_DISPONIBLE(void);  // Prototipo de función que indica si hay datos disponibles para lectura
char UART_LEER(void);  // Prototipo de función que espera y devuelve un carácter recibido por UART

#endif  // Fin de la protección contra inclusiones múltiples del archivo
