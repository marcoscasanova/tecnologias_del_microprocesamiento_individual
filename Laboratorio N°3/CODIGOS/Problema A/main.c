#define F_CPU 16000000UL // Se define la frecuencia del CPU a 16 MHz
#include <avr/io.h> // Se incluye la librería de entrada/salida del microcontrolador AVR
#include <avr/interrupt.h> // Se incluye la librería para el manejo de interrupciones
#include <util/delay.h> // Se incluye la librería para generar retardos
#include <stdbool.h> // Se incluye la librería para el manejo del tipo de dato booleano
#include <avr/pgmspace.h> // Se incluye la librería para almacenar datos en la memoria de programa (flash)
#include <math.h> // Se incluye la librería matemática para realizar cálculos con funciones trigonométricas

#define CLK_X PB3 // Se define el pin PB3 como señal de reloj (CLK) para el eje X
#define DIR_X PB4 // Se define el pin PB4 como señal de dirección (DIR) para el eje X
#define EN_X  PB5 // Se define el pin PB5 como señal de habilitación (EN) para el eje X
#define SOLENOID PC0 // Se define el pin PC0 como salida para controlar el solenoide
#define CLK_Y PC3 // Se define el pin PC3 como señal de reloj (CLK) para el eje Y
#define DIR_Y PC4 // Se define el pin PC4 como señal de dirección (DIR) para el eje Y
#define EN_Y  PC5 // Se define el pin PC5 como señal de habilitación (EN) para el eje Y
#define LIMIT_YA PD2 // Se define el pin PD2 como entrada del final de carrera superior del eje Y
#define LIMIT_YD PD3 // Se define el pin PD3 como entrada del final de carrera inferior del eje Y
#define LED PD5 // Se define el pin PD5 como salida para el LED indicador

#define FRECUENCIA 5000UL // Se define la frecuencia de pasos del motor en 5000 Hz
#define PI 3.14159265 // Se define el valor de PI para usarlo en la función de CIRCULO();

volatile uint16_t contador_X = 0; // Se declara la variable contador_X como volátil para contar los pasos del eje X
volatile uint16_t limite_X = 0; // Se declara la variable limite_X como volátil para definir el límite de pasos en X
volatile uint16_t contador_Y = 0; // Se declara la variable contador_Y como volátil para contar los pasos del eje Y
volatile uint16_t limite_Y = 0; // Se declara la variable limite_Y como volátil para definir el límite de pasos en Y

const uint16_t OCR = (uint16_t)((F_CPU / (8UL * 2UL * FRECUENCIA)) - 1); // Se calcula el valor del registro OCR para obtener la frecuencia deseada en el Timer1

// Función para inicializar el Timer1 en modo CTC
void TIMER1_INICIAR(void){
	cli(); // Se deshabilitan las interrupciones globales
	TCCR1A = 0; // Se limpia el registro TCCR1A
	TCCR1B = 0; // Se limpia el registro TCCR1B
	TCNT1 = 0; // Se inicializa el contador del Timer1 en 0
	OCR1A = OCR; // Se carga el valor de comparación en OCR1A
	OCR1B = OCR; // Se carga el mismo valor de comparación en OCR1B
	TIFR1 |= (1<<OCF1A)|(1<<OCF1B); // Se limpian las banderas de interrupción de comparación A y B
	TCCR1B = (1<<WGM12) | (1<<CS11); // Se configura el Timer1 en modo CTC con prescaler 8
	sei(); // Se habilitan las interrupciones globales
}

// Función para generar una cantidad determinada de pasos en el eje X
void PASOS_X(uint16_t pasos){
	contador_X = 0; // Se reinicia el contador de pasos en X
	limite_X = pasos; // Se establece el número de pasos a realizar en X
	DDRB |= (1 << CLK_X); // Se configura el pin CLK_X como salida
	PORTB &= ~(1 << CLK_X); // Se inicia el pin CLK_X en bajo
	TIMSK1 |= (1 << OCIE1B); // Se habilita la interrupción de comparación B del Timer1
}

