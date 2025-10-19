#define F_CPU 16000000UL // Se define la frecuencia del CPU a 16 MHz
#define BAUD 9600 // Se define la velocidad de comunicación en baudios
#define BPS ((F_CPU / (16UL * BAUD)) - 1) // Se calcula el valor de UBRR para configurar 9600 baudios con reloj a 16 MHz

#include <avr/io.h> // Librería de E/S del microcontrolador AVR (registros de puertos, periféricos)
#include <util/delay.h> // Librería para funciones de retardo basadas en F_CPU
#include <math.h> // Librería matemática (uso de sqrtf y operaciones de punto flotante)
#include <avr/interrupt.h> // Librería para manejo de interrupciones (cli/sei)

void UART_INICIAR(void){ // Función para inicializar la comunicación UART (USART0)
	UBRR0H = (uint8_t)(BPS >> 8); // Se carga la parte alta de UBRR0 con el valor calculado
	UBRR0L = (uint8_t)(BPS); // Se carga la parte baja de UBRR0 con el valor calculado
	UCSR0B = (1 << RXEN0) | (1 << TXEN0); // Se habilitan RX (recepción) y TX (transmisión)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Se configuran 8 bits de datos, sin paridad, 1 bit de stop
}
void UART_TX(char c){ // Función para transmitir un carácter por UART
	while (!(UCSR0A & (1 << UDRE0))); // Se espera a que el buffer de transmisión esté libre (UDRE0=1)
	UDR0 = c; // Se escribe el carácter a transmitir en el registro UDR0
}
void UART_TX_CADENA(const char *s){ // Función para transmitir una cadena terminada en '\0'
	while (*s) UART_TX(*s++); // Se envía carácter a carácter hasta el fin de la cadena
}
void UART_IMPRIMIR_UINT(uint16_t n){ // Función para imprimir un entero sin signo (0..65535) por UART
	char buf[6]; uint8_t i=0; // Buffer temporal para dígitos (máx 5 dígitos + '\0'), índice de llenado
	if(n==0){UART_TX('0');return;} // Caso especial: si n es 0, se imprime '0' y se retorna
	while(n>0 && i<5){ buf[i++] = (n%10)+'0'; n/=10; } // Se descompone n en dígitos (base 10) en orden inverso
	while(i>0) UART_TX(buf[--i]); // Se envían los dígitos en orden correcto (del más significativo al menos)
}

void ADC_INICIAR(void){ // Función para inicializar el ADC
	ADMUX = (1 << REFS0); // Se selecciona referencia AVcc con capacitor en AREF (REFS0=1, REFS1=0)
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Se habilita ADC y se fija prescaler a 128 (125 kHz a 16 MHz)
}
uint16_t ADC_LEER(uint8_t canal){ // Función para leer el ADC en el canal indicado (0..7)
	ADMUX = (ADMUX & 0xF0) | (canal & 0x0F); // Se preservan bits altos y se selecciona el canal en los 4 bits bajos
	ADCSRA |= (1 << ADSC); // Se inicia la conversión ADC (ADSC=1)
	while (ADCSRA & (1 << ADSC)); // Se espera a que la conversión finalice (ADSC=0 cuando termina)
	return ADC; // Se retorna el valor de 10 bits del registro ADC (ADCL+ADCH)
}
uint16_t PROMEDIO(uint8_t canal, uint8_t n){ // Función para promediar n lecturas ADC en un canal
	uint32_t acc=0; // Acumulador de 32 bits para evitar overflow durante la suma
	for(uint8_t i=0;i<n;i++){ acc += ADC_LEER(canal); _delay_ms(5); } // Se suman n lecturas con breve retardo entre ellas
	return (uint16_t)(acc/n); // Se retorna el promedio entero de las lecturas
}

void LEDS_INICIAR(void){ // Función para inicializar pines de LEDs discretos (R,G,B) en PORTD
	DDRD |= (1 << PD2) | (1 << PD3) | (1 << PD4); // Se configuran PD2, PD3 y PD4 como salidas
}
static inline void LEDS_OFF(void){ PORTD &= ~((1<<PD2)|(1<<PD3)|(1<<PD4)); } // Se apagan los tres LEDs (R,G,B)
static inline void LED_R_ON(void){ PORTD |= (1<<PD2); } // Se enciende LED rojo en PD2
static inline void LED_G_ON(void){ PORTD |= (1<<PD3); } // Se enciende LED verde en PD3
static inline void LED_B_ON(void){ PORTD |= (1<<PD4); } // Se enciende LED azul en PD4

