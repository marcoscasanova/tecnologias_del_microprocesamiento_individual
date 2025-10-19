#define F_CPU 16000000UL // Se define la frecuencia del CPU a 16 MHz
#define BAUD 9600 // Se define la velocidad de comunicacion en baudios
#define BPS ((F_CPU / (16UL * BAUD)) - 1) // Se calculan los baudios por segundo

#include <avr/io.h> // Libreria de entrada y salida de AVR
#include <util/delay.h> // Libreria de retardos
#include <stdint.h> // Libreria de tipos de datos estandarizados
#include <avr/pgmspace.h> // Libreria para manejo de memoria en programa

// Funcion para inicializar la comunicacion UART
void UART_INICIAR(void){
	UBRR0H = (uint8_t)(BPS >> 8); // Se configura el registro UBRR0H
	UBRR0L = (uint8_t)(BPS);   // Se configura el registro UBRR0L
	UCSR0B = (1 << RXEN0) | (1 << TXEN0); // Se habilita receptor y transmisor
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Se configuran 8 bits de datos, sin paridad y 1 bit de stop
}

// Funcion para transmitir un caracter por UART
void UART_TX(char data){
	while (!(UCSR0A & (1 << UDRE0))); // Se espera a que el buffer de transmision este listo
	UDR0 = data; // Se carga el dato en el registro UDR0 para su transmision
}

// Funcion para recibir un caracter por UART
char UART_RX(void){
	while (!(UCSR0A & (1 << RXC0))); // Se espera a que haya datos para recibir
	return UDR0; // Se retorna el dato recibido desde el registro UDR0
}

// Funcion para transmitir una cadena por UART
void UART_TX_CADENA(const char *str){
	while (*str) { // Mientras no se llegue al final de la cadena
		UART_TX(*str++); // Se transmite el caracter actual y se avanza al siguiente
	}
}

// Funciones para controlar el plotter
void PLOTTER_SUBIR(void){ // Se levanta la solenoide
	PORTD = 0b00001000; // Se pone en alto el pin PD3
	_delay_ms(250); // Se espera 250 ms para asegurar que la solenoide se levante
}
void PLOTTER_BAJAR(void){ // Se baja la solenoide
	PORTD = 0b00000100; // Se pone en alto el pin PD2
	_delay_ms(250); // Se espera 250 ms para asegurar que la solenoide se baje
}
void PLOTTER_ARRIBA_NO_BAJAR(void){ // Se mueve el plotter hacia arriba sin bajar la solenoide
	PORTD = 0b00100000; // Se pone en alto el pin PD5
}
void PLOTTER_ABAJO_NO_BAJAR(void){ // Se mueve el plotter hacia abajo sin bajar la solenoide
	PORTD = 0b00010000; // Se pone en alto el pin PD4
}
void PLOTTER_DERECHA_NO_BAJAR(void){ // Se mueve el plotter hacia la derecha sin bajar la solenoide
	PORTD = 0b01000000; // Se pone en alto el pin PD6
}
void PLOTTER_IZQUIERDA_NO_BAJAR(void){ // Se mueve el plotter hacia la izquierda sin bajar la solenoide
	PORTD = 0b10000000; // Se pone en alto el pin PD7
}
void PLOTTER_ARRIBA(void){ // Se mueve el plotter hacia arriba
	PORTD = 0b00100100; // Se ponen en alto los pines PD2 y PD5
}
void PLOTTER_ABAJO(void){ // Se mueve el plotter hacia abajo
	PORTD = 0b00010100; // Se ponen en alto los pines PD2 y PD4
}
void PLOTTER_DERECHA(void){ // Se mueve el plotter hacia la derecha
	PORTD = 0b01000100; // Se ponen en alto los pines PD4 y PD6
}
void PLOTTER_IZQUIERDA(void){ // Se mueve el plotter hacia la izquierda
	PORTD = 0b10000100; // Se ponen en alto los pines PD4 y PD7
}
void PLOTTER_ARRIBA_DERECHA(void){ // Se mueve el plotter hacia arriba y a la derecha para formar una diagonal
	PORTD = 0b01100100; // Se ponen en alto los pines PD2, PD5 y PD6
}
void PLOTTER_ABAJO_DERECHA(void){ // Se mueve el plotter hacia abajo y a la derecha para formar una diagonal
	PORTD = 0b01010100; // Se ponen en alto los pines PD4 y PD6
}
void PLOTTER_ABAJO_IZQUIERDA(void){ // Se mueve el plotter hacia abajo y a la izquierda para formar una diagonal
	PORTD = 0b10010100; // Se ponen en alto los pines PD4 y PD7
}
void PLOTTER_ARRIBA_IZQUIERDA(void){ // Se mueve el plotter hacia arriba y a la izquierda para formar una diagonal
	PORTD = 0b10100100; // Se ponen en alto los pines PD2, PD5 y PD7
}

