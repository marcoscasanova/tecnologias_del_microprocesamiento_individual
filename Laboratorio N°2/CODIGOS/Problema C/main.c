#define F_CPU 16000000UL // Se define la frecuencia de reloj del microcontrolador en 16 MHz
#include <avr/io.h> // Librería de entrada/salida del microcontrolador AVR
#include <util/delay.h> // Librería para la generación de retardos temporales
#include <stdint.h> // Librería para usar tipos de datos de tamaño fijo (uint8_t, uint16_t, etc.)
#include <stdbool.h> // Librería para manejar valores booleanos (true/false)

// Función para inicializar la comunicación UART con un baudrate dado
static void UART_INICIALIZAR(uint32_t baud){
	uint16_t ubrr = (uint16_t)((F_CPU / (16UL*baud)) - 1UL); // Se calcula el valor del registro UBRR según la velocidad deseada
	UBRR0H = (uint8_t)(ubrr >> 8); // Se carga la parte alta del valor de UBRR
	UBRR0L = (uint8_t)(ubrr & 0xFF); // Se carga la parte baja del valor de UBRR
	UCSR0A = 0; // Se limpia el registro de estado A
	UCSR0B = (1<<RXEN0) | (1<<TXEN0); // Se habilitan los módulos de recepción y transmisión UART
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); // Se configura el formato: 8 bits de datos, 1 bit de stop, sin paridad
}

// Funciones básicas de UART
static inline uint8_t UART_DISPONIBLE(void){ return (UCSR0A & (1<<RXC0)); } // Devuelve 1 si hay datos disponibles para leer
static inline uint8_t UART_LEER(void){ return UDR0; } // Lee y retorna el dato recibido
static void UART_ESCRIBIR(uint8_t c){ while(!(UCSR0A & (1<<UDRE0))); UDR0 = c; } // Envía un byte, esperando que el buffer esté libre
static void UART_IMPRIMIR(const char* s){ while(*s){ UART_ESCRIBIR((uint8_t)*s++); } } // Envía una cadena de caracteres por UART
static void UART_IMPRIMIRLN(const char* s){ UART_IMPRIMIR(s); UART_ESCRIBIR('\r'); UART_ESCRIBIR('\n'); } // Envía una cadena y luego salto de línea

// Funciones para control PWM del Timer1 (modo Fast PWM, canal OC1A)
static inline void PWM_INICIALIZAR(void){
	DDRB  |= (1<<PB1); // Se configura el pin PB1 (OC1A) como salida
	TCCR1A = (1<<WGM11); // Se selecciona el modo de generación PWM (parte baja de configuración)
	TCCR1B = (1<<WGM13) | (1<<WGM12); // Se completa configuración de Fast PWM con ICR1 como TOP
	TCCR1A &= ~((1<<COM1A1)|(1<<COM1A0)); // Se desconecta la salida OC1A del pin físico
	TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10)); // Se detiene el temporizador (sin prescaler activo)
	ICR1 = 0; OCR1A = 0; // Se inicializan los registros de comparación y tope en 0
	PORTB &= ~(1<<PB1); // Se coloca el pin PB1 en bajo
}

// Función para apagar el PWM
static inline void PWM_APAGAR(void){
	TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10)); // Se detiene el temporizador
	TCCR1A &= ~((1<<COM1A1)|(1<<COM1A0)); // Se desconecta la salida del pin
	OCR1A = 0; // Se pone el ciclo de trabajo en 0
	PORTB &= ~(1<<PB1); // Se fuerza el pin PB1 a nivel bajo
}

// Función para configurar la frecuencia de PWM
static inline void PWM_FRECUENCIA(uint16_t f_hz){
	if(!f_hz){ PWM_APAGAR(); return; } // Si la frecuencia es cero, se apaga el PWM
	uint32_t tope = (F_CPU/(8UL*(uint32_t)f_hz)) - 1UL; // Se calcula el valor TOP para la frecuencia deseada
	if(tope > 65535UL) tope = 65535UL; // Se limita el valor máximo permitido por el registro de 16 bits
	ICR1  = (uint16_t)tope; // Se establece el tope (periodo)
	OCR1A = (uint16_t)(tope/2); // Se ajusta el duty cycle al 50%
	TCCR1A |=  (1<<COM1A1); // Se conecta la salida OC1A en modo no inversor
	TCCR1A &= ~(1<<COM1A0); // Se asegura el modo correcto
	TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10)); // Se limpia el prescaler
	TCCR1B |=  (1<<CS11); // Se configura prescaler en 8 e inicia el temporizador
}

// Definición de frecuencias para notas musicales (en Hz)
#define NOTA_C4 262
#define NOTA_D4 294
#define NOTA_E4 330
#define NOTA_F4 349
#define NOTA_G4 392
#define NOTA_A4 440
#define NOTA_B4 494
#define NOTA_C5 523