void SERVO_INICIAR(void){ // Función para inicializar PWM en Timer1 para control de servo (OC1A/PB1)
	DDRB |= (1 << PB1); // Se configura PB1 (OC1A) como salida
	TCCR1A = (1 << COM1A1) | (1 << WGM11); // Se habilita salida no inversora en OC1A y modo PWM con ICR1 como TOP (parte baja)
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Se completa WGM para modo 14 (Fast PWM) y prescaler = 8
	ICR1 = 39999; // Se fija TOP=39999 para periodo de 20 ms (50 Hz) con Fclk=2 MHz (16 MHz / 8)
}
void SERVO_POS(uint16_t grados){ // Función para posicionar el servo en un ángulo (0..180°)
	if (grados > 180) grados = 180; // Se limita el valor máximo del ángulo a 180°
	uint16_t pulso = 800 + ((uint32_t)grados * (2200 - 800)) / 180; // Se calcula ancho de pulso en µs (aprox 0.8–2.2 ms)
	OCR1A = pulso * 2; // Se carga OCR1A en ticks de 0.5 µs (2 MHz) multiplicando microsegundos por 2
}

#define NUM_LEDS 64 // Se define la cantidad de LEDs en la tira WS2812
#define LED_PIN PB0 // Se define el pin de datos para WS2812 como PB0

uint8_t leds[NUM_LEDS][3]; // [G,R,B] para cada LED (formato requerido por WS2812: primero G, luego R, luego B)

void sendBit(uint8_t bitVal){ // Rutina de envío de un bit para WS2812 con temporización por NOPs
	if(bitVal){ // Si el bit es 1
		// Bit 1: ~0.7us alto, ~0.55us bajo (timings aproximados para WS2812 a 16 MHz)
		PORTB |= (1 << LED_PIN); // Se coloca el pin en alto
		__asm__ __volatile__( // Se insertan NOPs para sostener el tiempo en alto
		"nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t" // ~10 NOPs para ajustar ~0.7 µs
		);
		PORTB &= ~(1 << LED_PIN); // Se coloca el pin en bajo
		__asm__ __volatile__( // Se insertan NOPs para sostener el tiempo en bajo
		"nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t" // ~9 NOPs para ajustar ~0.55 µs
		);
		} else { // Si el bit es 0
		// Bit 0: ~0.35us alto, ~0.9us bajo (timings aproximados para WS2812 a 16 MHz)
		PORTB |= (1 << LED_PIN); // Se coloca el pin en alto
		__asm__ __volatile__( // Se insertan NOPs para sostener el tiempo en alto más corto
		"nop\n\tnop\n\tnop\n\tnop\n\t" // ~4 NOPs para ~0.35 µs
		);
		PORTB &= ~(1 << LED_PIN); // Se coloca el pin en bajo
		__asm__ __volatile__( // Se insertan NOPs para sostener el tiempo en bajo más largo
		"nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t" // ~14 NOPs para ~0.9 µs
		);
	}
}

void sendByte(uint8_t byte){ // Rutina para enviar un byte MSB-first a la tira WS2812
	for(uint8_t i = 0; i < 8; i++){ // Bucle de 8 bits
		sendBit(byte & (1 << (7 - i))); // Se envía el bit correspondiente desde el más significativo al menos
	}
}

void show(uint8_t (*colors)[3]){ // Función para enviar el arreglo completo de colores a la tira WS2812
	cli(); // Se deshabilitan interrupciones para asegurar timing preciso durante la transmisión
	for(int i = 0; i < NUM_LEDS; i++){ // Se recorren todos los LEDs
		sendByte(colors[i][0]); // Se envía canal G del LED i
		sendByte(colors[i][1]); // Se envía canal R del LED i
		sendByte(colors[i][2]); // Se envía canal B del LED i
	}
	sei(); // Se vuelven a habilitar interrupciones tras la transmisión
	_delay_us(80); // Se espera el tiempo de reset (>50 µs) para latch de WS2812
}

void setLedRGB(uint8_t (*leds)[3], int ledIndex, uint8_t r, uint8_t g, uint8_t b){ // Función para setear color RGB en un LED
	if(ledIndex < 0 || ledIndex >= NUM_LEDS) return; // Se valida el índice para evitar accesos fuera de rango
	leds[ledIndex][0] = g; // Se guarda componente G en la posición 0 (formato WS2812)
	leds[ledIndex][1] = r; // Se guarda componente R en la posición 1
	leds[ledIndex][2] = b; // Se guarda componente B en la posición 2
}

