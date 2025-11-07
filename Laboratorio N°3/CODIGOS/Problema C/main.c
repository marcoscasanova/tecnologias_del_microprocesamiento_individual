#define F_CPU 16000000UL // Se define la frecuencia del microcontrolador en 16 MHz para las funciones de retardo y temporización

#include <avr/io.h> // Librería principal para el manejo de registros de entrada/salida del microcontrolador
#include <util/delay.h> // Librería para generar retardos precisos en milisegundos
#include <stdlib.h> // Librería estándar con funciones generales (como abs(), rand(), etc.)
#include <stdio.h> // Librería para formateo de cadenas (sprintf)
#include "uart.h" // Librería personalizada para comunicación serial UART
#include "adc.h" // Librería personalizada para manejo del conversor analógico-digital (ADC)

#define BAUD 9600 // Se define la velocidad de transmisión UART en 9600 baudios
#define MYUBRR (F_CPU / 16 / BAUD - 1) // Se calcula el valor del registro UBRR para configurar la UART

#define IN1 PB0 // Pin PB0 asignado al control de dirección del motor (entrada 1 del puente H)
#define IN2 PB1 // Pin PB1 asignado al control de dirección del motor (entrada 2 del puente H)
#define PWM PD6 // Pin PD6 asignado como salida PWM (OC0A) para control de velocidad

// Prototipos de funciones
void PWM_INICIAR(void); // Inicializa el módulo PWM
void PWM_SETEAR(uint8_t value); // Ajusta el ciclo de trabajo del PWM
void MOTOR(int16_t error); // Controla la dirección y velocidad del motor según el error

// Constantes del control proporcional
#define KP 0.35f // Constante proporcional (ganancia P)
#define PWM_MIN 80 // Valor mínimo de PWM para superar la inercia del motor
#define DEAD_ERR 1 // Zona muerta: error mínimo para considerar el motor detenido

void PWM_INICIAR(void) {
	DDRD |= (1 << PWM); // Configura el pin PD6 como salida (canal OC0A)
	TCCR0A = (1 << COM0A1) | (1 << WGM00) | (1 << WGM01); // Configura el Timer0 en modo Fast PWM, salida no inversora
	TCCR0B = (1 << CS01) | (1 << CS00); // Selecciona prescaler 64 → frecuencia PWM ≈ 1 kHz (según F_CPU)
}

void PWM_SETEAR(uint8_t value) {
	OCR0A = value; // Carga el valor del ciclo de trabajo (duty cycle) en el registro OCR0A
}

void MOTOR(int16_t error) {
	int16_t abs_err = (error >= 0) ? error : -error; // Calcula el valor absoluto del error

	if (abs_err > DEAD_ERR) { // Si el error supera la zona muerta
		float pwm_f = PWM_MIN + (KP * abs_err); // Calcula el valor PWM proporcional al error
		if (pwm_f > 255.0f) pwm_f = 255.0f; // Limita el valor máximo del PWM a 255
		uint8_t pwm = (uint8_t)pwm_f; // Convierte el valor flotante a entero

		if (error > 0) { // Si el error es positivo → el valor real es menor que el de referencia
			PORTB |= (1 << IN1); // Activa IN1
			PORTB &= ~(1 << IN2); // Desactiva IN2 → Gira en sentido horario
			PWM_SETEAR(pwm); // Ajusta la velocidad según el valor PWM calculado
		} else { // Si el error es negativo → el valor real es mayor que el de referencia
			PORTB |= (1 << IN2); // Activa IN2
			PORTB &= ~(1 << IN1); // Desactiva IN1 → Gira en sentido antihorario
			PWM_SETEAR(pwm); // Ajusta la velocidad
		}
	} else { // Si el error está dentro de la zona muerta → se detiene el motor
		PORTB &= ~((1 << IN1) | (1 << IN2)); // Desactiva ambas entradas del puente H
		PWM_SETEAR(0); // Aplica 0% de ciclo útil → sin movimiento
	}
}

int main(void) {
	UART_INICIAR(MYUBRR); // Inicializa la comunicación serial UART
	ADC_INICIAR(); // Inicializa el módulo ADC para lectura de potenciómetros
	PWM_INICIAR(); // Inicializa el módulo PWM para control del motor
	DDRB |= (1 << IN1) | (1 << IN2); // Configura los pines de dirección del motor como salidas

	char buffer[64]; // Buffer para almacenar los mensajes a enviar por UART
	uint16_t ref, act; // Variables para la lectura de referencia (setpoint) y valor actual
	int16_t error; // Variable para almacenar la diferencia entre ref y act
	uint8_t pwm; // Variable para almacenar el valor actual del PWM
	char *sentido; // Puntero a texto descriptivo del sentido de giro
	uint16_t contador = 0; // Contador para limitar la frecuencia de impresión UART

	UART_IMPRIMIR("\r\n=== Control de potenciometro con motor PWM ===\r\n"); // Mensaje inicial de bienvenida

	while (1) { // Bucle principal de ejecución continua
		ref = ADC_LEER_CANAL(0); // Lectura del valor de referencia (potenciómetro de entrada)
		act = ADC_LEER_CANAL(1); // Lectura del valor actual (potenciómetro acoplado al motor)
		error = (int16_t)ref - (int16_t)act; // Cálculo del error como diferencia entre ambos valores

		MOTOR(error); // Controla el motor según el error calculado
		pwm = OCR0A; // Lee el valor actual del PWM aplicado al motor

		int16_t tolerancia = 25; // Define una tolerancia para determinar el estado de movimiento
		if (error > tolerancia)
			sentido = "Horario"; // Si el error es positivo → motor gira en sentido horario
		else if (error < -tolerancia)
			sentido = "Antihorario"; // Si el error es negativo → motor gira en sentido antihorario
		else
			sentido = "Detenido"; // Si el error está dentro del rango → el motor está quieto

		contador++; // Incrementa el contador de ciclos
		if (contador >= 10) { // Envía los datos por UART cada 10 iteraciones
			sprintf(buffer, "Ref:%u | Act:%u | PWM:%u | Sent:%s\r\n", ref, act, pwm, sentido); // Formatea los datos
			UART_IMPRIMIR(buffer); // Envía el mensaje al monitor serial
			contador = 0; // Reinicia el contador
		}

		_delay_ms(30); // Pequeño retardo para estabilidad del bucle y del ADC
	}
}