// Notas adicionales
#define SILENCIO 0
#define A3 220
#define AS3 233
#define B3 247

#define C4 262
#define CS4 277
#define D4 294
#define DS4 311
#define E4 330
#define F4 349
#define FS4 370
#define G4 392
#define GS4 415
#define A4 440
#define AS4 466
#define B4 494

#define C5 523
#define CS5 554
#define D5 587
#define DS5 622
#define E5 659
#define F5 698
#define FS5 740
#define G5 784
#define GS5 831
#define A5 880
#define AS5 932
#define B5 988

#define C6 1046
#define D6 1175
#define E6 1319
#define F6 1397
#define G6 1568
#define A6 1760
#define AS6 1865
#define B6 1976

#define C7 2093
#define D7 2349
#define E7 2637
#define F7 2794
#define G7 3136
#define A7 3520
#define B7 3951

// Configuración de tiempos para melodías
#define MARIO_BPM 200 // Velocidad de la melodía de Mario en beats por minuto
#define MAR_Q ((uint16_t)(60000UL/MARIO_BPM)) // Duración de una negra
#define MAR_E ((uint16_t)(30000UL/MARIO_BPM)) // Duración de una corchea
#define MAR_S ((uint16_t)(15000UL/MARIO_BPM)) // Duración de una semicorchea
#define MAR_ED ((uint16_t)(45000UL/MARIO_BPM)) // Corchea con puntillo
#define MAR_QD ((uint16_t)(90000UL/MARIO_BPM)) // Negra con puntillo

#define TETRIS_BPM 144 // Velocidad de la melodía de Tetris
#define TET_Q  ((uint16_t)(60000UL/TETRIS_BPM)) // Duración de una negra
#define TET_E  ((uint16_t)(30000UL/TETRIS_BPM)) // Duración de una corchea
#define TET_S  ((uint16_t)(15000UL/TETRIS_BPM)) // Duración de una semicorchea
#define TET_QD ((uint16_t)(90000UL/TETRIS_BPM)) // Negra con puntillo
#define TET_H  ((uint16_t)(120000UL/TETRIS_BPM)) // Blanca

// Estructura para definir una nota musical con su frecuencia y duración
typedef struct { uint16_t f; uint16_t ms; } NOTA;

// Melodía de Super Mario
static const NOTA mario[] = {
	{E7,MAR_E},{E7,MAR_E},{SILENCIO,MAR_E},{E7,MAR_E},{SILENCIO,MAR_E},{C7,MAR_E},{E7,MAR_E},{SILENCIO,MAR_E},
	{G7,MAR_E},{SILENCIO,MAR_Q},{G6,MAR_E},{SILENCIO,MAR_Q},
	{C7,MAR_E},{SILENCIO,MAR_Q},{G6,MAR_E},{SILENCIO,MAR_Q},{E6,MAR_E},{SILENCIO,MAR_Q},
	{A6,MAR_E},{SILENCIO,MAR_E},{B6,MAR_E},{SILENCIO,MAR_E},{AS6,MAR_E},{A6,MAR_E},
	{G6,MAR_E},{E7,MAR_E},{G7,MAR_E},{A7,MAR_E},{SILENCIO,MAR_E},{F7,MAR_E},{G7,MAR_E},{SILENCIO,MAR_E},
	{E7,MAR_E},{SILENCIO,MAR_E},{C7,MAR_E},{D7,MAR_E},{B6,MAR_E},{SILENCIO,MAR_Q},
	{C7,MAR_E},{SILENCIO,MAR_Q},{G6,MAR_E},{SILENCIO,MAR_Q},{E6,MAR_E},{SILENCIO,MAR_Q},
	{A6,MAR_E},{SILENCIO,MAR_E},{B6,MAR_E},{SILENCIO,MAR_E},{AS6,MAR_E},{A6,MAR_E},
	{G6,MAR_E},{E7,MAR_E},{G7,MAR_E},{A7,MAR_E},{SILENCIO,MAR_Q}
};
static const uint16_t mario_long = sizeof(mario)/sizeof(mario[0]); // Largo del arreglo de notas de Mario