// Estructura para definir un paso en una figura
typedef struct {
	char dir; // Direccion del movimiento
	uint16_t t; // Tiempo en milisegundos para el movimiento
} Paso;

// Funcion para ejecutar una figura dada una secuencia de pasos
void EJECUTAR_FIGURA(const Paso *figura, uint16_t n_pasos){
	// Se recorren todos los pasos de la figura
	for (uint16_t i = 0; i < n_pasos; i++){
		char dir = pgm_read_byte(&figura[i].dir); // Se lee la direccion del paso desde la memoria de programa
		uint16_t tiempo = pgm_read_word(&figura[i].t); // Se lee el tiempo del paso desde la memoria de programa

		// Se ejecuta el movimiento correspondiente segun la direccion
		switch(dir){
			case 'D': PLOTTER_DERECHA(); break; // Se mueve el plotter hacia la derecha
			case 'I': PLOTTER_IZQUIERDA(); break; // Se mueve el plotter hacia la izquierda
			case 'A': PLOTTER_ABAJO(); break; // Se mueve el plotter hacia abajo
			case 'U': PLOTTER_ARRIBA(); break; // Se mueve el plotter hacia arriba
			case 'B': PLOTTER_BAJAR(); break; // Se baja la solenoide
			case 'S': PLOTTER_SUBIR(); break; // Se levanta la solenoide
			case 'd': PLOTTER_DERECHA_NO_BAJAR(); break; // Se mueve el plotter hacia la derecha sin bajar la solenoide
			case 'i': PLOTTER_IZQUIERDA_NO_BAJAR(); break; // Se mueve el plotter hacia la izquierda sin bajar la solenoide
			case 'a': PLOTTER_ABAJO_NO_BAJAR(); break; // Se mueve el plotter hacia abajo sin bajar la solenoide
			case 'u': PLOTTER_ARRIBA_NO_BAJAR(); break; // Se mueve el plotter hacia arriba sin bajar la solenoide
			default: break; // Si la direccion no es valida, no se hace nada
		}
		
		// Se espera el tiempo especificado para el movimiento
		for (uint16_t j = 0; j < tiempo; j++){
			_delay_ms(1); // Retardo de 1 ms
		}
	}
}

// Funcion para dibujar un triangulo
void TRIANGULO(void){
	PLOTTER_BAJAR(); _delay_ms(250); // Se baja la solenoide durante 250 ms
	PLOTTER_IZQUIERDA(); _delay_ms(6000); // Se mueve a la izquierda durante 6000 ms
	PLOTTER_ABAJO_DERECHA(); _delay_ms(3000); // Se mueve hacia abajo y a la derecha durante 3000 ms
	PLOTTER_ARRIBA_DERECHA(); _delay_ms(3000); // Se mueve hacia arriba y a la derecha durante 3000 ms
	PLOTTER_SUBIR(); _delay_ms(250); // Se levanta la solenoide durante 250 ms
}

