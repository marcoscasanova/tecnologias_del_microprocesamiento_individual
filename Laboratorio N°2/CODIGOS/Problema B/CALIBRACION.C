#define F_CPU 16000000UL // Se define la frecuencia del CPU en 16 MHz
#define BAUD 9600 // Se define la velocidad de comunicación en baudios
#define BPS ((F_CPU / (16UL * BAUD)) - 1) // Se calcula el valor del registro UBRR para la velocidad definida

#include <avr/io.h> // Librería de entrada/salida del microcontrolador AVR
#include <util/delay.h> // Librería para generar retardos temporales en milisegundos

#define LED_R PD2 // Se asigna el pin PD2 como LED rojo
#define LED_G PD3 // Se asigna el pin PD3 como LED verde
#define LED_B PD4 // Se asigna el pin PD4 como LED azul

#define ADC_CANAL_LDR 0 // Se define el canal ADC0 para la lectura de la LDR
#define N_MUESTRAS 30 // Se define la cantidad de muestras a promediar por medición

// Función para inicializar la comunicación UART
void UART_INICIAR(void) {
	UBRR0H = (uint8_t)(BPS >> 8); // Se carga la parte alta del registro UBRR con el valor calculado
	UBRR0L = (uint8_t)(BPS); // Se carga la parte baja del registro UBRR con el valor calculado
	UCSR0B = (1 << RXEN0) | (1 << TXEN0); // Se habilitan los módulos de recepción y transmisión
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Se configura la trama con 8 bits de datos, sin paridad y 1 bit de stop
}

// Función para transmitir un carácter por UART
void UART_TX(char c) {
	while (!(UCSR0A & (1 << UDRE0))); // Se espera a que el registro de transmisión esté libre
	UDR0 = c; // Se escribe el carácter en el registro de transmisión
}

// Función para transmitir una cadena de caracteres por UART
void UART_TX_CADENA(const char *s) {
	while (*s) UART_TX(*s++); // Se transmiten los caracteres uno a uno hasta encontrar el terminador nulo
}

// Función para recibir un carácter por UART
char UART_RX(void) {
	while (!(UCSR0A & (1 << RXC0))); // Se espera a que se reciba un carácter completo
	return UDR0; // Se retorna el carácter recibido
}

// Función para imprimir un número entero sin signo por UART
void UART_IMPRIMIR_UINT(uint16_t num) {
	char buf[6]; // Se crea un buffer para almacenar los dígitos
	uint8_t i = 0; // Índice para recorrer el buffer
	if (num == 0) { // Si el número es cero
		UART_TX('0'); // Se envía el carácter '0'
		return; // Se sale de la función
	}
	while (num > 0 && i < 5) { // Se descompone el número en dígitos individuales
		buf[i++] = (num % 10) + '0'; // Se convierte cada dígito a carácter ASCII
		num /= 10; // Se reduce el número dividiéndolo entre 10
	}
	while (i > 0) UART_TX(buf[--i]); // Se transmiten los dígitos en orden inverso
}

// Función para inicializar el ADC
void ADC_INICIAR(void) {
	ADMUX  = (1 << REFS0); // Se selecciona AVCC como referencia de voltaje
	ADCSRA = (1 << ADEN)  | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Se habilita el ADC con prescaler 128
}

// Función para leer un valor analógico desde un canal específico
uint16_t ADC_LEER(uint8_t canal) {
	ADMUX = (ADMUX & 0xF0) | (canal & 0x0F); // Se selecciona el canal de entrada del ADC
	ADCSRA |= (1 << ADSC); // Se inicia la conversión
	while (ADCSRA & (1 << ADSC)); // Se espera hasta que la conversión finalice
	return ADC; // Se retorna el valor convertido (10 bits)
}

// Función para inicializar los pines de los LEDs como salida
void LEDS_INICIAR(void) {
	DDRD |= (1 << LED_R) | (1 << LED_G) | (1 << LED_B); // Se configuran los pines PD2, PD3 y PD4 como salidas
}

// Función para apagar todos los LEDs
static inline void LEDS_OFF(void){
	PORTD &= ~((1 << LED_R) | (1 << LED_G) | (1 << LED_B)); // Se apagan los tres LEDs
}
static inline void LED_R_ON(void)  { PORTD |=  (1 << LED_R); }  // Se enciende el LED rojo
static inline void LED_R_OFF(void) { PORTD &= ~(1 << LED_R); }  // Se apaga el LED rojo
static inline void LED_G_ON(void)  { PORTD |=  (1 << LED_G); }  // Se enciende el LED verde
static inline void LED_G_OFF(void) { PORTD &= ~(1 << LED_G); }  // Se apaga el LED verde
static inline void LED_B_ON(void)  { PORTD |=  (1 << LED_B); }  // Se enciende el LED azul
static inline void LED_B_OFF(void) { PORTD &= ~(1 << LED_B); }  // Se apaga el LED azul

// Función para calcular el promedio de n lecturas ADC en un canal
uint16_t PROMEDIO(uint8_t canal, uint8_t n) {
	uint32_t acc = 0; // Acumulador de lecturas
	for (uint8_t i = 0; i < n; i++) { // Se repite n veces
		acc += ADC_LEER(canal); // Se suma el valor leído
		_delay_ms(5); // Se agrega un pequeño retardo entre lecturas
	}
	return (uint16_t)(acc / n); // Se devuelve el promedio como entero
}