// Melodía de Tetris
static const NOTA tetris[] = {
	{E5,TET_E},{B4,TET_E},{C5,TET_E},{D5,TET_E},
	{C5,TET_E},{B4,TET_E},{A4,TET_Q},{A4,TET_E},
	{C5,TET_E},{E5,TET_E},{D5,TET_E},{C5,TET_E},
	{B4,TET_Q},{B4,TET_E},{SILENCIO,TET_E},
	{C5,TET_E},{D5,TET_E},{E5,TET_Q},{C5,TET_E},
	{A4,TET_E},{A4,TET_Q},{SILENCIO,TET_E},
	{D5,TET_E},{F5,TET_E},{A5,TET_Q},{G5,TET_E},
	{F5,TET_E},{E5,TET_Q},{C5,TET_E},{E5,TET_E},
	{D5,TET_Q},{C5,TET_E},{B4,TET_E},{B4,TET_Q},
	{C5,TET_E},{D5,TET_E},{E5,TET_Q},{C5,TET_E},
	{A4,TET_E},{A4,TET_Q},{SILENCIO,TET_E},
	{E5,TET_E},{C5,TET_E},{D5,TET_E},{B4,TET_Q},
	{C5,TET_E},{A4,TET_E},{G4,TET_Q},{SILENCIO,TET_E},
	{E5,TET_E},{C5,TET_E},{D5,TET_E},{B4,TET_Q},
	{C5,TET_E},{E5,TET_E},{A5,TET_Q},{G5,TET_E},
	{F5,TET_E},{D5,TET_Q},{F5,TET_E},{E5,TET_QD},
	{C5,TET_E},{D5,TET_E},{E5,TET_E},{C5,TET_E},
	{A4,TET_E},{A4,TET_Q},{SILENCIO,TET_E},
	{D5,TET_E},{F5,TET_E},{A5,TET_Q},{G5,TET_E},
	{F5,TET_E},{E5,TET_Q},{C5,TET_E},{E5,TET_E},
	{D5,TET_Q},{C5,TET_E},{B4,TET_E},{B4,TET_H},
	{SILENCIO,TET_Q}
};
static const uint16_t tetris_long = sizeof(tetris)/sizeof(tetris[0]); // Largo del arreglo de notas de Tetris

// Funciones para inicializar y leer teclado (piano)
static void TECLAS_INICIALIZAR(void){
	DDRC  &= ~((1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3)); // Se configuran PC0–PC3 como entradas
	PORTC |=  ((1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3)); // Se habilitan resistencias pull-up internas
	DDRD  &= ~((1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)); // Se configuran PD2–PD5 como entradas
	PORTD |=  ((1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)); // Se habilitan resistencias pull-up internas
}
static uint16_t LEER_NOTA(void){
	if(!(PINC & (1<<PC0))) return NOTA_C4; // Si se presiona PC0 → nota DO4
	if(!(PINC & (1<<PC1))) return NOTA_D4; // Si se presiona PC1 → nota RE4
	if(!(PINC & (1<<PC2))) return NOTA_E4; // Si se presiona PC2 → nota MI4
	if(!(PINC & (1<<PC3))) return NOTA_F4; // Si se presiona PC3 → nota FA4
	if(!(PIND & (1<<PD2))) return NOTA_G4; // Si se presiona PD2 → nota SOL4
	if(!(PIND & (1<<PD3))) return NOTA_A4; // Si se presiona PD3 → nota LA4
	if(!(PIND & (1<<PD4))) return NOTA_B4; // Si se presiona PD4 → nota SI4
	if(!(PIND & (1<<PD5))) return NOTA_C5; // Si se presiona PD5 → nota DO5
	return 0; // Si no se presiona ninguna tecla
}

// Definición de modos de operación
typedef enum { PIANO=0, CANCION_1=1, CANCION_2=2, PAUSA=3 } MODO; // Enumeración de modos de funcionamiento
static volatile MODO modo_actual = PIANO; // Variable global para el modo actual (inicialmente Piano)

// Declaración de la función de manejo de comandos UART
static void MANEJAR_UART(uint8_t b);

// Función que realiza retardos verificando comandos UART en tiempo real
static inline void RETARDO_COMPROBAR(uint16_t ms, uint8_t en_cancion){
	while(ms--){ // Se repite el retardo indicado
		if(UART_DISPONIBLE()) MANEJAR_UART(UART_LEER()); // Si llega un comando UART, se procesa
		if(en_cancion && !(modo_actual==CANCION_1 || modo_actual==CANCION_2)) return; // Si se sale del modo canción, se interrumpe
		if(modo_actual==PAUSA) PWM_APAGAR(); // Si se activa pausa, se apaga el sonido
		_delay_ms(1); // Retardo de 1 ms
	}
}

// Función para reproducir una nota con duración determinada
static inline void REPRODUCIR_NOTA(uint16_t f, uint16_t ms, uint8_t en_cancion){
	if(f==SILENCIO || ms==0){ PWM_APAGAR(); RETARDO_COMPROBAR(ms, en_cancion); return; } // Si hay silencio, no se reproduce
	if(en_cancion && !(modo_actual==CANCION_1 || modo_actual==CANCION_2)) return; // Si se cambió de modo, se sale
	PWM_FRECUENCIA(f); // Se establece la frecuencia de la nota
	if(ms > 6){
		RETARDO_COMPROBAR(ms - 4, en_cancion); // Se mantiene el sonido durante la mayor parte del tiempo
		PWM_APAGAR(); // Se apaga antes del final
		RETARDO_COMPROBAR(4, en_cancion); // Pequeño silencio entre notas
		}else{
		RETARDO_COMPROBAR(ms, en_cancion); // Si la nota es corta, se mantiene completa
		PWM_APAGAR(); // Se apaga el PWM
	}
}