// Definicion de la figura circulo a través de una estructura de pasos
const Paso CIRCULO[] PROGMEM = {
	{'B', 250}, {'D', 600}, {'A', 100}, {'D', 400}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 300}, {'A', 100}, {'D', 100},
	{'A', 100}, {'D', 200}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100},
	{'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 200}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 200}, {'D', 100},
	{'A', 100}, {'D', 100}, {'A', 300}, {'D', 100}, {'A', 200}, {'D', 100}, {'A', 400}, {'D', 100}, {'A', 500}, {'A', 600},
	{'I', 100}, {'A', 400}, {'I', 100}, {'A', 200}, {'I', 100}, {'A', 300}, {'I', 100}, {'A', 100}, {'I', 100}, {'A', 200},
	{'I', 100}, {'A', 100}, {'I', 100}, {'A', 200}, {'I', 100}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 100}, {'A', 100},
	{'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 100}, {'A', 100},
	{'I', 300}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 400}, {'A', 100}, {'I', 500}, {'I', 600}, {'U', 100}, {'I', 400},
	{'U', 100}, {'I', 200}, {'U', 100}, {'I', 300}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 100},
	{'U', 100}, {'I', 200}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 100},
	{'U', 200}, {'I', 100}, {'U', 100}, {'I', 100}, {'U', 200}, {'I', 100}, {'U', 100}, {'I', 100}, {'U', 300}, {'I', 100},
	{'U', 200}, {'I', 100}, {'U', 300}, {'I', 100}, {'U', 500}, {'U', 600}, {'D', 100}, {'U', 400}, {'D', 100}, {'U', 200},
	{'D', 100}, {'U', 300}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 200}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 200},
	{'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200}, {'U', 100},
	{'D', 100}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 300}, {'U', 100}, {'D', 200}, {'U', 100},
	{'D', 400}, {'U', 100}, {'D', 600}, {'S', 250}
};

// Funcion para dibujar una cruz
void CRUZ(void){
	PLOTTER_IZQUIERDA_NO_BAJAR(); _delay_ms(2500); // Se mueve el plotter hacia la izquierda sin bajar la solenoide durante 2500 ms
	PLOTTER_ABAJO(); _delay_ms(5000); // Se mueve el plotter hacia abajo durante 5000 ms
	PLOTTER_SUBIR(); _delay_ms(250); // Se levanta la solenoide durante 250 ms
	PLOTTER_ARRIBA_NO_BAJAR(); _delay_ms(2500); // Se mueve el plotter hacia arriba sin bajar la solenoide durante 2500 ms
	PLOTTER_DERECHA_NO_BAJAR(); _delay_ms(2500); // Se mueve el plotter hacia la derecha sin bajar la solenoide durante 2500 ms
	PLOTTER_IZQUIERDA(); _delay_ms(5000); // Se mueve el plotter hacia la izquierda durante 5000 ms
	PLOTTER_SUBIR(); _delay_ms(250); // Se levanta la solenoide durante 250 ms
}

// Funcion para dibujar las figuras en secuencia
void FIGURAS(void){
	TRIANGULO(); // Dibuja un triángulo
	PLOTTER_IZQUIERDA_NO_BAJAR(); _delay_ms(10000); // Se mueve el plotter hacia la izquierda sin bajar la solenoide durante 10000 ms
	EJECUTAR_FIGURA(CIRCULO, sizeof(CIRCULO) / sizeof(Paso)); // Dibuja un círculo
	PLOTTER_IZQUIERDA_NO_BAJAR(); _delay_ms(5000); // Se mueve el plotter hacia la izquierda sin bajar la solenoide durante 5000 ms
	CRUZ(); // Dibuja una cruz
}

