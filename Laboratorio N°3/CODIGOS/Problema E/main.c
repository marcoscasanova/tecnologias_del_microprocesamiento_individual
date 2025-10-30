#define F_CPU 16000000UL // Se define la frecuencia del microcontrolador en 16 MHz

#include <avr/io.h> // Se incluye la librería AVR para el manejo de registros y pines del ATmega328P
#include <util/delay.h> // Se incluye la librería util/delay para la generación de retardos en milisegundos y microsegundos
#include <string.h> // Se incluye la librería string para el manejo de operaciones con memoria (memset, memcmp)
#include <stdbool.h> // Se incluye la librería stdbool para el manejo de variables booleanas (true / false)
#include <stdint.h> // Se incluye la librería stdint para el uso de tipos de datos con tamaño fijo (uint8_t, int16_t, etc.)

#include "uart.h" // Se incluye la librería uart creada personalmente para el manejo del puerto serial (UART)
#include "i2c.h" // Se incluye la librería i2c creada personalmente para la comunicación I2C utilizada por la LCD
#include "eeprom.h" // Se incluye la librería eeprom creada personalmente para el manejo de la memoria EEPROM interna del microcontrolador
#include "spi.h" // Se incluye la librería spi creada personalmente para la comunicación SPI con el módulo RFID
#include "rc522.h" // Se incluye la librería rc522 creada personalmente para el manejo del lector RFID MFRC522 mediante SPI
#include "lcd.h" // Se incluye la librería lcd creada personalmente para el manejo del display LCD (que utiliza internamente la librería i2c)

// Se crea la estructura enumerada modo_t con los estados de funcionamiento del sistema
typedef enum{
	DETECCION = 0, // Modo normal de lectura y verificación de tarjetas
	REGISTRO, // Modo de registro de una nueva tarjeta RFID
	BORRADO // Modo de eliminación de la tarjeta guardada en EEPROM
} modo_t;

void CONFIGURACION(void); // Inicializa todos los periféricos del sistema: UART, I2C, SPI, LCD, RFID, pines de LEDs, botones y buzzer
void REGISTRAR_TARJETA(const uint8_t *id, uint8_t len); // Guarda en la memoria EEPROM el ID de una nueva tarjeta RFID registrada
void LEER_TARJETA_GUARDADA(uint8_t *id_out, uint8_t *len_out); // Lee desde la memoria EEPROM el ID almacenado previamente
bool COMPARAR_IDS(const uint8_t *id1, uint8_t len1, const uint8_t *id2, uint8_t len2); // Compara dos IDs RFID y devuelve true si son iguales
void BORRAR_TARJETA(void); // Elimina el ID guardado en la EEPROM, dejando el sistema sin tarjeta registrada
void VERIFICAR_TARJETA(const uint8_t *id, uint8_t len); // Compara la tarjeta leída con la guardada y determina si el acceso es permitido o denegado
void BUZZER_BEEP(uint8_t n); // Genera una cantidad determinada de beeps cortos en el buzzer como señal acústica