// Función para generar una cantidad determinada de pasos en el eje Y
void PASOS_Y(uint16_t pasos){
	contador_Y = 0; // Se reinicia el contador de pasos en Y
	limite_Y = pasos; // Se establece el número de pasos a realizar en Y
	DDRC |= (1 << CLK_Y); // Se configura el pin CLK_Y como salida
	PORTC &= ~(1 << CLK_Y); // Se inicia el pin CLK_Y en bajo
	TIMSK1 |= (1 << OCIE1A); // Se habilita la interrupción de comparación A del Timer1
}

// Función para detener el movimiento del eje X
void DETENER_X(void){
	TIMSK1 &= ~(1 << OCIE1B); // Se deshabilita la interrupción de comparación B
	PORTB &= ~(1 << CLK_X); // Se pone en bajo el pin CLK_X para detener los pulsos
}

// Función para detener el movimiento del eje Y
void DETENER_Y(void){
	TIMSK1 &= ~(1 << OCIE1A); // Se deshabilita la interrupción de comparación A
	PORTC &= ~(1 << CLK_Y); // Se pone en bajo el pin CLK_Y para detener los pulsos
}

// Interrupción del Timer1 para generar los pasos en el eje Y
ISR(TIMER1_COMPA_vect){
	PORTC ^= (1 << CLK_Y); // Se conmuta el pin CLK_Y para generar el pulso del motor Y
	if(++contador_Y >= limite_Y){ // Se incrementa el contador y se compara con el límite establecido
		DETENER_Y(); // Si se alcanza el límite, se detiene el eje Y
	}
}

// Interrupción del Timer1 para generar los pasos en el eje X
ISR(TIMER1_COMPB_vect){
	PORTB ^= (1 << CLK_X); // Se conmuta el pin CLK_X para generar el pulso del motor X
	if(++contador_X >= limite_X){ // Se incrementa el contador y se compara con el límite establecido
		DETENER_X(); // Si se alcanza el límite, se detiene el eje X
	}
}

// Función para bajar el solenoide
void PLOTTER_BAJAR(){
	PORTC &= ~(1 << SOLENOID); // Se desactiva el pin del solenoide para bajarlo
	_delay_ms(30); // Se espera 30 ms para asegurar el movimiento
}

// Función para subir el solenoide
void PLOTTER_SUBIR(){
	PORTC |= (1 << SOLENOID); // Se activa el pin del solenoide para subirlo
}

// Función para mover el plotter hacia la derecha
void PLOTTER_DERECHA(uint16_t pasos){
	PORTB |= (1 << DIR_X); // Se establece la dirección del eje X hacia la derecha
	PORTB |=  (1 << EN_X); // Se habilita el motor del eje X
	PASOS_X(pasos); // Se generan los pasos en el eje X
	while(TIMSK1 & (1<<OCIE1B)); // Se espera a que termine el movimiento
}

// Función para mover el plotter hacia la izquierda
void PLOTTER_IZQUIERDA(uint16_t pasos){
	PORTB &= ~(1 << DIR_X); // Se establece la dirección del eje X hacia la izquierda
	PORTB |=  (1 << EN_X); // Se habilita el motor del eje X
	PASOS_X(pasos); // Se generan los pasos en el eje X
	while(TIMSK1 & (1<<OCIE1B)); // Se espera a que termine el movimiento
}

// Función para mover el plotter hacia abajo
void PLOTTER_ABAJO(uint16_t pasos){
	PORTC |=  (1 << DIR_Y); // Se establece la dirección del eje Y hacia abajo
	PORTC |=  (1 << EN_Y); // Se habilita el motor del eje Y
	PASOS_Y(pasos); // Se generan los pasos en el eje Y
	while(TIMSK1 & (1<<OCIE1A)); // Se espera a que termine el movimiento
}