// Definicion de la figura zorro a través de una estructura de pasos
const Paso ZORRO[] PROGMEM = {
	{'i', 10000}, {'a', 3000}, {'D', 3900}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200}, {'U', 100},
	{'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200}, {'U', 100},
	{'D', 100}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200}, {'U', 100},
	{'D', 100}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 100}, {'U', 100},
	{'D', 200}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 100}, {'U', 100},
	{'D', 200}, {'U', 100}, {'D', 200}, {'A', 300}, {'I', 100}, {'A', 600}, {'I', 100}, {'A', 500}, {'I', 100}, {'A', 600},
	{'I', 100}, {'A', 500}, {'I', 100}, {'A', 600}, {'I', 100}, {'A', 500}, {'I', 100}, {'A', 600}, {'I', 100}, {'A', 300},
	{'D', 100}, {'A', 100}, {'D', 100}, {'A', 400}, {'D', 100}, {'A', 300}, {'D', 100}, {'A', 300}, {'D', 100}, {'A', 300},
	{'D', 100}, {'A', 300}, {'D', 100}, {'A', 300}, {'D', 100}, {'A', 300}, {'D', 100}, {'A', 300}, {'D', 100}, {'A', 300},
	{'D', 100}, {'A', 300}, {'D', 100}, {'A', 300}, {'D', 100}, {'A', 200}, {'I', 100}, {'A', 100}, {'I', 100}, {'A', 100},
	{'I', 200}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 200}, {'A', 100},
	{'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 100}, {'A', 100},
	{'I', 200}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 200}, {'A', 100},
	{'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 200}, {'A', 100},
	{'I', 200}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 100}, {'A', 100},
	{'I', 200}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 100}, {'A', 100},
	{'I', 200}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 300}, {'A', 100}, {'U', 100}, {'I', 300},
	{'U', 100}, {'I', 200}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 200},
	{'U', 100}, {'I', 200}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 200},
	{'U', 100}, {'I', 200}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 100},
	{'U', 100}, {'I', 200}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 200},
	{'U', 100}, {'I', 100}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 200},
	{'U', 100}, {'I', 200}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 200},
	{'U', 100}, {'I', 100}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 100},
	{'U', 200}, {'D', 100}, {'U', 300}, {'D', 100}, {'U', 300}, {'D', 100}, {'U', 300}, {'D', 100}, {'U', 300}, {'D', 100},
	{'U', 300}, {'D', 100}, {'U', 300}, {'D', 100}, {'U', 300}, {'D', 100}, {'U', 300}, {'D', 100}, {'U', 300}, {'D', 100},
	{'U', 300}, {'D', 100}, {'U', 400}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 300}, {'I', 100}, {'U', 600}, {'I', 100},
	{'U', 500}, {'I', 100}, {'U', 600}, {'I', 100}, {'U', 500}, {'I', 100}, {'U', 600}, {'I', 100}, {'U', 500}, {'I', 100},
	{'U', 600}, {'I', 100}, {'U', 300}, {'D', 200}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 200},
	{'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 200},
	{'A', 100}, {'D', 100}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 100},
	{'A', 100}, {'D', 100}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 100},
	{'A', 100}, {'D', 200}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 100},
	{'A', 100}, {'D', 200}, {'A', 200}, {'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 100},
	{'A', 100}, {'I', 200}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 100},
	{'A', 100}, {'I', 200}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 200},
	{'A', 100}, {'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 300}, {'A', 300}, {'D', 100}, {'A', 100}, {'D', 100},
	{'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100},
	{'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100},
	{'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100},
	{'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100},
	{'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 200}, {'I', 200},
	{'A', 100}, {'I', 400}, {'A', 100}, {'I', 300}, {'A', 100}, {'I', 500}, {'A', 100}, {'I', 400}, {'A', 100}, {'I', 400},
	{'A', 100}, {'I', 400}, {'A', 100}, {'I', 400}, {'A', 100}, {'I', 400}, {'A', 100}, {'I', 500}, {'S', 250}, {'d', 500},
	{'u', 100}, {'d', 400}, {'u', 100}, {'d', 400}, {'u', 100}, {'d', 400}, {'u', 100}, {'d', 400}, {'u', 100}, {'d', 400},
	{'u', 100}, {'d', 500}, {'u', 100}, {'d', 300}, {'u', 100}, {'d', 400}, {'u', 100}, {'d', 200}, {'B', 250}, {'D', 200},
	{'A', 300}, {'D', 100}, {'A', 400}, {'D', 100}, {'A', 400}, {'D', 100}, {'A', 400}, {'D', 100}, {'A', 400}, {'D', 100},
	{'A', 300}, {'D', 100}, {'A', 400}, {'D', 100}, {'A', 400}, {'D', 100}, {'A', 400}, {'D', 100}, {'A', 300}, {'D', 100},
	{'A', 200}, {'D', 200}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100},
	{'U', 100}, {'D', 100}, {'U', 200}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200}, {'A', 100}, {'D', 100},
	{'A', 100}, {'D', 100}, {'A', 200}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100},
	{'A', 100}, {'D', 100}, {'A', 100}, {'D', 200}, {'U', 200}, {'D', 100}, {'U', 300}, {'D', 100}, {'U', 400}, {'D', 100},
	{'U', 400}, {'D', 100}, {'U', 400}, {'D', 100}, {'U', 300}, {'D', 100}, {'U', 400}, {'D', 100}, {'U', 400}, {'D', 100},
	{'U', 400}, {'D', 100}, {'U', 400}, {'D', 100}, {'U', 300}, {'D', 300}, {'A', 100}, {'D', 400}, {'A', 100}, {'D', 400},
	{'A', 100}, {'D', 400}, {'A', 100}, {'D', 400}, {'A', 100}, {'D', 400}, {'A', 100}, {'D', 400}, {'A', 100}, {'D', 400},
	{'A', 100}, {'D', 400}, {'A', 100}, {'D', 600}, {'S', 250}, {'i', 600}, {'u', 100}, {'i', 400}, {'u', 100}, {'i', 400},
	{'u', 100}, {'i', 400}, {'u', 100}, {'i', 400}, {'u', 100}, {'i', 400}, {'u', 100}, {'i', 400}, {'u', 100}, {'i', 400},
	{'u', 100}, {'i', 400}, {'u', 100}, {'i', 100}, {'B', 250}, {'U', 200}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100},
	{'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100},
	{'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100},
	{'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100},
	{'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100},
	{'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 300}, {'I', 300}, {'U', 100},
	{'I', 200}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 100}, {'U', 100},
	{'I', 200}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 100}, {'U', 100},
	{'I', 200}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 200}, {'U', 100},
	{'I', 200}, {'A', 300}, {'I', 100}, {'A', 200}, {'I', 100}, {'A', 200}, {'I', 100}, {'A', 200}, {'I', 100}, {'A', 200},
	{'I', 100}, {'A', 100}, {'I', 100}, {'A', 200}, {'I', 100}, {'A', 200}, {'I', 100}, {'A', 200}, {'I', 100}, {'A', 200},
	{'I', 100}, {'A', 200}, {'I', 100}, {'A', 200}, {'I', 100}, {'A', 100}, {'I', 100}, {'A', 200}, {'I', 100}, {'A', 200},
	{'I', 100}, {'A', 200}, {'I', 100}, {'A', 200}, {'I', 100}, {'A', 200}, {'I', 200}, {'U', 200}, {'I', 100}, {'U', 200},
	{'I', 100}, {'U', 200}, {'I', 100}, {'U', 200}, {'I', 100}, {'U', 200}, {'I', 100}, {'U', 100}, {'I', 100}, {'U', 200},
	{'I', 100}, {'U', 200}, {'I', 100}, {'U', 200}, {'I', 100}, {'U', 200}, {'I', 100}, {'U', 200}, {'I', 100}, {'U', 200},
	{'I', 100}, {'U', 100}, {'I', 100}, {'U', 200}, {'I', 100}, {'U', 200}, {'I', 100}, {'U', 200}, {'I', 100}, {'U', 200},
	{'I', 100}, {'U', 400}, {'S', 250}, {'d', 1800}, {'a', 3400}, {'A', 4300}, {'S', 250}, {'u', 4300}, {'B', 250}, {'D', 200},
	{'A', 100}, {'D', 200}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 200},
	{'A', 100}, {'D', 200}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 200},
	{'A', 100}, {'S', 250}, {'a', 300}, {'i', 100}, {'a', 400}, {'i', 100}, {'a', 300}, {'B', 250}, {'D', 200}, {'U', 100},
	{'D', 200}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 200}, {'U', 100},
	{'D', 200}, {'U', 100}, {'S', 250}, {'i', 6300}, {'B', 250}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 200}, {'A', 100},
	{'D', 200}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 200}, {'S', 250},
	{'u', 1000}, {'i', 200}, {'B', 250}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 200}, {'U', 100},
	{'D', 100}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200}, {'U', 100},
	{'D', 200}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 200}, {'S', 250}
};