// Función principal
int main(void){
	CONFIGURACION(); // Se llama a la función CONFIGURACION para inicializar los periféricos mencionados
	LCD_MOSTRAR("Bienvenido al", "sistema RFID"); // Se muestra en la LCD el mensaje anexo
	UART_IMPRIMIR("=== Sistema Cerradura RFID Iniciado ===\r\n"); // Se imprime por puerto serial (UART) el mensaje anexo
	_delay_ms(1000); // Se espera 1 segundo para que se inicialice el sistema correctamente antes de la rutina de detección
	LCD_MOSTRAR("Acerque su", "tarjeta RFID"); // Se muestra en la LCD el mensaje anexo

	uint8_t uid[16]; // Se declara un arreglo de 16 elementos de nombre uid, usado para guardar el valor de la id de la tarjeta en la eeprom
	bool token = false; // Se declara la variable token de tipo bool, e inicializada con valor false
	modo_t modo = DETECCION; // Se declara la variable modo, de tipo perteneciente a la estructura modo_t, e inicializada en modo detección

	// Bucle infinito
	while(1){
		// Lectura del pulsador REGISTRO (PD3)
		if (!(PIND & (1 << PD3))){ // Si se presiona el pulsador PD3 asignado a REGISTRO, entonces...
			_delay_ms(500); // Debouncing por software de 500 ms para evitar ruido y multiples recepcion de ordenes
			modo = REGISTRO; // Pasa de modo DETECCIÓN inicialmente establecido a modo REGISTRO para registrar una tarjeta nueva en la eeprom que permita abrir la cerradura
			while (!(PIND & (1 << PD3))); // Espera a que se suelte el botón
			_delay_ms(300); // Retardo adicional  de 300 ms para evitar doble lectura
		}

		// Lectura del pulsador BORRADO (PD2)
		if (!(PIND & (1 << PD2))){ // Si se presiona el pulsador PD2 asignado a BORRADO, entonces...
			_delay_ms(500); // Debouncing por software de 500 ms para evitar ruido y multiples recepcion de ordenes
			modo = BORRADO; // Pasa de modo DETECCIÓN inicialmente establecido a modo BORRADO para borrar la tarjeta guardada en la eeprom que permita abrir la cerradura
			while (!(PIND & (1 << PD2))); // Espera a que se suelte el botón
			_delay_ms(300); // Retardo adicional  de 300 ms para evitar doble lectura
		}

		// Se implementa un switch case que toma la variable modo y acorde a su valor, ejecuta diferentes comandos
		switch (modo){
			// Si la variable modo está en DETECCION, entonces...
			case DETECCION:{
				memset(uid, 0, sizeof(uid)); // Limpia el buffer del UID completandolo con 0s
				RFID_STANDARD(uid); // Intenta leer una tarjeta RFID mediante el módulo RC522

				if (uid[0] != 0x00 && !token){ // Si se detectó un UID válido y aún no se procesó
					token = true; // Asigna true a la variable token para indicar que se detectó la tarjeta
					UART_IMPRIMIR("UID detectado: "); // Imprime por puerto serial el mensaje anexo
					UART_IMPRIMIR_HEX_ARRAY(uid, 4); // Envía el ID leído en formato hexadecimal
					UART_IMPRIMIR("\r\n"); // Envía un salto de línea para mejorar la legibilidad en el monitor serial
					VERIFICAR_TARJETA(uid, 4); // Llama a la función que compara el UID leído con el almacenado
					_delay_ms(1500); // Retardo de 1,5 s para mostrar el mensaje de verificación antes de limpiar
					PORTB &= ~((1 << PB0) | (1 << PB1)); // Apaga ambos LEDs (rojo y verde)
					LCD_MOSTRAR("Acerque su", "tarjeta RFID"); // Muestra en la LCD en mensaje anexo e inicial del sistema
				}
				else if (uid[0] == 0x00){ // Si no hay tarjeta presente
					token = false; // Reinicia la bandera para permitir una nueva lectura
				}
				_delay_ms(100); // Pequeño retardo para estabilidad entre lecturas sucesivas
			} break; // Sale del case DETECCION

			// Si la variable modo está en REGISTRO, entonces...
			case REGISTRO:{
				LCD_MOSTRAR("Registrar", "acerque tarjeta"); // Muestra en la LCD el mensaje anexo
				UART_IMPRIMIR("[REGISTRO] Esperando tarjeta...\r\n"); // Imprime en el puerto serial el mensaje anexo + salto de línea
				bool registrada = false; // Asigna false a la variable bool "registrada" para inicializar el sistema sin tarjeta registrada (sin acceso)
				while (!registrada){ // Bucle while con condición de mientras la variable registrada sea diferente a false (true), entonces...
					memset(uid, 0, sizeof(uid)); // Limpia el buffer del UID completandolo con 0s
					RFID_STANDARD(uid); // Intenta leer una tarjeta RFID mediante el módulo RC522
					if (uid[0] != 0x00){ // Si la lectura de la tarjeta es diferente a cero (se detectó una tarjeta), entonces...
						UART_IMPRIMIR("UID leído: "); // Se imrpime en puerto serial el mensaje anexo
						UART_IMPRIMIR_HEX_ARRAY(uid, 4); // Envía el ID leído en formato hexadecimal
						UART_IMPRIMIR("\r\n"); // Se imrpime en puerto serial un salto de línea
						REGISTRAR_TARJETA(uid, 4); // Se llama a la función REGISTRAR_TARJETA que toma el uid leído, con len = 4, y lo guarda en la eeprom como tarjeta de acceso actual
						BUZZER_BEEP(1); // Llama a la función BUZZER_BEEP con parámetro 1 que emite un beep a través del buzzer indicando el correcto guardado de la tarjeta
						registrada = true; // Se asigna el valor true a la variable "registrada" de tipo bool para indicar que el sistema sí cuenta con una tarjeta de acceso
					}
					_delay_ms(150); // Se esperan 150 ms para evitar lecturas consecutivas del mismo UID y permitir estabilidad entre detecciones
				}
				_delay_ms(1000); // Espera 1 segundo tras el registro para asegurar estabilidad visual y evitar doble detección inmediata
				PORTB &= ~((1 << PB0) | (1 << PB1)); // Apaga ambos LEDs (rojo y verde)
				LCD_MOSTRAR("Acerque su", "tarjeta RFID"); // Muestra en la LCD en mensaje anexo e inicial del sistema
				modo = DETECCION; // Vuelve a asignar el modo del sistema a DETECCION
				token = false; // Reinicia la variable token para permitir una nueva lectura de tarjeta
			} break; // Sale del case REGISTRO

			// Si la variable modo está en BORRADO, entonces...
			case BORRADO:{
				UART_IMPRIMIR("[BORRAR] Eliminando tarjeta registrada...\r\n"); // Se imprime en puerto serial el mensaje anexo
				BORRAR_TARJETA(); // Se llama a la función BORRAR_TARJETA que borra el contenido de la eeprom para dejarla vacía (sin tarjetas de acceso)
				_delay_ms(800); // Espera 800 ms para asegurar que la operación de borrado se complete correctamente
				LCD_MOSTRAR("Tarjeta", "borrada"); // Se muestra en la LCD el mensaje anexo en formato
				UART_IMPRIMIR("[BORRAR] Tarjeta borrada.\r\n"); // Se imprime en puerto serial el mensaje anexo
				_delay_ms(1000); // Espera 1 segundo antes de volver al estado de detección
				LCD_MOSTRAR("Acerque su", "tarjeta RFID"); // Se muestra en la LCD el mensaje anexo
				modo = DETECCION; // Vuelve a asignar el modo del sistema a DETECCION
				token = false; // Reinicia la variable token para permitir una nueva lectura de tarjeta
			} break; // Sale del case BORRADO
		}
	}
}

