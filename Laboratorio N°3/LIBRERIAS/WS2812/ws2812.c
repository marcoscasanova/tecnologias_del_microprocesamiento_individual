#include "ws2812.h"  // Se incluye el archivo de cabecera con las definiciones y prototipos del control de los LEDs WS2812

static inline void WS2812_enviarBit(uint8_t bitVal) {  // Envía un único bit al LED respetando los tiempos de la señal WS2812
    if (bitVal) {  // Si el bit a enviar es 1
        PORTB |= (1 << LED_PIN);  // Se pone el pin en alto
        __asm__ __volatile__(  // Retardo preciso para mantener el tiempo alto correspondiente a un '1'
            "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t");
        PORTB &= ~(1 << LED_PIN);  // Se baja el pin
        __asm__ __volatile__(  // Retardo para completar el ciclo total del bit
            "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t");
    } else {  // Si el bit a enviar es 0
        PORTB |= (1 << LED_PIN);  // Se pone el pin en alto
        __asm__ __volatile__("nop\n\tnop\n\tnop\n\tnop\n\t");  // Retardo más corto correspondiente al '0'
        PORTB &= ~(1 << LED_PIN);  // Se baja el pin
        __asm__ __volatile__(  // Retardo más largo para completar el tiempo total del bit '0'
            "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n"
            "nop\n\tnop\n\tnop\n\tnop\n\t");
    }
}

static inline void WS2812_enviarByte(uint8_t byte) {  // Envía un byte completo (8 bits) al LED
    for (uint8_t i = 0; i < 8; i++)  // Recorre cada bit del byte
        WS2812_enviarBit(byte & (1 << (7 - i)));  // Envía los bits de más significativo a menos significativo
}

void WS2812_MOSTRAR(uint8_t (*colores)[3]) {  // Envía la información de color de todos los LEDs a la tira
    cli();  // Deshabilita interrupciones para asegurar precisión en los tiempos
    for (int i = 0; i < NUM_LEDS; i++) {  // Recorre todos los LEDs definidos
        WS2812_enviarByte(colores[i][0]);  // Envía componente verde (orden GRB)
        WS2812_enviarByte(colores[i][1]);  // Envía componente roja
        WS2812_enviarByte(colores[i][2]);  // Envía componente azul
    }
    sei();  // Habilita nuevamente las interrupciones
    _delay_us(80);  // Retardo de 80 µs para indicar fin de transmisión
}

void WS2812_SETEAR_LED(uint8_t (*leds)[3], int indice, uint8_t r, uint8_t g, uint8_t b) {  // Configura el color de un LED específico
    if (indice < 0 || indice >= NUM_LEDS) return;  // Verifica que el índice esté dentro del rango válido
    leds[indice][0] = g;  // Asigna el valor de la componente verde
    leds[indice][1] = r;  // Asigna el valor de la componente roja
    leds[indice][2] = b;  // Asigna el valor de la componente azul
}

void WS2812_INICIAR(void) {  // Inicializa el pin de datos para controlar los LEDs WS2812
    DDRB  |= (1 << LED_PIN);  // Configura el pin de datos como salida
    PORTB &= ~(1 << LED_PIN);  // Asegura que el pin inicie en estado bajo
    _delay_ms(1);  // Pequeña pausa para estabilización
}

void WS2812_LIMPIAR(uint8_t (*leds)[3]) {  // Apaga todos los LEDs estableciendo sus valores en 0
    for (uint8_t i = 0; i < NUM_LEDS; i++)  // Recorre todos los LEDs de la matriz
        leds[i][0] = leds[i][1] = leds[i][2] = 0;  // Asigna 0 a los tres componentes (G, R, B)
}

uint8_t WS2812_INDICE(uint8_t x, uint8_t y) {  // Calcula el índice lineal de un LED a partir de sus coordenadas (x, y)
    return y * 8 + x;  // Devuelve el índice correspondiente en una matriz de 8 columnas
}

void WS2812_COLOR_ALEATORIO(uint8_t *r, uint8_t *g, uint8_t *b) {  // Genera un color aleatorio asignando valores RGB entre 0 y 255
    *r = rand() % 256;  // Genera componente roja aleatoria
    *g = rand() % 256;  // Genera componente verde aleatoria
    *b = rand() % 256;  // Genera componente azul aleatoria
}