// Función para mover el plotter hacia arriba
void PLOTTER_ARRIBA(uint16_t pasos){
	PORTC &= ~(1 << DIR_Y); // Se establece la dirección del eje Y hacia arriba
	PORTC |=  (1 << EN_Y); // Se habilita el motor del eje Y
	PASOS_Y(pasos); // Se generan los pasos en el eje Y
	while(TIMSK1 & (1<<OCIE1A)); // Se espera a que termine el movimiento
}

// Función para mover el plotter hacia la derecha sin bajar el solenoide
void PLOTTER_DERECHA_NO_BAJAR(uint16_t pasos){
	PLOTTER_SUBIR(); // Se asegura que el solenoide esté levantado
	PORTB |= (1 << DIR_X); // Se establece la dirección hacia la derecha
	PORTB |=  (1 << EN_X); // Se habilita el motor del eje X
	PASOS_X(pasos); // Se generan los pasos correspondientes
	while(TIMSK1 & (1<<OCIE1B)); // Se espera hasta finalizar el movimiento
}

// Función para mover el plotter hacia la izquierda sin bajar el solenoide
void PLOTTER_IZQUIERDA_NO_BAJAR(uint16_t pasos){
	PLOTTER_SUBIR(); // Se asegura que el solenoide esté levantado
	PORTB &= ~(1 << DIR_X); // Se establece la dirección hacia la izquierda
	PORTB |=  (1 << EN_X); // Se habilita el motor del eje X
	PASOS_X(pasos); // Se generan los pasos correspondientes
	while(TIMSK1 & (1<<OCIE1B)); // Se espera hasta finalizar el movimiento
}

// Función para mover el plotter hacia abajo sin bajar el solenoide
void PLOTTER_ABAJO_NO_BAJAR(uint16_t pasos){
	PLOTTER_SUBIR(); // Se asegura que el solenoide esté levantado
	PORTC |=  (1 << DIR_Y); // Se establece la dirección hacia abajo
	PORTC |=  (1 << EN_Y); // Se habilita el motor del eje Y
	PASOS_Y(pasos); // Se generan los pasos correspondientes
	while(TIMSK1 & (1<<OCIE1A)); // Se espera hasta finalizar el movimiento
}

// Función para mover el plotter hacia arriba sin bajar el solenoide
void PLOTTER_ARRIBA_NO_BAJAR(uint16_t pasos){
	PLOTTER_SUBIR(); // Se asegura que el solenoide esté levantado
	PORTC &= ~(1 << DIR_Y); // Se establece la dirección hacia arriba
	PORTC |=  (1 << EN_Y); // Se habilita el motor del eje Y
	PASOS_Y(pasos); // Se generan los pasos correspondientes
	while(TIMSK1 & (1<<OCIE1A)); // Se espera hasta finalizar el movimiento
}

// Función para mover el plotter en diagonal hacia arriba y a la derecha
void PLOTTER_ARRIBA_DERECHA(uint16_t pasos){
	PORTB |= (1 << DIR_X); // Se establece la dirección X hacia la derecha
	PORTC &= ~(1 << DIR_Y); // Se establece la dirección Y hacia arriba
	PORTB |=  (1 << EN_X); // Se habilita el motor del eje X
	PORTC |=  (1 << EN_Y); // Se habilita el motor del eje Y
	PASOS_X(pasos); // Se generan pasos simultáneos en el eje X
	PASOS_Y(pasos); // Se generan pasos simultáneos en el eje Y
	while((TIMSK1 & ((1<<OCIE1A)|(1<<OCIE1B)))); // Se espera a que ambos movimientos finalicen
}