// Función CONFIGURACIÓN
void CONFIGURACION(void){
	UART_INICIAR((F_CPU / (16UL * 9600UL)) - 1); // Usa el comando de la librería uart.h UART_INICIAR con parámetro de bps calculados automáticamente para inicializar la UART
	I2C_INICIAR(); // Usa el comando de la librería i2c.h I2C_INICIAR para iniciar la comunicación I2C utilizada por el módulo LCD
	LCD_INICIAR(); // Usa el comando de la librería lcd.h LCD_INICIAR para inicializar el display LCD (modo 4 bits con interfaz I2C)
	SPI_INICIAR(); // Usa el comando de la librería spi.h SPI_INICIAR para inicializar la comunicación SPI necesaria para el módulo RFID
	RFID_INICIAR(); // Usa el comando de la librería rc522.h RFID_INICIAR para inicializar el lector RFID MFRC522 y dejarlo listo para lecturas
	
	DDRB |= (1 << PB0) | (1 << PB1); // Configura los pines PB0 y PB1 como salidas digitales (LEDs verde y rojo respectivamente)
	DDRD &= ~((1 << PD2) | (1 << PD3)); // Configura los pines PD2 y PD3 como entradas digitales (BORRADO y REGISTRO respectivamente)
	PORTD |= (1 << PD2) | (1 << PD3); // Activa las resistencias internas pull-up en los pines PD2 y PD3 para mantenerlos en nivel alto cuando no se presionan
	DDRD |= (1 << PD7); // Configura el pin PD7 como salida digital (buzzer)
	PORTD &= ~(1 << PD7); // Inicializa el pin PD7 en nivel bajo (buzzer apagado)
	PORTB &= ~(1 << PB0); // Inicializa el pin PB0 en nivel bajo (LED verde apagado)
	PORTB &= ~(1 << PB1); // Inicializa el pin PB1 en nivel bajo (LED rojo apagado)
}

