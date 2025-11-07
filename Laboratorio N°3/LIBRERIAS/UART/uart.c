#ifndef F_CPU  // Verifica si no está definida la frecuencia del microcontrolador
#define F_CPU 16000000UL  // Define la frecuencia del reloj en 16 MHz
#endif  // Fin de la verificación de F_CPU

#include <avr/io.h>  // Se incluye la librería para acceder a los registros de hardware del microcontrolador AVR
#include <util/delay.h>  // Se incluye para permitir retardos de tiempo mediante _delay_ms() o _delay_us()
#include <stdio.h>  // Se incluye para el uso de funciones de formato como sprintf()
#include "uart.h"  // Se incluye el archivo de cabecera del módulo UART

void UART_INICIAR(unsigned int ubrr) {  // Inicializa el módulo UART con el valor UBRR especificado
    UBRR0H = (unsigned char)(ubrr >> 8);  // Carga la parte alta del valor del divisor de baud rate
    UBRR0L = (unsigned char)ubrr;  // Carga la parte baja del valor del divisor de baud rate
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);  // Habilita la transmisión y recepción de datos
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  // Configura el formato: 8 bits de datos, 1 bit de stop, sin paridad
}

char UART_RECIBIR(void) {  // Espera la recepción de un carácter y lo devuelve
    while (!(UCSR0A & (1 << RXC0)));  // Espera hasta que haya un dato recibido disponible
    return UDR0;  // Retorna el byte recibido
}

void UART_ENVIAR(char c) {  // Envía un carácter a través del puerto UART
    while (!(UCSR0A & (1 << UDRE0)));  // Espera hasta que el buffer de transmisión esté vacío
    UDR0 = c;  // Carga el carácter en el registro de transmisión
}

void UART_IMPRIMIR(const char *s) {  // Envía una cadena de texto completa por UART
    while (*s) UART_ENVIAR(*s++);  // Envía carácter por carácter hasta el fin de la cadena
}

void UART_IMPRIMIR_HEX(uint8_t val) {  // Imprime un valor de 8 bits en formato hexadecimal
    char buf[6];  // Buffer temporal para almacenar el texto formateado
    sprintf(buf, "0x%02X", val);  // Convierte el valor numérico a texto hexadecimal
    UART_IMPRIMIR(buf);  // Envía la cadena generada por UART
}

void UART_IMPRIMIR_HEX_ARRAY(const uint8_t *arr, uint8_t len) {  // Imprime un arreglo de bytes en formato hexadecimal
    for (uint8_t i = 0; i < len; i++) {  // Recorre cada elemento del arreglo
        UART_IMPRIMIR_HEX(arr[i]);  // Imprime el valor en formato 0xXX
        UART_ENVIAR(' ');  // Añade un espacio entre valores
    }
    UART_IMPRIMIR("\r\n");  // Finaliza la línea con salto de carro y nueva línea
}

void UART_LEER_CADENA(char *buffer, uint8_t max_len) {  // Lee una cadena de texto recibida por UART hasta Enter
    uint8_t i = 0;  // Índice de posición en el buffer
    char c;  // Variable para almacenar el carácter recibido
    while (1) {  // Bucle continuo de lectura
        c = UART_RECIBIR();  // Espera y recibe un carácter
        if (c == '\r' || c == '\n') break;  // Finaliza si se presiona Enter
        if (i < max_len - 1) {  // Verifica que no se exceda el tamaño máximo del buffer
            buffer[i++] = c;  // Almacena el carácter en el buffer
            UART_ENVIAR(c);  // Hace eco del carácter enviado
        }
    }
    buffer[i] = '\0';  // Finaliza la cadena con el carácter nulo
    UART_IMPRIMIR("\r\n");  // Imprime salto de línea para limpiar la entrada
}

uint8_t UART_DISPONIBLE(void) {  // Devuelve si hay datos disponibles para leer
    return (UCSR0A & (1 << RXC0));  // Retorna 1 si hay un dato disponible, 0 si no
}

char UART_LEER(void) {  // Bloquea la ejecución hasta recibir un carácter y lo retorna
    while (!(UCSR0A & (1 << RXC0)));  // Espera hasta que el dato esté disponible
    return UDR0;  // Retorna el byte recibido
}
