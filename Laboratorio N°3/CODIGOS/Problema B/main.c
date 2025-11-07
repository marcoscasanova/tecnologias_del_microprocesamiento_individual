#define F_CPU 16000000UL // Se define la frecuencia del CPU a 16 MHz
#include <avr/io.h> // Se incluye la librería de entrada/salida del microcontrolador AVR
#include <util/delay.h> // Se incluye la librería para generar retardos
#include <stdlib.h> // Se incluye la librería estándar para funciones como atoi()
#include <stdio.h> // Se incluye para manejo de cadenas formateadas con sprintf()
#include <string.h> // Se incluye para manipulación de cadenas de caracteres
#include "uart.h" // Se incluye la librería personalizada para la comunicación UART
#include "adc.h" // Se incluye la librería personalizada para la lectura analógica del ADC
#include "pwm.h" // Se incluye la librería personalizada para el control PWM

#define BAUD 9600 // Se define la velocidad de comunicación serial en baudios
#define MYUBRR (F_CPU / 16 / BAUD - 1) // Se calcula el valor del registro UBRR para la UART

#define CALEFACTOR PB0 // Se define el pin PB0 como salida para controlar el calefactor
#define ENABLE PB1 // Se define el pin PB1 como salida de habilitación del puente H
#define IN1 PD2 // Se define el pin PD2 como entrada IN1 del puente H
#define IN2 PD3 // Se define el pin PD3 como entrada IN2 del puente H

static uint8_t punto_medio = 26; // Se define el valor inicial del punto medio de temperatura en 26 °C
static uint8_t pausa = 0; // Variable bandera que indica si el sistema está en modo pausa para ajuste