// Función para mover el plotter en diagonal hacia arriba y a la izquierda
void PLOTTER_ARRIBA_IZQUIERDA(uint16_t pasos){
	PORTB &= ~(1 << DIR_X); // Se establece la dirección X hacia la izquierda
	PORTC &= ~(1 << DIR_Y); // Se establece la dirección Y hacia arriba
	PORTB |=  (1 << EN_X); // Se habilita el motor del eje X
	PORTC |=  (1 << EN_Y); // Se habilita el motor del eje Y
	PASOS_X(pasos); // Se generan pasos simultáneos en el eje X
	PASOS_Y(pasos); // Se generan pasos simultáneos en el eje Y
	while((TIMSK1 & ((1<<OCIE1A)|(1<<OCIE1B)))); // Se espera a que ambos movimientos finalicen
}

// Función para mover el plotter en diagonal hacia abajo y a la derecha
void PLOTTER_ABAJO_DERECHA(uint16_t pasos){
	PORTB |= (1 << DIR_X); // Se establece la dirección X hacia la derecha
	PORTC |=  (1 << DIR_Y); // Se establece la dirección Y hacia abajo
	PORTB |=  (1 << EN_X); // Se habilita el motor del eje X
	PORTC |=  (1 << EN_Y); // Se habilita el motor del eje Y
	PASOS_X(pasos); // Se generan pasos simultáneos en el eje X
	PASOS_Y(pasos); // Se generan pasos simultáneos en el eje Y
	while((TIMSK1 & ((1<<OCIE1A)|(1<<OCIE1B)))); // Se espera a que ambos movimientos finalicen
}

// Función para mover el plotter en diagonal hacia abajo y a la izquierda
void PLOTTER_ABAJO_IZQUIERDA(uint16_t pasos){
	PORTB &= ~(1 << DIR_X); // Se establece la dirección X hacia la izquierda
	PORTC |=  (1 << DIR_Y); // Se establece la dirección Y hacia abajo
	PORTB |=  (1 << EN_X); // Se habilita el motor del eje X
	PORTC |=  (1 << EN_Y); // Se habilita el motor del eje Y
	PASOS_X(pasos); // Se generan pasos simultáneos en el eje X
	PASOS_Y(pasos); // Se generan pasos simultáneos en el eje Y
	while((TIMSK1 & ((1<<OCIE1A)|(1<<OCIE1B)))); // Se espera a que ambos movimientos finalicen
}

void TRIANGULO(void){ // Función para dibujar un triángulo
	PLOTTER_BAJAR(); // Se baja la solenoide para comenzar a dibujar
	PLOTTER_DERECHA(3000); // Se mueve el plotter hacia la derecha una distancia de 3000 pasos
	PLOTTER_ABAJO_IZQUIERDA(1500); // Se mueve el plotter en diagonal hacia abajo e izquierda una distancia de 1500 pasos
	PLOTTER_ARRIBA_IZQUIERDA(1500); // Se mueve el plotter en diagonal hacia arriba e izquierda una distancia de 1500 pasos
}

void CRUZ(void){ // Función para dibujar una cruz
	PLOTTER_BAJAR(); // Se baja la solenoide para comenzar a dibujar
	PLOTTER_ABAJO_DERECHA(2000); // Se mueve el plotter en diagonal hacia abajo y a la derecha una distancia de 2000 pasos
	PLOTTER_IZQUIERDA_NO_BAJAR(2000); // Se mueve el plotter hacia la izquierda sin bajar la solenoide una distancia de 2000 pasos
	PLOTTER_BAJAR(); // Se baja nuevamente la solenoide para continuar el dibujo
	PLOTTER_ARRIBA_DERECHA(2000); // Se mueve el plotter en diagonal hacia arriba y a la derecha una distancia de 2000 pasos
}

