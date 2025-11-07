#ifndef WS2812_H  // Se define una directiva de inclusión condicional para evitar múltiples inclusiones del archivo
#define WS2812_H  // Marca el inicio del bloque protegido de inclusión

#ifndef F_CPU  // Verifica si no está definida la frecuencia del microcontrolador
#define F_CPU 16000000UL  // Define la frecuencia del reloj principal en 16 MHz
#endif  // Fin de la comprobación de F_CPU

#include <avr/io.h>  // Se incluye la librería para acceder a los registros de E/S del microcontrolador AVR
#include <util/delay.h>  // Se incluye para generar retardos precisos mediante _delay_ms() o _delay_us()
#include <avr/interrupt.h>  // Se incluye para permitir el control de interrupciones con cli() y sei()
#include <stdlib.h>  // Se incluye para utilizar funciones como rand() para generar colores aleatorios
#include <stdint.h>  // Se incluye para manejar tipos de datos enteros con tamaño definido (uint8_t, etc.)

#define NUM_LEDS   64  // Define la cantidad total de LEDs en la matriz WS2812
#define LED_PIN    PB0  // Define el pin físico del puerto B que se utiliza para la señal de datos de los LEDs

void WS2812_INICIAR(void);  // Prototipo de función para inicializar el pin de control del WS2812
void WS2812_MOSTRAR(uint8_t (*colores)[3]);  // Prototipo de función para enviar los colores actuales a todos los LEDs
void WS2812_SETEAR_LED(uint8_t (*leds)[3], int indice, uint8_t r, uint8_t g, uint8_t b);  // Prototipo para asignar un color específico a un LED determinado
void WS2812_LIMPIAR(uint8_t (*leds)[3]);  // Prototipo para apagar todos los LEDs estableciendo sus valores RGB en 0
uint8_t WS2812_INDICE(uint8_t x, uint8_t y);  // Prototipo para calcular el índice lineal de un LED según sus coordenadas (x, y)
void WS2812_COLOR_ALEATORIO(uint8_t *r, uint8_t *g, uint8_t *b);  // Prototipo para generar un color aleatorio en formato RGB

#endif  // Fin de la protección contra inclusiones múltiples del archivo
