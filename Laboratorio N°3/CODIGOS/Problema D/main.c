#define F_CPU 16000000UL // Se define la frecuencia del microcontrolador en 16 MHz
#define BAUD 9600 // Se define la velocidad de transmisión UART en 9600 baudios
#define MYUBRR (F_CPU / 16 / BAUD - 1) // Se calcula el valor del registro UBRR según la fórmula de comunicación serial

#include <avr/io.h> // Librería de entrada/salida para control de registros del microcontrolador
#include <util/delay.h> // Librería para generar retardos temporales
#include <stdlib.h> // Librería estándar para funciones como rand(), srand(), etc.
#include <stdio.h> // Librería para formateo de cadenas (sprintf)
#include "uart.h" // Librería personalizada para manejo de comunicación UART
#include "adc.h" // Librería personalizada para manejo del conversor analógico-digital
#include "ws2812.h" // Librería personalizada para control de la matriz de LEDs WS2812B

uint8_t leds[NUM_LEDS][3]; // Arreglo bidimensional que almacena los valores RGB de cada LED de la matriz

int main(void){ // Función principal del programa
	UART_INICIAR(MYUBRR); // Inicializa la comunicación UART con el valor calculado del registro UBRR
	ADC_INICIAR(); // Inicializa el módulo ADC para leer valores analógicos (joystick)
	WS2812_INICIAR(); // Inicializa la comunicación con la matriz de LEDs WS2812B

	DDRD &= ~(1 << PD2); // Configura el pin PD2 como entrada (botón del joystick)
	PORTD |= (1 << PD2); // Activa la resistencia pull-up interna en PD2

	srand(ADC_LEER_CANAL(0)); // Inicializa la semilla del generador de números aleatorios con una lectura del ADC

	uint8_t posX = 3, posY = 3; // Posición inicial del LED encendido en la matriz (coordenadas X,Y)
	uint8_t r, g, b; // Variables para almacenar los componentes de color RGB
	WS2812_COLOR_ALEATORIO(&r, &g, &b); // Genera un color aleatorio inicial para el LED

	WS2812_LIMPIAR(leds); // Limpia la matriz apagando todos los LEDs
	WS2812_SETEAR_LED(leds, WS2812_INDICE(posX, posY), r, g, b); // Enciende el LED en la posición inicial con el color aleatorio
	WS2812_MOSTRAR(leds); // Envía los datos a la matriz para reflejar los cambios visualmente

	UART_IMPRIMIR("\r\n=== CONTROL DE LED DE MATRIZ WS2813B CON JOYSTICK ===\r\n"); // Mensaje de inicio por UART

	while (1){ // Bucle principal del programa
		uint16_t x = ADC_LEER_CANAL(0); // Lectura del eje X del joystick (valor analógico)
		uint16_t y = ADC_LEER_CANAL(1); // Lectura del eje Y del joystick (valor analógico)
		uint8_t sw; // Variable para almacenar el estado del botón (switch) del joystick
		
		if (PIND & (1 << PD2)){ // Si el pin PD2 está en alto, el botón no está presionado
			sw = 0; // Botón suelto
			}else{
			sw = 1; // Botón presionado
		}
		
		const char *direccion = "CENTRO"; // Variable tipo cadena para indicar la dirección detectada

		// Se definen los rangos de valores que indican cada movimiento obtenidos a través del programa calibración, adjunto en el repositorio
		if (x < 400 && posY > 0){ // Si el joystick se mueve hacia arriba y no está en el borde superior
			posY--; // Mueve la posición hacia arriba
			direccion = "ARRIBA";
		}
		else if (x > 600 && posY < 7){ // Si el joystick se mueve hacia abajo y no está en el borde inferior
			posY++; // Mueve la posición hacia abajo
			direccion = "ABAJO";
		}

		if (y < 400 && posX < 7){ // Si el joystick se mueve hacia la derecha y no está en el borde derecho
			posX++; // Mueve la posición hacia la derecha
			direccion = "DERECHA";
		}
		else if (y > 600 && posX > 0){ // Si el joystick se mueve hacia la izquierda y no está en el borde izquierdo
			posX--; // Mueve la posición hacia la izquierda
			direccion = "IZQUIERDA";
		}

		if (sw){ // Si el botón del joystick fue presionado
			WS2812_COLOR_ALEATORIO(&r, &g, &b); // Se genera un nuevo color aleatorio
			UART_IMPRIMIR("APLICACIÓN DE COLOR ALEATORIO\r\n"); // Se notifica por UART el cambio de color
			while (!((PIND & (1 << PD2)))); // Espera hasta que el botón sea liberado
		}

		WS2812_LIMPIAR(leds); // Apaga todos los LEDs de la matriz
		WS2812_SETEAR_LED(leds, WS2812_INDICE(posX, posY), r, g, b); // Enciende nuevamente el LED en la nueva posición
		WS2812_MOSTRAR(leds); // Actualiza la matriz con el nuevo estado

		char buffer[100]; // Buffer temporal para formatear el texto a enviar por UART
		sprintf(buffer,
		"X=%4u | Y=%4u | Dir=%-9s | Color(R,G,B)=(%3u,%3u,%3u) | LED=(%u,%u)\r\n",
		x, y, direccion, r, g, b, posX, posY); // Se formatea la información de posición, dirección y color
		UART_IMPRIMIR(buffer); // Envía los datos por UART al monitor serial

		_delay_ms(150); // Pequeño retardo para evitar lectura demasiado rápida del joystick
	}
}