// Función para reproducir una canción completa
static void REPRODUCIR_CANCION(const NOTA *cancion, uint16_t longitud, uint32_t tiempo_total){
	uint32_t transcurrido = 0; // Contador del tiempo transcurrido
	while(transcurrido < tiempo_total && (modo_actual==CANCION_1 || modo_actual==CANCION_2)){
		for(uint16_t i=0; i<longitud && transcurrido<tiempo_total; i++){
			if(!(modo_actual==CANCION_1 || modo_actual==CANCION_2)) break; // Si se cambia de modo, se detiene
			uint16_t d = cancion[i].ms; // Se toma la duración de la nota
			if(transcurrido + d > tiempo_total) d = (uint16_t)(tiempo_total - transcurrido); // Ajuste final
			REPRODUCIR_NOTA(cancion[i].f, d, 1); // Se reproduce la nota
			if(!(modo_actual==CANCION_1 || modo_actual==CANCION_2)) break;
			transcurrido += d; // Se acumula tiempo
		}
	}
	PWM_APAGAR(); // Se apaga el sonido al finalizar
}

// Función para manejar comandos recibidos por UART
static void MANEJAR_UART(uint8_t b){
	if(b=='H' || b=='h'){
		UART_IMPRIMIRLN("Comandos: P = Piano | 1 = Mario | 2 = Tetris | S = Pausa | H = Comandos"); // Ayuda de comandos
		}else if(b=='P' || b=='p'){
		modo_actual = PIANO; // Cambia a modo piano
		UART_IMPRIMIRLN("[Modo] Piano");
		}else if(b=='1'){
		modo_actual = CANCION_1; // Reproduce canción 1 (Mario)
		UART_IMPRIMIRLN("[Modo] Cancion 1 (Mario)");
		}else if(b=='2'){
		modo_actual = CANCION_2; // Reproduce canción 2 (Tetris)
		UART_IMPRIMIRLN("[Modo] Cancion 2 (Tetris)");
		}else if(b=='S' || b=='s'){
		modo_actual = PAUSA; // Pone el sistema en pausa
		PWM_APAGAR(); // Apaga el sonido
		UART_IMPRIMIRLN("[Modo] Pausa");
	}
}

// Función principal del programa
int main(void){
	UART_INICIALIZAR(9600); // Se inicializa UART a 9600 baudios
	PWM_INICIALIZAR(); // Se inicializa el módulo PWM
	TECLAS_INICIALIZAR(); // Se configuran las teclas como entradas

	modo_actual = PIANO; // Se establece el modo inicial como Piano
	UART_IMPRIMIRLN("Comandos: P = Piano | 1 = Mario | 2 = Tetris | S = Pausa | H = Comandos"); // Se muestran comandos disponibles
	UART_IMPRIMIRLN("Modo inicial: Piano"); // Se indica modo inicial

	while(1){ // Bucle principal
		if(UART_DISPONIBLE()) MANEJAR_UART(UART_LEER()); // Se verifica si hay comandos UART entrantes
		if(modo_actual == PIANO){
			uint16_t f = LEER_NOTA(); // Se lee la nota presionada
			if(f){
				_delay_ms(8); // Pequeño retardo para eliminar rebotes
				if(f == LEER_NOTA()){ // Se confirma que la tecla sigue presionada
					PWM_FRECUENCIA(f); // Se reproduce la nota
					while(LEER_NOTA()==f){ // Mientras se mantenga presionada
						if(UART_DISPONIBLE()){ // Si llega comando UART, se procesa
							MANEJAR_UART(UART_LEER());
							if(modo_actual != PIANO) break;
						}
						_delay_ms(2); // Pequeño retardo para evitar sobrecarga de CPU
					}
					PWM_APAGAR(); // Se apaga el sonido al soltar la tecla
				}
				}else{
				PWM_APAGAR(); // Si no hay tecla presionada, se apaga el PWM
			}
			}else if(modo_actual == CANCION_1){
			REPRODUCIR_CANCION(mario, mario_long, 15000UL); // Se reproduce la canción de Mario
			}else if(modo_actual == CANCION_2){
			REPRODUCIR_CANCION(tetris, tetris_long, 15000UL); // Se reproduce la canción de Tetris
			}else{
			PWM_APAGAR(); // Si el modo es pausa, se apaga el sonido
			_delay_ms(2); // Pequeño retardo para estabilidad
		}
	}
}
