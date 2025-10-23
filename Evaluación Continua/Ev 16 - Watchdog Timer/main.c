#define F_CPU 16000000UL // Se define la frecuencia del CPU a 16 MHz
#include <avr/io.h> // Librería de entrada/salida para registros y puertos del microcontrolador
#include <avr/interrupt.h> // Librería para manejo de interrupciones
#include <avr/sleep.h> // Librería para control de modos de bajo consumo (sleep)
#include <util/delay.h> // Librería para generar retardos temporales
#include <stdint.h> // Librería de tipos de datos estandarizados (uint8_t, etc.)

volatile uint8_t bandera = 0; // Variable global tipo bandera utilizada para indicar interrupciones del temporizador

// Rutina de servicio de interrupción (ISR) del comparador A del Timer1
ISR(TIMER1_COMPA_vect) {
	bandera = 1; // Se coloca la bandera en 1 al cumplirse la interrupción del Timer1
}

// Función para configurar el Timer1 en modo CTC (Clear Timer on Compare Match)
void TIMER(void) {
	TCCR1A = 0; // Se limpia el registro de control A del Timer1
	TCCR1B = (1 << WGM12); // Se habilita el modo CTC (comparación con OCR1A)
	OCR1A = 15624; // Se carga el valor de comparación para obtener 1 segundo con prescaler 1024
	TIMSK1 = (1 << OCIE1A); // Se habilita la interrupción por coincidencia en OCR1A
	TCCR1B |= (1 << CS12) | (1 << CS10); // Se configura el prescaler del Timer1 en 1024 (16 MHz / 1024 = 15625 Hz)
}

// Función para esperar una cantidad de segundos definida utilizando las interrupciones del Timer1
void esperar_segundos(uint8_t segundos) {
	uint8_t count = 0; // Contador local de segundos
	while (count < segundos) { // Mientras no se alcance el número de segundos solicitado
		bandera = 0; // Se reinicia la bandera
		while (!bandera); // Se espera hasta que ocurra la interrupción (bandera = 1)
		count++; // Se incrementa el contador por cada segundo transcurrido
	}
}

// Función para colocar al microcontrolador en un modo de bajo consumo durante un tiempo determinado
void MODO_SLEEP(uint8_t modo) {
	set_sleep_mode(modo); // Se selecciona el modo de bajo consumo recibido por parámetro
	sleep_enable(); // Se habilita la función de modo sleep
	bandera = 0; // Se limpia la bandera de interrupción
	sei(); // Se habilitan las interrupciones globales
	while (!bandera) sleep_cpu(); // Se entra en modo sleep hasta que la bandera sea activada por el Timer1
	sleep_disable(); // Se deshabilita el modo sleep al salir
}

// Función principal del programa
int main(void) {
	DDRB  |= 0b00011111; // Se configuran los pines PB0–PB4 como salidas para los 5 LEDs
	PORTB &= ~0b00011111; // Se inicializan los LEDs apagados (nivel lógico bajo)

	TIMER(); // Se inicializa el Timer1
	sei(); // Se habilitan las interrupciones globales

	for (;;) { // Bucle principal infinito
		PORTB |= 0b00011111; // Se encienden los 5 LEDs
		_delay_ms(2000); // Se mantienen encendidos durante 2 segundos

		PORTB &= ~0b00011111; // Se apagan los 5 LEDs
		_delay_ms(50); // Breve retardo antes de entrar en modo de bajo consumo para asegurar el apagado correcto

		MODO_SLEEP(SLEEP_MODE_IDLE); // Se entra 10 segundos en modo IDLE
		esperar_segundos(10); // Se espera 10 segundos mientras el Timer1 mantiene la cuenta

		MODO_SLEEP(SLEEP_MODE_STANDBY); // Se entra 10 segundos en modo STANDBY
		esperar_segundos(10); // Se espera 10 segundos mientras el Timer1 despierta el micro

		MODO_SLEEP(SLEEP_MODE_PWR_DOWN); // Se entra 10 segundos en modo POWER-DOWN
		esperar_segundos(10); // Se espera 10 segundos para completar los 30 s totales de reposo
	}
}

//No se usó el Watchdog Timer porque no despierta correctamente al micro en bajo consumo con el bootloader de
//Arduino, así que se empleó el Timer1, que garantiza el funcionamiento real de los tres modos de sleep requeridos.