// Definicion de la figura flor a través de una estructura de pasos
const Paso FLOR[] PROGMEM = {
	{'i', 10000}, {'B', 250}, {'D', 200}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 200},
	{'D', 100}, {'A', 100}, {'D', 500}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 200}, {'U', 100},
	{'D', 400}, {'A', 400}, {'D', 100}, {'A', 700}, {'D', 200}, {'U', 100}, {'D', 300}, {'U', 100}, {'D', 700}, {'A', 600},
	{'I', 200}, {'A', 400}, {'D', 800}, {'A', 100}, {'D', 300}, {'A', 100}, {'D', 100}, {'A', 200}, {'I', 100}, {'A', 100},
	{'I', 100}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 300}, {'A', 200},
	{'D', 500}, {'A', 100}, {'D', 300}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 200},
	{'I', 100}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 500}, {'A', 200},
	{'D', 200}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 200},
	{'I', 100}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 900}, {'A', 200}, {'D', 100}, {'A', 200},
	{'D', 100}, {'A', 100}, {'D', 100}, {'A', 500}, {'I', 700}, {'U', 100}, {'I', 300}, {'U', 100}, {'I', 200}, {'A', 700},
	{'I', 100}, {'A', 400}, {'I', 500}, {'U', 100}, {'I', 400}, {'U', 100}, {'I', 100}, {'U', 200}, {'I', 500}, {'A', 100},
	{'I', 100}, {'A', 200}, {'I', 100}, {'A', 200}, {'I', 200}, {'A', 100}, {'I', 100}, {'U', 100}, {'I', 200}, {'U', 200},
	{'I', 100}, {'U', 200}, {'I', 100}, {'U', 100}, {'I', 500}, {'A', 200}, {'I', 100}, {'A', 100}, {'I', 400}, {'A', 100},
	{'I', 400}, {'U', 400}, {'I', 100}, {'U', 700}, {'I', 200}, {'A', 100}, {'I', 300}, {'A', 100}, {'I', 700}, {'U', 500},
	{'D', 100}, {'U', 100}, {'D', 100}, {'U', 200}, {'D', 100}, {'U', 100}, {'I', 900}, {'U', 100}, {'I', 200}, {'U', 100},
	{'I', 100}, {'U', 100}, {'I', 100}, {'U', 200}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200}, {'U', 100},
	{'D', 200}, {'U', 100}, {'D', 200}, {'U', 200}, {'I', 500}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 100}, {'U', 100},
	{'I', 100}, {'U', 100}, {'I', 100}, {'U', 200}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100},
	{'D', 300}, {'U', 100}, {'D', 500}, {'U', 200}, {'I', 300}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 100}, {'U', 100},
	{'I', 100}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 100}, {'U', 200}, {'D', 100}, {'U', 100}, {'D', 300}, {'U', 100},
	{'D', 800}, {'U', 400}, {'I', 200}, {'U', 600}, {'D', 700}, {'A', 100}, {'D', 300}, {'A', 100}, {'D', 200}, {'U', 700},
	{'D', 100}, {'U', 400}, {'D', 400}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 100}, {'A', 100},
	{'D', 500}, {'U', 100}, {'D', 100}, {'U', 200}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100},
	{'S', 250}, {'a', 1700}, {'B', 250}, {'D', 500}, {'A', 100}, {'D', 300}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 200},
	{'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100},
	{'A', 100}, {'D', 100}, {'A', 200}, {'D', 100}, {'A', 200}, {'D', 100}, {'A', 300}, {'D', 100}, {'A', 800}, {'I', 100},
	{'A', 300}, {'I', 100}, {'A', 200}, {'I', 100}, {'A', 200}, {'I', 100}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 100},
	{'A', 100}, {'I', 100}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 200},
	{'A', 100}, {'I', 300}, {'A', 100}, {'I', 900}, {'U', 100}, {'I', 300}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 200},
	{'U', 100}, {'I', 100}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 100},
	{'U', 100}, {'I', 100}, {'U', 200}, {'I', 100}, {'U', 200}, {'I', 100}, {'U', 300}, {'I', 100}, {'U', 800}, {'D', 100},
	{'U', 300}, {'D', 100}, {'U', 200}, {'D', 100}, {'U', 200}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100},
	{'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 200},
	{'U', 100}, {'D', 300}, {'U', 100}, {'D', 400}, {'S', 250}, {'a', 5600}, {'i', 200}, {'B', 250}, {'A', 5400}, {'D', 500},
	{'U', 5400}, {'S', 250}, {'a', 1100}, {'B', 250}, {'D', 200}, {'U', 100}, {'D', 100}, {'U', 200}, {'D', 200}, {'U', 200},
	{'D', 200}, {'U', 100}, {'D', 500}, {'U', 200}, {'D', 700}, {'U', 100}, {'D', 1300}, {'A', 100}, {'D', 500}, {'A', 100},
	{'D', 100}, {'A', 100}, {'D', 400}, {'A', 100}, {'D', 300}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 400},
	{'I', 400}, {'U', 100}, {'I', 600}, {'A', 100}, {'I', 700}, {'A', 100}, {'I', 300}, {'A', 100}, {'I', 100}, {'A', 100},
	{'I', 300}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 400}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 600}, {'U', 100},
	{'I', 300}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 300}, {'D', 300},
	{'U', 100}, {'D', 500}, {'U', 100}, {'D', 500}, {'U', 100}, {'D', 500}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 300},
	{'U', 100}, {'D', 1500}, {'S', 250}, {'i', 7900}, {'B', 250}, {'D', 1500}, {'A', 100}, {'D', 300}, {'A', 100}, {'D', 100},
	{'A', 100}, {'D', 500}, {'A', 100}, {'D', 500}, {'A', 100}, {'D', 500}, {'A', 100}, {'D', 300}, {'U', 300}, {'I', 300},
	{'U', 100}, {'I', 100}, {'U', 200}, {'I', 200}, {'U', 200}, {'I', 200}, {'U', 100}, {'I', 500}, {'U', 200}, {'I', 700},
	{'U', 100}, {'I', 1300}, {'A', 100}, {'I', 500}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 400}, {'A', 100}, {'I', 300},
	{'A', 100}, {'I', 100}, {'A', 100}, {'I', 100}, {'A', 400}, {'D', 400}, {'U', 100}, {'D', 600}, {'A', 100}, {'D', 700},
	{'A', 100}, {'D', 300}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 300}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 400},
	{'A', 100}, {'D', 100}, {'A', 100}, {'D', 600}, {'U', 100}, {'D', 300}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 100},
	{'U', 100}, {'D', 200}, {'U', 100}, {'S', 250}, {'u', 1900}, {'B', 250}, {'U', 300}, {'I', 100}, {'U', 100}, {'I', 100},
	{'U', 800}, {'I', 400}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 100}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 100},
	{'A', 100}, {'I', 200}, {'S', 250}, {'U', 1600}, {'B', 250}, {'I', 300}, {'A', 100}, {'I', 200}, {'A', 100}, {'I', 100},
	{'A', 100}, {'I', 100}, {'A', 100}, {'I', 100}, {'A', 200}, {'I', 100}, {'A', 200}, {'S', 250}, {'i', 500}, {'u', 900},
	{'B', 250}, {'D', 1400}, {'S', 250}, {'d', 3900}, {'B', 250}, {'D', 1400}, {'S', 250}, {'u', 1300}, {'B', 250}, {'I', 1400},
	{'U', 100}, {'D', 200}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200},
	{'U', 200}, {'D', 100}, {'U', 300}, {'D', 100}, {'S', 250}, {'i', 5800}, {'B', 250}, {'A', 300}, {'D', 100}, {'A', 200},
	{'D', 200}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 300}, {'A', 100},
	{'i', 1300}, {'S', 250}, {'u', 1700}, {'d', 1300}, {'B', 250}, {'D', 200}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 100},
	{'A', 100}, {'D', 200}, {'A', 100}, {'D', 200}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 200}, {'U', 900}, {'D', 100},
	{'U', 100}, {'D', 100}, {'U', 300}, {'S', 250}, {'d', 1100}, {'B', 250}, {'A', 300}, {'D', 100}, {'A', 100}, {'D', 100},
	{'A', 900}, {'D', 200}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 100},
	{'U', 100}, {'D', 200}, {'U', 100}, {'D', 200}, {'S', 250}, {'a', 3200}, {'B', 250}, {'D', 300}, {'A', 100}, {'D', 200},
	{'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 100}, {'D', 100}, {'A', 200}, {'D', 100}, {'A', 200}, {'S', 250},
	{'a', 800}, {'i', 800}, {'B', 250}, {'I', 300}, {'U', 100}, {'I', 100}, {'U', 100}, {'I', 200}, {'U', 100}, {'I', 100},
	{'U', 100}, {'I', 200}, {'U', 100}, {'I', 400}, {'A', 800}, {'I', 100}, {'A', 100}, {'I', 100}, {'A', 300}, {'S', 250}
};