// Función REGISTRAR_TARJETA
void REGISTRAR_TARJETA(const uint8_t *id, uint8_t len){
	if (len == 0 || len > 10){ // Si la variable len (largo) es igual a 0 o mayor a 10, entonces...
		UART_IMPRIMIR("[REGISTRO] UID invalido, no se guarda.\r\n"); // Imprime en el puerto serial el mensaje anexo
		LCD_MOSTRAR("UID invalido", "no se guardo"); // Muestra en el LCD el mensaje anexo indicando error de guardado
		return; // Finaliza la ejecución de la función y vuelve al programa principal
	}
	// Si la longitud del UID es válida, entonces...
	EEPROM_ESCRIBIR(0x00 + 0, len); // Guarda en la dirección 0x00 de la EEPROM el valor de longitud del UID leído
	for (uint8_t i = 0; i < len; i++) // Bucle for que recorre la cantidad de bytes que conforman el UID
	EEPROM_ESCRIBIR(0x00 + 1 + i, id[i]); // Escribe cada byte del UID en direcciones consecutivas de la EEPROM (desde 0x01 en adelante)
	LCD_MOSTRAR("Nueva tarjeta", "registrada"); // Muestra en la LCD el mensaje anexo
	UART_IMPRIMIR("[REGISTRO] Tarjeta registrada.\r\n"); // Imprime en el puerto serial el mensaje anexo
}

// Función LEER_TARJETA_GUARDADA
void LEER_TARJETA_GUARDADA(uint8_t *id_out, uint8_t *len_out){
	uint8_t len = EEPROM_LEER(0x00 + 0); // Asigna a la variable len el valor leído en la dirección 0x00 de la EEPROM (correspondiente a la longitud del UID almacenado)
	if (len == 0xFF || len == 0x00 || len > 10){ // Si el valor leído es 0xFF (memoria vacía), 0 o mayor a 10, entonces no hay UID válido guardado
		*len_out = 0; // Asigna 0 a la variable len_out para indicar que no hay datos válidos
		return; // Finaliza la ejecución de la función y retorna al programa principal
	}
	// Si la longitud del UID es válida, entonces...
	*len_out = len; // Asigna el valor de len (longitud válida) a la variable de salida len_out
	for (uint8_t i = 0; i < len; i++) // Bucle for que recorre la cantidad de bytes correspondientes al UID almacenado
	id_out[i] = EEPROM_LEER(0x00 + 1 + i); // Carga en el vector id_out cada byte leído desde la EEPROM, comenzando en la dirección 0x01
}

// Función COMPARAR_IDS
bool COMPARAR_IDS(const uint8_t *id1, uint8_t len1, const uint8_t *id2, uint8_t len2){
	if (len1 != len2) return false; // Si las longitudes de ambos IDs son diferentes, devuelve false (los IDs no coinciden)
	return (memcmp(id1, id2, len1) == 0); // Compara ambos vectores byte a byte mediante la función memcmp y devuelve true si son idénticos
}