void WS2812_INIT(void){ // Función para inicializar el pin de datos de WS2812
	DDRB |= (1 << LED_PIN); // Se configura PB0 como salida
	PORTB &= ~(1 << LED_PIN); // Se asegura nivel bajo inicial en la línea de datos
	_delay_ms(1); // Pequeño retardo para estabilidad antes de la primera transmisión
}

typedef struct { // Estructura para definir rangos por color y metadatos
	uint16_t minR, maxR; // Rango mínimo y máximo de dR (diferencia rojo)
	uint16_t minG, maxG; // Rango mínimo y máximo de dG (diferencia verde)
	uint16_t minB, maxB; // Rango mínimo y máximo de dB (diferencia azul)
	const char *nombre; // Nombre legible del color detectado
	uint16_t angulo; // Ángulo de servo asociado a este color
} ColorRange;

const ColorRange colores[] = { // Tabla de colores de referencia (rangos y ángulos)
	{241,274, 287,367,  70,138, "VERDE",    155}, // Rango VERDE: límites dR/dG/dB y ángulo 155°
	{309,341, 274,282,  68, 86, "AMARILLO", 110}, // Rango AMARILLO: límites dR/dG/dB y ángulo 110°
	{340,354, 119,142,  78, 86, "ROJO",      70}, // Rango ROJO: límites dR/dG/dB y ángulo 70°
	{266,382, 140,155, 135,150, "MORADO",    25}  // Rango MORADO: límites dR/dG/dB y ángulo 25°
}; // Se cierra el arreglo de estructuras de color
const uint8_t N_COLORES = sizeof(colores)/sizeof(colores[0]); // Se calcula el número de entradas en la tabla de colores