void CIRCULO(uint16_t radio, float factor_y){ // Función para dibujar un círculo elíptico según el radio y un factor y, ya que el motor PaP del eje Y va mas rapido que el del eje X
	float x_anterior = radio * cos(0); // Se calcula la coordenada X inicial usando coseno
	float y_anterior = radio * sin(0) * factor_y; // Se calcula la coordenada Y inicial aplicando el factor de escala

	PLOTTER_DERECHA_NO_BAJAR((uint16_t)x_anterior); // Se mueve el plotter a la posición inicial en X sin bajar la solenoide
	PLOTTER_ABAJO_NO_BAJAR((uint16_t)(-y_anterior)); // Se mueve el plotter a la posición inicial en Y sin bajar la solenoide

	PLOTTER_BAJAR(); // Se baja la solenoide para comenzar a dibujar el círculo

	for (float angulo = 1.0; angulo <= 360.0; angulo += 1.0){ // Se recorre el círculo completo en incrementos de 1 grado
		float rad = angulo * (PI / 180.0); // Se convierte el ángulo a radianes
		float x = radio * cos(rad); // Se calcula la nueva coordenada X
		float y = radio * sin(rad) * factor_y; // Se calcula la nueva coordenada Y aplicando el factor de escala

		int16_t dx = (int16_t)roundf(x - x_anterior); // Se calcula el desplazamiento en X respecto al punto anterior
		int16_t dy = (int16_t)roundf(y - y_anterior); // Se calcula el desplazamiento en Y respecto al punto anterior

		if (dx > 0) PLOTTER_DERECHA(dx); // Si el desplazamiento es positivo, se mueve hacia la derecha
		else if (dx < 0) PLOTTER_IZQUIERDA(-dx); // Si es negativo, se mueve hacia la izquierda

		if (dy > 0) PLOTTER_ARRIBA(dy); // Si el desplazamiento en Y es positivo, se mueve hacia arriba
		else if (dy < 0) PLOTTER_ABAJO(-dy); // Si es negativo, se mueve hacia abajo

		x_anterior = x; // Se actualiza la coordenada anterior en X
		y_anterior = y; // Se actualiza la coordenada anterior en Y
	}

	PLOTTER_SUBIR(); // Se levanta la solenoide al finalizar el dibujo del círculo
}

typedef struct { // Se define una estructura para representar un paso de dibujo
	char dir; // Se almacena la dirección del movimiento ('D', 'I', 'A', 'U', etc.)
	uint16_t t; // Se almacena la cantidad de pasos o tiempo asociado al movimiento
} Paso;

#define ESCALA 0.3 // Se define un factor de escala para ajustar la magnitud de los movimientos adaptados del problema del plotter anterior

void EJECUTAR_FIGURA(const Paso *figura, uint16_t n_pasos){ // Función para ejecutar una figura compuesta por varios pasos
	for (uint16_t i = 0; i < n_pasos; i++){ // Se recorre la cantidad total de pasos definidos
		char dir = pgm_read_byte(&figura[i].dir); // Se lee la dirección del paso desde la memoria de programa
		uint16_t pasos_original = pgm_read_word(&figura[i].t); // Se lee la cantidad original de pasos desde la memoria de programa
		uint16_t pasos = (uint16_t)(pasos_original * ESCALA); // Se aplica la escala definida a la cantidad de pasos

		switch(dir){ // Se evalúa la dirección del paso para ejecutar el movimiento correspondiente
			case 'D': PLOTTER_DERECHA(pasos); break; // Se mueve el plotter hacia la derecha
			case 'I': PLOTTER_IZQUIERDA(pasos); break; // Se mueve el plotter hacia la izquierda
			case 'A': PLOTTER_ABAJO(pasos); break; // Se mueve el plotter hacia abajo
			case 'U': PLOTTER_ARRIBA(pasos); break; // Se mueve el plotter hacia arriba
			case 'B': PLOTTER_BAJAR(); break; // Se baja la solenoide
			case 'S': PLOTTER_SUBIR(); break; // Se levanta la solenoide
			case 'd': PLOTTER_DERECHA_NO_BAJAR(pasos); break; // Se mueve hacia la derecha sin bajar la solenoide
			case 'i': PLOTTER_IZQUIERDA_NO_BAJAR(pasos); break; // Se mueve hacia la izquierda sin bajar la solenoide
			case 'a': PLOTTER_ABAJO_NO_BAJAR(pasos); break; // Se mueve hacia abajo sin bajar la solenoide
			case 'u': PLOTTER_ARRIBA_NO_BAJAR(pasos); break; // Se mueve hacia arriba sin bajar la solenoide
			default: break; // Si la dirección no coincide con ninguna opción válida, no se realiza acción
		}
	}
}