int main(void) { // Función principal del programa
	char buffer[64]; // Buffer para almacenar mensajes formateados
	char entrada[8]; // Buffer para capturar la entrada del usuario por UART
	uint8_t idx = 0; // Índice para recorrer el arreglo de entrada

	UART_INICIAR(MYUBRR); // Se inicializa la comunicación UART con el baudrate definido
	ADC_INICIAR(); // Se inicializa el módulo ADC para la lectura de temperatura
	PWM_INICIAR(64); // Se inicializa el módulo PWM con un prescaler de 64

	DDRB |= (1 << CALEFACTOR) | (1 << ENABLE); // Se configuran los pines PB0 y PB1 como salidas
	DDRD |= (1 << IN1) | (1 << IN2); // Se configuran los pines PD2 y PD3 como salidas

	PORTB &= ~(1 << CALEFACTOR); // Se asegura que el calefactor esté apagado al inicio
	PORTD &= ~((1 << IN1) | (1 << IN2)); // Se apagan ambas entradas del puente H
	PORTB |=  (1 << ENABLE); // Se habilita el puente H

	UART_IMPRIMIR("=== Control de Temperatura con PWM y Puente H ===\r\n"); // Se muestra el título del sistema
	UART_IMPRIMIR("Presione 'x' para cambiar el punto medio\r\n"); // Se indica al usuario cómo modificar el punto medio
	UART_IMPRIMIR("-------------------------------------------------\r\n"); // Separador visual en la consola

	while (1){ // Bucle principal del programa
		if (UART_DISPONIBLE()){ // Si hay datos disponibles por UART
			char t = UART_LEER(); // Se lee el carácter recibido
			if (t == 'x' || t == 'X'){ // Si el usuario presiona 'x' o 'X'
				pausa = 1; // Se activa el modo pausa
				UART_IMPRIMIR("\r\n>> Ajuste de punto medio activado\r\n"); // Se notifica al usuario
				UART_IMPRIMIR("Ingrese nuevo valor (10–50): "); // Se solicita un nuevo valor
				idx = 0; // Se reinicia el índice de entrada
				}else if (pausa){ // Si el sistema está en modo pausa
				if (t == '\r' || t == '\n'){ // Si el usuario presiona Enter
					entrada[idx] = '\0'; // Se finaliza la cadena de entrada
					int nuevo_pm = atoi(entrada); // Se convierte la cadena a número entero

					if (nuevo_pm >= 10 && nuevo_pm <= 50){ // Si el valor ingresado está dentro del rango permitido
						punto_medio = nuevo_pm; // Se actualiza el punto medio
						UART_IMPRIMIR("\r\nPunto medio actualizado correctamente\r\n"); // Se confirma la actualización
						}else{ // Si el valor está fuera del rango
						UART_IMPRIMIR("\r\nValor fuera de rango (10–50)\r\n"); // Se notifica error
					}
					pausa = 0; // Se desactiva el modo pausa
					idx = 0; // Se reinicia el índice
					UART_IMPRIMIR("Reanudando medición...\r\n"); // Se informa que se retoma la lectura
					}else if(idx < sizeof(entrada) - 1){ // Si aún hay espacio en el buffer de entrada
					entrada[idx++] = t; // Se almacena el carácter recibido
					UART_ENVIAR(t); // Se hace eco del carácter en la terminal
				}
				continue; // Se salta al siguiente ciclo del bucle principal
			}
		}

		if(!pausa){ // Si el sistema no está en modo pausa
			uint16_t adc_val = ADC_LEER_CANAL(0); // Se lee el valor analógico del canal 0 (sensor de temperatura)
			uint16_t tempC = (adc_val * 500UL) / 1023UL; // Se convierte el valor ADC a grados Celsius (escala 0–500 mV)

			int16_t lim_calef      = punto_medio - 4; // Límite inferior para encender el calefactor
			int16_t lim_neutro_min = punto_medio - 3; // Límite inferior de la zona neutra
			int16_t lim_neutro_max = punto_medio + 4; // Límite superior de la zona neutra
			int16_t lim_vlow_min   = punto_medio + 5; // Límite inferior para ventilador en velocidad baja
			int16_t lim_vlow_max   = punto_medio + 14; // Límite superior para ventilador en velocidad baja
			int16_t lim_vmed_min   = punto_medio + 15; // Límite inferior para ventilador en velocidad media
			int16_t lim_vmed_max   = punto_medio + 24; // Límite superior para ventilador en velocidad media
			int16_t lim_valt_min   = punto_medio + 25; // Límite inferior para ventilador en velocidad alta

			const char *accion = "Ninguna"; // Se inicializa la variable de texto para mostrar la acción actual

			if(tempC <= lim_calef){ // Si la temperatura está por debajo del rango de calefacción
				PORTB |=  (1 << CALEFACTOR); // Se enciende el calefactor
				PORTD &= ~((1 << IN1) | (1 << IN2)); // Se detiene el ventilador
				PWM_ESTABLECER_DUTY(0); // Se establece el duty cycle del PWM en 0
				accion = "Calefactor ON"; // Se actualiza la acción actual
				}else if(tempC >= lim_neutro_min && tempC <= lim_neutro_max){ // Si la temperatura está dentro del rango neutro
				PORTB &= ~(1 << CALEFACTOR); // Se apaga el calefactor
				PORTD &= ~((1 << IN1) | (1 << IN2)); // Se apaga el ventilador
				PWM_ESTABLECER_DUTY(0); // Duty cycle 0%
				accion = "Todo OFF"; // Se indica que todo está apagado
				}else if(tempC >= lim_vlow_min && tempC <= lim_vlow_max){ // Si la temperatura está entre los límites de velocidad baja
				PORTB &= ~(1 << CALEFACTOR); // Se apaga el calefactor
				PORTD |=  (1 << IN1); // Se activa IN1 para girar el ventilador en un sentido
				PORTD &= ~(1 << IN2); // Se mantiene IN2 apagado
				PWM_ESTABLECER_DUTY(85); // Se ajusta el PWM a velocidad baja (~33%)
				accion = "Ventilador BAJO"; // Se actualiza la acción
				}else if(tempC >= lim_vmed_min && tempC <= lim_vmed_max){ // Si la temperatura está entre los límites de velocidad media
				PORTB &= ~(1 << CALEFACTOR); // Se apaga el calefactor
				PORTD |=  (1 << IN1); // Se activa el pin IN1
				PORTD &= ~(1 << IN2); // Se apaga IN2
				PWM_ESTABLECER_DUTY(170); // Se ajusta el PWM a velocidad media (~66%)
				accion = "Ventilador MEDIO"; // Se actualiza la acción
				}else if(tempC >= lim_valt_min){ // Si la temperatura está por encima del límite superior
				PORTB &= ~(1 << CALEFACTOR); // Se apaga el calefactor
				PORTD |=  (1 << IN1); // Se activa el ventilador en el mismo sentido
				PORTD &= ~(1 << IN2); // Se mantiene el otro pin apagado
				PWM_ESTABLECER_DUTY(255); // Se establece el PWM en 100% de velocidad
				accion = "Ventilador ALTO"; // Se actualiza la acción
				}else{ // En cualquier otro caso (seguridad)
				PORTB &= ~(1 << CALEFACTOR); // Se apaga el calefactor
				PORTD &= ~((1 << IN1) | (1 << IN2)); // Se detiene el ventilador
				PWM_ESTABLECER_DUTY(0); // Duty cycle 0%
				accion = "Todo OFF"; // Se indica que todo está apagado
			}
			sprintf(buffer, "Temp:%uC | PM:%u | %s\r\n", tempC, punto_medio, accion); // Se formatea el mensaje con temperatura, punto medio y acción
			UART_IMPRIMIR(buffer); // Se envía la información al puerto serial
			_delay_ms(1000); // Se espera 1 segundo antes de la siguiente lectura
		}
	}
}