// Función para capturar el nombre del color antes de medir
uint8_t CALIBRAR(char *nombre, uint8_t maxlen) {
	uint8_t i = 0; // Índice para almacenar caracteres del nombre
	UART_TX_CADENA("\r\nColor (escriba nombre y luego 'w' para medir, '0' para terminar): "); // Se pide ingreso por UART
	for (;;) { // Bucle infinito hasta recibir 'w' o '0'
		char c = UART_RX(); // Se lee un carácter del usuario
		if (c == '\r' || c == '\n') continue; // Se ignoran ENTER o salto de línea
		if (i == 0 && c == '0') { // Si el primer carácter es '0'
			nombre[0] = '0'; // Se guarda '0' en la cadena
			nombre[1] = '\0'; // Se termina la cadena con nulo
			UART_TX_CADENA("\r\nFin de calibracion.\r\n"); // Se notifica fin de calibración
			return 0; // Se retorna 0 para indicar finalización
		}
		if (c == 'w') { nombre[i] = '\0'; UART_TX_CADENA("\r\n"); return 1; } // Si se recibe 'w', se termina la captura y se inicia medición
		if (i < (maxlen - 1)) { nombre[i++] = c; UART_TX(c); } // Se guarda el carácter en el buffer y se ecoa por UART
	}
}

// Función principal del programa
int main(void) {
	UART_INICIAR(); // Se inicializa la comunicación UART
	ADC_INICIAR(); // Se inicializa el conversor ADC
	LEDS_INICIAR(); // Se inicializan los pines de los LEDs

	char nombre[32]; // Buffer para almacenar el nombre del color actual

	UART_TX_CADENA("\r\n=== Calibracion RGB con LDR (sin ENTER, sin CSV) ===\r\n"); // Mensaje inicial en la terminal

	while (1) { // Bucle principal de calibración
		uint8_t medir = CALIBRAR(nombre, sizeof(nombre)); // Se solicita el nombre y se espera la orden para medir
		if (!medir) break; // Si se recibe '0', se termina el proceso de calibración

		uint16_t Roff, Ron, Goff, Gon, Boff, Bon; // Variables para almacenar lecturas de cada color (apagado/encendido)
		uint16_t dR, dG, dB; // Diferencias de reflexión para cada color

		UART_TX_CADENA("Midiendo...\r\n"); // Mensaje de inicio de medición

		// ----- ROJO -----
		LEDS_OFF(); _delay_ms(50); // Se apagan todos los LEDs y se espera un breve tiempo
		Roff = PROMEDIO(ADC_CANAL_LDR, N_MUESTRAS); // Se mide la luz ambiente (LED apagado)
		LED_R_ON(); _delay_ms(100); // Se enciende el LED rojo
		Ron = PROMEDIO(ADC_CANAL_LDR, N_MUESTRAS); // Se mide la luz reflejada (LED encendido)
		LED_R_OFF(); // Se apaga el LED rojo

		// ----- VERDE -----
		LEDS_OFF(); _delay_ms(50); // Se apagan todos los LEDs
		Goff = PROMEDIO(ADC_CANAL_LDR, N_MUESTRAS); // Se mide la luz ambiente sin LED verde
		LED_G_ON(); _delay_ms(100); // Se enciende el LED verde
		Gon = PROMEDIO(ADC_CANAL_LDR, N_MUESTRAS); // Se mide la luz reflejada con LED verde
		LED_G_OFF(); // Se apaga el LED verde

		// ----- AZUL -----
		LEDS_OFF(); _delay_ms(50); // Se apagan todos los LEDs
		Boff = PROMEDIO(ADC_CANAL_LDR, N_MUESTRAS); // Se mide la luz ambiente sin LED azul
		LED_B_ON(); _delay_ms(100); // Se enciende el LED azul
		Bon = PROMEDIO(ADC_CANAL_LDR, N_MUESTRAS); // Se mide la luz reflejada con LED azul
		LED_B_OFF(); // Se apaga el LED azul

		dR = (Ron > Roff) ? (Ron - Roff) : 0; // Se calcula la diferencia de reflexión en el canal rojo
		dG = (Gon > Goff) ? (Gon - Goff) : 0; // Se calcula la diferencia en el canal verde
		dB = (Bon > Boff) ? (Bon - Boff) : 0; // Se calcula la diferencia en el canal azul

		UART_TX_CADENA("COLOR="); // Se imprime el encabezado del color
		UART_TX_CADENA(nombre); // Se imprime el nombre del color medido
		UART_TX_CADENA(" | R:"); // Se imprime etiqueta para canal rojo
		UART_IMPRIMIR_UINT(Ron); // Se imprime valor del LED rojo encendido
		UART_TX('/'); UART_IMPRIMIR_UINT(Roff); // Se imprime valor del LED rojo apagado
		UART_TX_CADENA("  G:"); // Se imprime etiqueta para canal verde
		UART_IMPRIMIR_UINT(Gon); // Se imprime valor del LED verde encendido
		UART_TX('/'); UART_IMPRIMIR_UINT(Goff); // Se imprime valor del LED verde apagado
		UART_TX_CADENA("  B:"); // Se imprime etiqueta para canal azul
		UART_IMPRIMIR_UINT(Bon); // Se imprime valor del LED azul encendido
		UART_TX('/'); UART_IMPRIMIR_UINT(Boff); // Se imprime valor del LED azul apagado
		UART_TX_CADENA("  |  dR="); UART_IMPRIMIR_UINT(dR); // Se imprime diferencia de R
		UART_TX_CADENA("  dG="); UART_IMPRIMIR_UINT(dG); // Se imprime diferencia de G
		UART_TX_CADENA("  dB="); UART_IMPRIMIR_UINT(dB); // Se imprime diferencia de B
		UART_TX_CADENA("\r\n"); // Se agrega salto de línea

		UART_TX_CADENA("Listo. Ingrese otro color o '0' para terminar.\r\n"); // Se muestra mensaje para continuar o finalizar
	}

	while (1) { } // Bucle infinito al final del programa (se detiene la ejecución)
}