// Función para mostrar el menú de opciones
void MENU(void){
	UART_TX_CADENA("\r\nSeleccione: 1 = Triangulo, 2 = Circulo, 3 = Cruz, 4 = Todas las figuras, Z = Zorro, F = Flor\r\n");
}

// Función principal
int main(void){
	DDRD = 0xFC; // Se configuran PD0 y PD1 como entradas, el resto como salidas
	PORTD = 0x00; // Se inicializa PORTD en 0
	UART_INICIAR(); // Se inicializa la comunicación UART
	char opcion; // Variable para almacenar la opción seleccionada

	// Bucle principal
	while (1){
		MENU(); // Se muestra el menú de opciones
		opcion = UART_RX(); // Se almacena la opción seleccionada por el usuario
		// Se ejecuta la figura correspondiente según la opción seleccionada
		switch (opcion){
			case '1': TRIANGULO(); // Se dibuja un triángulo
			break; // Se sale del switch
			case '2': EJECUTAR_FIGURA(CIRCULO, sizeof(CIRCULO) / sizeof(Paso)); // Se dibuja un círculo
			break; // Se sale del switch
			case '3': CRUZ(); // Se dibuja una cruz
			break; // Se sale del switch
			case '4': FIGURAS(); // Se dibujan todas las figuras
			break; // Se sale del switch
			case 'Z': EJECUTAR_FIGURA(ZORRO, sizeof(ZORRO) / sizeof(Paso)); // Se dibuja un zorro
			break; // Se sale del switch
			case 'F': EJECUTAR_FIGURA(FLOR, sizeof(FLOR) / sizeof(Paso)); // Se dibuja una flor
			break; // Se sale del switch
			default: UART_TX_CADENA("\r\n¡Error! Opción inválida\r\n"); // Se notifica al usuario sobre la opción inválida
			break; // Se sale del switch
		}
	}
}
