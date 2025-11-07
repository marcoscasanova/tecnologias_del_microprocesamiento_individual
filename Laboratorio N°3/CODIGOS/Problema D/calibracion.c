#define F_CPU 16000000UL // Se define la frecuencia del microcontrolador en 16 MHz para las funciones de retardo
#include <avr/io.h> // Librería de control de puertos de entrada/salida del microcontrolador AVR
#include <util/delay.h> // Librería para generar retardos temporales precisos
#include <stdio.h> // Librería estándar para formateo de cadenas (sprintf)
#include <string.h> // Librería para manejo de cadenas de caracteres

#include "uart.h" // Librería personalizada para comunicación serial UART
#include "adc.h" // Librería personalizada para manejo del conversor analógico-digital (ADC)

#define BAUD 9600 // Se define la velocidad de comunicación UART en 9600 baudios
#define MYUBRR (F_CPU / 16 / BAUD - 1) // Se calcula el valor del registro UBRR para inicializar el UART

// Prototipo de función para leer una línea completa desde UART
void UART_LEER_LINEA(char *buf, uint8_t maxlen);

int main(void) { // Función principal del programa
	UART_INICIAR(MYUBRR); // Inicializa el módulo UART con el valor calculado del registro de baudios
	ADC_INICIAR(); // Inicializa el módulo ADC para realizar lecturas analógicas del joystick

	DDRD &= ~(1 << PD2); // Configura el pin PD2 como entrada (botón del joystick)
	PORTD |= (1 << PD2); // Activa la resistencia pull-up interna en PD2

	char etiqueta[32]; // Arreglo para almacenar el texto introducido por el usuario (dirección)
	char msg[64]; // Arreglo para formatear y enviar los mensajes por UART

	UART_IMPRIMIR("\r\n=== Calibracion Joystick (modo manual) ===\r\n"); // Mensaje de inicio del modo de calibración
	UART_IMPRIMIR("1) Escriba la DIRECCION y presione ENTER.\r\n"); // Instrucción para ingresar la dirección
	UART_IMPRIMIR("2) Cuando quiera medir, presione 'w'.\r\n"); // Instrucción para realizar la medición
	UART_IMPRIMIR("   Direcciones sugeridas: ARRIBA/ABAJO/IZQUIERDA/DERECHA/CENTRO\r\n"); // Sugerencias de etiquetas válidas

	while (1) { // Bucle principal del programa
		UART_IMPRIMIR("\r\nDireccion> "); // Solicita al usuario que ingrese una etiqueta de dirección
		UART_LEER_LINEA(etiqueta, sizeof(etiqueta)); // Lee la línea completa ingresada por UART

		if (etiqueta[0] == '\0') { // Si la entrada está vacía
			UART_IMPRIMIR("(vacio, reintente)\r\n"); // Se notifica al usuario que debe reintentar
			continue; // Se vuelve a solicitar una nueva entrada
		}

		UART_IMPRIMIR("Listo. Presione 'w' para medir...\r\n"); // Mensaje de confirmación antes de la medición

		for (;;) { // Bucle interno que espera la tecla 'w' para iniciar la medición
			char c = UART_RECIBIR(); // Se lee un carácter recibido por UART
			if (c == 'w' || c == 'W') { // Si el usuario presiona 'w' o 'W'
				uint16_t x = ADC_LEER_CANAL(0); // Se lee el valor analógico del eje X del joystick
				uint16_t y = ADC_LEER_CANAL(1); // Se lee el valor analógico del eje Y del joystick
				uint8_t sw = ((PIND & (1 << PD2)) ? 0 : 1); // Se lee el estado del botón (activo en bajo)

				sprintf(msg, "[%s] X=%4u | Y=%4u | SW=%u\r\n", etiqueta, x, y, sw); // Se formatea el mensaje con los valores medidos
				UART_IMPRIMIR(msg); // Se envía la información al monitor serial
				break; // Se sale del bucle interno para permitir una nueva dirección
			}
		}
	}
}

// Función para leer una línea completa desde UART hasta recibir ENTER o hasta alcanzar la longitud máxima
void UART_LEER_LINEA(char *buf, uint8_t maxlen) {
	uint8_t i = 0; // Índice para recorrer el buffer
	while (i < (maxlen - 1)) { // Se repite mientras no se alcance el límite del buffer
		char c = UART_RECIBIR(); // Se recibe un carácter desde el puerto UART

		if (c == '\r' || c == '\n') { // Si el carácter recibido es ENTER (carriage return o newline)
			_delay_ms(5); // Pequeño retardo para limpiar el buffer de recepción
			while (UCSR0A & (1 << RXC0)) { // Mientras haya caracteres residuales en el buffer UART
				char next = UDR0; // Se lee el siguiente carácter recibido
				if (next != '\r' && next != '\n') break; // Se descartan los saltos de línea repetidos
			}
			break; // Se termina la lectura de la línea
		}

		buf[i++] = c; // Se almacena el carácter en el buffer
	}
	buf[i] = '\0'; // Se agrega el terminador nulo al final de la cadena para completar la cadena C
}