// Función BORRAR_TARJETA
void BORRAR_TARJETA(void){
	EEPROM_ESCRIBIR(0x00 + 0, 0xFF); // Escribe el valor 0xFF en la dirección 0x00 de la EEPROM para indicar que no hay una longitud válida almacenada
	for (uint8_t i = 0; i < 10; i++) // Bucle for que recorre 10 posiciones consecutivas de memoria EEPROM
	EEPROM_ESCRIBIR(0x00 + 1 + i, 0xFF); // Escribe el valor 0xFF en cada dirección desde 0x01 hasta 0x0A, borrando los bytes del UID previamente guardado
	LCD_MOSTRAR("Tarjeta", "borrada"); // Muestra en la LCD el mensaje anexo
	UART_IMPRIMIR("[BORRAR] Tarjeta borrada.\r\n"); // Imprime en el puerto serial el mensaje anexo
}

// Función VERIFICAR_TARJETA
void VERIFICAR_TARJETA(const uint8_t *id, uint8_t len){
	uint8_t guardado[10]; // Declara un arreglo de 10 bytes para almacenar el ID leído desde la EEPROM
	uint8_t guard_len = 0; // Declara la variable que contiene la longitud del ID guardado
	LEER_TARJETA_GUARDADA(guardado, &guard_len); // Llama a la función que lee desde la EEPROM el ID registrado y su longitud

	if (guard_len == 0){ // Si la longitud leída es 0 (no hay tarjeta guardada), entonces...
		UART_IMPRIMIR("[VERIFICAR] No hay tarjeta registrada.\r\n"); // Imprime en el puerto serial el mensaje anexo
		LCD_MOSTRAR("No hay", "tarjeta guardada"); // Muestra en la LCD el mensaje anexo
		PORTB |= (1 << PB1); // Enciende el LED rojo (error - no hay tarjeta guardada)
		PORTB &= ~(1 << PB0); // Apaga el LED verde
		BUZZER_BEEP(2); // Llama a la función BUZZER_BEEP con parámetro 2, emitiendo dos pitidos de advertencia
		return; // Finaliza la ejecución de la función y retorna al programa principal
	}

	if (COMPARAR_IDS(id, len, guardado, guard_len)){ // Si el UID leído coincide con el UID guardado en EEPROM, entonces...
		LCD_MOSTRAR("Acceso", "permitido"); // Muestra en la LCD el mensaje anexo
		PORTB |= (1 << PB0); // Enciende el LED verde (acceso permitido)
		PORTB &= ~(1 << PB1); // Apaga el LED rojo
		BUZZER_BEEP(1); // Llama a la función BUZZER_BEEP con parámetro 1, emitiendo un solo pitido corto de confirmación
		UART_IMPRIMIR("[VERIFICAR] Acceso permitido.\r\n"); // Imprime en el puerto serial el mensaje anexo
		}else{ // Si los UIDs no coinciden...
		LCD_MOSTRAR("Acceso", "denegado"); // Muestra en la LCD el mensaje anexo
		PORTB |= (1 << PB1); // Enciende el LED rojo (acceso denegado)
		PORTB &= ~(1 << PB0); // Apaga el LED verde
		BUZZER_BEEP(2); // Llama a la función BUZZER_BEEP con parámetro 2, emitiendo dos pitidos indicando acceso denegado
		UART_IMPRIMIR("[VERIFICAR] Acceso denegado.\r\n"); // Imprime en el puerto serial el mensaje anexo
	}
}

// Función BUZZER_BEEP
void BUZZER_BEEP(uint8_t n){
	for (uint8_t i = 0; i < n; i++){ // Bucle for que repite la cantidad de veces indicada por el parámetro n
		PORTD |= (1 << PD7); // Activa el buzzer (coloca PD7 en nivel alto)
		_delay_ms(100); // Mantiene el buzzer encendido durante 100 ms
		PORTD &= ~(1 << PD7); // Desactiva el buzzer (coloca PD7 en nivel bajo)
		_delay_ms(100); // Espera 100 ms antes del siguiente pitido
	}
}