int main(void){ // Función principal del programa
	UART_INICIAR(); // Se inicializa la UART para depuración/telemetría
	ADC_INICIAR(); // Se inicializa el ADC para lecturas de la LDR
	LEDS_INICIAR(); // Se inicializan los pines de control de LEDs discretos (R,G,B)
	SERVO_INICIAR(); // Se inicializa el PWM del servo en Timer1 (OC1A)
	WS2812_INIT(); // Se inicializa la línea de datos de la tira WS2812

	uint16_t Roff,Ron,Goff,Gon,Boff,Bon,dR,dG,dB; // Variables para medidas con LED apagado/encendido y sus diferencias

	while(1){ // Bucle principal
		Roff = PROMEDIO(0,30); // Se mide referencia (LED rojo apagado) en canal ADC0 promediando 30 muestras
		LED_R_ON(); _delay_ms(100); // Se enciende LED rojo y se espera estabilización 100 ms
		Ron = PROMEDIO(0,30); // Se mide con LED rojo encendido en canal ADC0 promediando 30 muestras
		LEDS_OFF(); // Se apagan LEDs discretos
		dR = (Ron>Roff)?(Ron-Roff):0; // Se calcula diferencia dR asegurando no-negatividad

		Goff = PROMEDIO(0,30); // Se mide referencia (LED verde apagado) en canal ADC0 promediando 30 muestras
		LED_G_ON(); _delay_ms(100); // Se enciende LED verde y se espera estabilización 100 ms
		Gon = PROMEDIO(0,30); // Se mide con LED verde encendido en canal ADC0 promediando 30 muestras
		LEDS_OFF(); // Se apagan LEDs discretos
		dG = (Gon>Goff)?(Gon-Goff):0; // Se calcula diferencia dG asegurando no-negatividad

		Boff = PROMEDIO(0,30); // Se mide referencia (LED azul apagado) en canal ADC0 promediando 30 muestras
		LED_B_ON(); _delay_ms(100); // Se enciende LED azul y se espera estabilización 100 ms
		Bon = PROMEDIO(0,30); // Se mide con LED azul encendido en canal ADC0 promediando 30 muestras
		LEDS_OFF(); // Se apagan LEDs discretos
		dB = (Bon>Boff)?(Bon-Boff):0; // Se calcula diferencia dB asegurando no-negatividad

		uint8_t colorID = 255; // Identificador de color (255 indica "no clasificado" inicialmente)
		for(uint8_t i=0;i<N_COLORES;i++){ // Se recorre la tabla de rangos para búsqueda exacta por pertenencia
			if(dR>=colores[i].minR && dR<=colores[i].maxR && // Condición de dR dentro del rango del color i
			dG>=colores[i].minG && dG<=colores[i].maxG && // Condición de dG dentro del rango del color i
			dB>=colores[i].minB && dB<=colores[i].maxB){ // Condición de dB dentro del rango del color i
				colorID = i; break; // Si coincide en los tres canales, se selecciona el color i y se sale del bucle
			}
		}
		if(colorID==255){ // Si no hubo coincidencia exacta en rangos
			float minDist = 99999; // Se inicializa distancia mínima con un valor grande
			for(uint8_t i=0;i<N_COLORES;i++){ // Se recorre la tabla para clasificación por proximidad (distancia euclídea)
				float cR = (colores[i].minR + colores[i].maxR)/2.0f; // Se calcula centro de rango R del color i
				float cG = (colores[i].minG + colores[i].maxG)/2.0f; // Se calcula centro de rango G del color i
				float cB = (colores[i].minB + colores[i].maxB)/2.0f; // Se calcula centro de rango B del color i
				float dist = sqrtf((dR-cR)*(dR-cR) + (dG-cG)*(dG-cG) + (dB-cB)*(dB-cB)); // Se calcula distancia euclídea a (dR,dG,dB)
				if(dist < minDist){ minDist = dist; colorID = i; } // Se actualiza el color más cercano si la distancia es menor
			}
		}

		UART_TX_CADENA("\r\nColor detectado: "); // Se imprime encabezado de color detectado
		UART_TX_CADENA(colores[colorID].nombre); // Se imprime el nombre del color asociado a colorID
		UART_TX_CADENA("\r\nValores ADC -> "); // Se imprime encabezado de valores medidos
		UART_TX_CADENA("R: "); UART_IMPRIMIR_UINT(dR); // Se imprime valor dR medido
		UART_TX_CADENA(" | G: "); UART_IMPRIMIR_UINT(dG); // Se imprime valor dG medido
		UART_TX_CADENA(" | B: "); UART_IMPRIMIR_UINT(dB); // Se imprime valor dB medido
		UART_TX_CADENA("\r\nValores de referencia -> "); // Se imprime encabezado de rangos de referencia
		UART_TX_CADENA("R: ["); UART_IMPRIMIR_UINT(colores[colorID].minR); // Se imprime mínimo de R del color seleccionado
		UART_TX_CADENA(" ; "); UART_IMPRIMIR_UINT(colores[colorID].maxR); UART_TX(']'); // Se imprime máximo de R y se cierra corchete
		UART_TX_CADENA(" | G: ["); UART_IMPRIMIR_UINT(colores[colorID].minG); // Se imprime mínimo de G del color seleccionado
		UART_TX_CADENA(" ; "); UART_IMPRIMIR_UINT(colores[colorID].maxG); UART_TX(']'); // Se imprime máximo de G y se cierra corchete
		UART_TX_CADENA(" | B: ["); UART_IMPRIMIR_UINT(colores[colorID].minB); // Se imprime mínimo de B del color seleccionado
		UART_TX_CADENA(" ; "); UART_IMPRIMIR_UINT(colores[colorID].maxB); UART_TX(']'); // Se imprime máximo de B y se cierra corchete
		UART_TX_CADENA("\r\n---------------------------------------\r\n"); // Separador visual en la terminal

		SERVO_POS(colores[colorID].angulo); // Se posiciona el servo al ángulo asociado al color detectado
		_delay_ms(300); // Se espera 300 ms para permitir movimiento/estabilización mecánica

		switch(colorID){ // Se actualiza la indicación visual (WS2812 y LEDs discretos) según el color detectado
			case 0: setLedRGB(leds, 0, 0,255,0); show(leds); LED_G_ON(); break; // Si VERDE: se enciende verde en tira y LED G
			case 1: setLedRGB(leds, 0, 255,255,0); show(leds); LED_R_ON(); LED_G_ON(); break; // Si AMARILLO: mezcla R+G y LEDs R,G
			case 2: setLedRGB(leds, 0, 255,0,0); show(leds); LED_R_ON(); break; // Si ROJO: se enciende rojo en tira y LED R
			default: setLedRGB(leds, 0, 128,0,128); show(leds); LED_B_ON(); break; // Caso MORADO/u otro: púrpura en tira y LED B
		}

		_delay_ms(500); // Se agrega retardo de 500 ms antes del siguiente ciclo de detección
		LEDS_OFF(); // Se apagan LEDs discretos tras la indicación
	}
}