// Definición de la figura zorro a través de una estructura de pasos
const Paso ZORRO[] PROGMEM = {
	{'i', 3000}, {'a', 3000}, {'B'}, {'D', 3900}, {'U', 100}, {'D', 200}, {'U', 100}, {'D', 100}, {'U', 100}, {'D', 200}, {'U', 100},
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
	{'I', 100}, {'U', 400}, {'S', 250}, {'d', 1800}, {'a', 3400}, {'B'}, {'A', 4300}, {'S', 250}, {'u', 4300}, {'B', 250}, {'D', 200},
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

// Definición de la figura flor a través de una estructura de pasos
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
	{'U', 100}, {'D', 300}, {'U', 100}, {'D', 400}, {'S', 250}, {'a', 5400}, {'i', 200}, {'B', 250}, {'A', 5400}, {'D', 500},
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

void FIGURAS(void){ // Función para ejecutar una secuencia completa de figuras
	TRIANGULO(); // Se dibuja un triángulo
	PLOTTER_DERECHA_NO_BAJAR(4000); // Se mueve el plotter hacia la derecha sin bajar la solenoide una distancia de 4000 pasos
	CRUZ(); // Se dibuja una cruz
	PLOTTER_DERECHA_NO_BAJAR(2000); // Se mueve el plotter hacia la derecha sin bajar la solenoide una distancia de 2000 pasos
	PLOTTER_ABAJO_NO_BAJAR(1000); // Se mueve el plotter hacia abajo sin bajar la solenoide una distancia de 1000 pasos
	CIRCULO(1000, 0.91); // Se dibuja un círculo con radio de 1000 pasos y un factor vertical de 0.91
	PLOTTER_ABAJO_NO_BAJAR(1000); // Se mueve el plotter hacia abajo sin bajar la solenoide una distancia de 1000 pasos
	EJECUTAR_FIGURA(ZORRO, sizeof(ZORRO) / sizeof(ZORRO[0])); // Se ejecuta la figura “Zorro” definida en memoria de programa
	PLOTTER_IZQUIERDA_NO_BAJAR(3000); // Se mueve el plotter hacia la izquierda sin bajar la solenoide una distancia de 3000 pasos
	PLOTTER_ARRIBA_NO_BAJAR(1500); // Se mueve el plotter hacia arriba sin bajar la solenoide una distancia de 1500 pasos
	EJECUTAR_FIGURA(FLOR, sizeof(FLOR) / sizeof(FLOR[0])); // Se ejecuta la figura “Flor” definida en memoria de programa
}

int main(void){ // Función principal del programa
	DDRB |= (1<<DIR_X) | (1<<EN_X); // Se configuran los pines DIR_X y EN_X del puerto B como salidas
	DDRC |= (1<<DIR_Y) | (1<<EN_Y) | (1<<SOLENOID); // Se configuran los pines DIR_Y, EN_Y y SOLENOID del puerto C como salidas

	TIMER1_INICIAR(); // Se inicializa el Timer1 para el control de los motores paso a paso

	FIGURAS(); // Se ejecuta la secuencia completa de figuras predefinidas

	while(1); // Bucle infinito para mantener el programa en ejecución
}

// No se realizó implementacion del LED ni de los limites, ya que se mantuvo el trazado acorde a la hoja, que es de menor proporción que el área de trazo de plotter.
