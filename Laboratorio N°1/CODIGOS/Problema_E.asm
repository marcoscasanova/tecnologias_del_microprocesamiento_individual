; LABORATORIO_Problema_E.asm
; Created: 17/9/2025 16:48:55
; Author: Marcos Casanova, Luis Bouvier, Santiago Moizo

.equ F_CPU = 16000000					;Se define el valor de la variable F_CPU (frecuencia del microcontrolador)
.equ baud = 9600						;Se define el valor de la variable baud (baudios)
.equ bps = (F_CPU/16/baud) - 1			;Se define el valor de la variable bps (baudios por segundo) como el resultado de la operación indicada

.dseg									;Inicio del segmento de datos en RAM
opcion: .byte 1							;Reserva 1 byte en memoria de datos para la variable opcion

.cseg									;Inicio del segmento de instrucciones
.org 0x00								;Direccion de inicio
    rjmp INICIO							;Salto relativo a etiqueta INICIO

INICIO:
	ldi r16, LOW(RAMEND)				;Carga la parte baja de la dirección final de RAM en r16
	out SPL, r16						;Configura el registro SPL (Stack Pointer Low) con ese valor
	ldi r16, HIGH(RAMEND)				;Carga la parte alta de la dirección final de RAM en r16
	out SPH, r16						;Configura el registro SPH (Stack Pointer High) con ese valor

	ldi r16, 0xFC						;Carga inmediatamente el valor hexadecimal 0xFC en el registro r16
	out DDRD, r16						;Asigna los pines PD2, PD3, PD4, PD5, PD6, PD7 como salidas

	ldi r16, 0x00						;Carga inmediatamente el valor 0 en el registro r16
	out PORTD, r16						;Pone en nivel bajo los pines del puerto D asignados como salidas

    ldi r16, LOW(bps)
    sts UBRR0L, r16						;Configura la parte baja del divisor de baudios (UBRR0L en 0xC4)
    ldi r16, HIGH(bps)
    sts UBRR0H, r16						;Configura la parte alta del divisor de baudios (UBRR0H en 0xC5)

    ldi r16, (1<<RXEN0)|(1<<TXEN0)		;Habilita la recepción (RX) y transmisión (TX) de la UART
    sts UCSR0B, r16						;Guarda la configuración en el registro de control UART B
    ldi r16, (1<<UCSZ01)|(1<<UCSZ00)	;Configura el formato de datos de la UART en 8 bits
    sts UCSR0C, r16						;Guarda la configuración en el registro de control UART C

	rjmp MENU							;Salto relativo a la etiqueta MENU

MENU:
    ldi ZH, HIGH(OPCIONES*2)			;Carga en ZH la parte alta de la dirección del mensaje OPCIONES (multiplicado por 2 por direccionamiento en palabras)
    ldi ZL, LOW(OPCIONES*2)				;Carga en ZL la parte baja de la dirección del mensaje OPCIONES (multiplicado por 2 por direccionamiento en palabras)

    rjmp TX_OPCIONES					;Salto relativo a la etiqueta TX_OPCIONES

UART_TX:
    lds r17, UCSR0A						;Carga directamente el valor del registro UCSR0A en el registro r17
    sbrs r17, UDRE0						;Omite la siguiente instrucción si el bit UDRE0 está en 0 (buffer de transmisión no listo)
    rjmp UART_TX						;Salto relativo a la etiqueta UART_TX (espera hasta que el buffer de transmisión esté listo)
    sts UDR0, r16						;Guarda directamente el valor del registro r16 en UDR0 (envía el dato por UART)
    ret									;Retorno de la subrutina

TX_OPCIONES:
    lpm r16, Z+							;Carga un byte desde la memoria de programa (tabla apuntada por Z) en r16 y luego incrementa Z
    cpi r16, 0							;Compara r16 con 0 (fin de cadena detectado)
    breq MAIN_LOOP						;Si r16 = 0, salta a MAIN_LOOP; si no, continúa
    rcall UART_TX						;Llama a la rutina UART_TX	de transmisión UART para enviar el byte
    rjmp TX_OPCIONES					;Salto relativo a la etiqueta TX_OPCIONES

UART_RX:
	lds r17, UCSR0A						;Carga el valor del registro de estado UCSR0A en r17
    sbrs r17, RXC0						;Omite la siguiente instrucción si el bit RXC0 está en 0 (no hay dato recibido)
    rjmp UART_RX						;Vuelve a esperar hasta que llegue un dato
    lds r16, UDR0						;Carga en r16 el dato recibido en el registro UDR0
    ret									;Retorno de la subrutina

MAIN_LOOP:
	rcall UART_RX						;Llamada relativa a la rutina UART_RX
    sts opcion, r16						;Guarda directamente en la variable opcion reservada en la RAM, el valor del registro r16
    rjmp SELECCIONAR_OPCION				;Salto relativo a la etiqueta SELECCIONAR_OPCION

CALL_TRIANGULO:
    rcall TRIANGULO						;Llamada relativa a al rutina TRIANGULO
    rjmp MENU							;Regresa al menú principal

CALL_CIRCULO:
    rcall CIRCULO						;Llamada relativa a al rutina CIRCULO
    rjmp MENU							;Regresa al menú principal

CALL_CRUZ:
    rcall CRUZ							;Llamada relativa a al rutina CRUZ
    rjmp MENU							;Regresa al menú principal

CALL_FIGURAS:
    rcall FIGURAS						;Llamada relativa a al rutina FIGURAS
    rjmp MENU							;Regresa al menú principal

SELECCIONAR_OPCION:
	lds r16, opcion						;Carga directamente el valor de opcion y lo guarda en el registro r16
	cpi r16, '1'						;Compara el valor del registro r16 con "1"
	breq CALL_TRIANGULO					;Si la comparación coincide, salta a la etiqueta CALL_TRIANGULO, sino, continúa
	cpi r16, '2'						;Compara el valor del registro r16 con "2"
	breq CALL_CIRCULO					;Si la comparación coincide, salta a la etiqueta CALL_CIRCULO, sino, continúa
	cpi r16, '3'						;Compara el valor del registro r16 con "3"
	breq CALL_CRUZ						;Si la comparación coincide, salta a la etiqueta CALL_CRUZ, sino, continúa
	cpi r16, 'T'						;Compara el valor del registro r16 con "T"
	breq SALTO_FIGURAS					;Si la comparación coincide, salta a la etiqueta SALTO_FIGURAS, sino, continúa
	rjmp MAIN_LOOP						;Salto relativo a la etiqueta MAIN_LOOP

SALTO_FIGURAS:
    rjmp CALL_FIGURAS					;Salto relativo a la etiqueta CALL_FIGURAS

TRIANGULO:
;Bajar solenoide
	rcall PLOTTER_BAJAR

;Mover a la izquierda 6 segundos
	rcall PLOTTER_IZQUIERDA
	ldi r18, 60
	rcall DELAY

;Mover abajo y a la derecha a la vez por 3 segundos
	rcall PLOTTER_ABAJO_DERECHA
	ldi r18, 30
	rcall DELAY

;Mover arriba y a la derecha a la vez por 3 segundos
	rcall PLOTTER_ARRIBA_DERECHA
	ldi r18, 30
	rcall DELAY

;Subir solenoide
	rcall PLOTTER_SUBIR

    ret

CIRCULO:
;Se realiza un círculo aproximado en pasos discretos, dividido en cuadrantes de 64 segmentos de resolución

;--------------- CUADRANTE 1 ---------------

	rcall PLOTTER_IZQUIERDA_NO_BAJAR
	ldi r18, 50
	rcall DELAY

	rcall PLOTTER_BAJAR

	rcall PLOTTER_DERECHA
	ldi r18, 6
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 4
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 3
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 3
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 4
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 5
	rcall DELAY

;--------------- CUADRANTE 2 ---------------

	rcall PLOTTER_ABAJO
	ldi r18, 6
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY
	
	rcall PLOTTER_ABAJO
	ldi r18, 4
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 3
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 3
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 4
	rcall DELAY

	rcall PLOTTER_ABAJO
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 5
	rcall DELAY

;--------------- CUADRANTE 3 ---------------

	rcall PLOTTER_IZQUIERDA
	ldi r18, 6
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 4
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 3
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 3
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 4
	rcall DELAY

	rcall PLOTTER_IZQUIERDA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 5
	rcall DELAY

;--------------- CUADRANTE 4 ---------------

	rcall PLOTTER_ARRIBA
	ldi r18, 6
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 4
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 3
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 3
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 2
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 4
	rcall DELAY

	rcall PLOTTER_ARRIBA
	ldi r18, 1
	rcall DELAY

	rcall PLOTTER_DERECHA
	ldi r18, 6
	rcall DELAY

	rcall PLOTTER_SUBIR

    ret

CRUZ:
;Mover hacia la izquierda sin bajar la solenoide por 2.5 segundos
	rcall PLOTTER_IZQUIERDA_NO_BAJAR
	ldi r18, 25
	rcall DELAY

;Mover hacia abajo
	rcall PLOTTER_ABAJO
	ldi r18, 50
	rcall DELAY

;Subir solenoide
	rcall PLOTTER_SUBIR

;Mover hacia arriba sin bajar la solenoide
	rcall PLOTTER_ARRIBA_NO_BAJAR
	ldi r18, 25
	rcall DELAY

;Mover hacia la derecha sin bajar la solenoide
	rcall PLOTTER_DERECHA_NO_BAJAR
	ldi r18, 25
	rcall DELAY

;Mover hacia la izquierda
	rcall PLOTTER_IZQUIERDA
	ldi r18, 50
	rcall DELAY

;Subir solenoide
	rcall PLOTTER_SUBIR

    ret

FIGURAS:
    rcall TRIANGULO						;Llamada relativa a la rutina TRIANGULO
    rcall PLOTTER_IZQUIERDA_NO_BAJAR	;Mover hacia la izquierda sin bajar la solenoide por 6.5 segundos
	ldi r18, 65
	rcall DELAY
    rcall CRUZ							;Llamada relativa a la rutina CRUZ
	rcall PLOTTER_ARRIBA_NO_BAJAR		;Mover hacia la izquierda sin bajar la solenoide por 5 segundos
	ldi r18, 50
	rcall DELAY
	rcall CIRCULO						;Llamada relativa a la rutina CIRCULO

    ret

PLOTTER_SUBIR:
	ldi r16, 0b00001000					;Activa bit PD3 para subir el solenoide
	out PORTD, r16
	ldi r18, 5							;Mantiene el pulso un breve tiempo
	rcall DELAY
	ret

PLOTTER_BAJAR:
	ldi r16, 0b00000100					;Activa bit PD2 para bajar el solenoide
	out PORTD, r16
	ldi r18, 5							;Mantiene el pulso un breve tiempo
	rcall DELAY
	ret

PLOTTER_ARRIBA_NO_BAJAR:
	ldi r16, 0b00100000					;Activa PD5 para mover hacia arriba
	out PORTD, r16
	ret

PLOTTER_ABAJO_NO_BAJAR:
	ldi r16, 0b00010000					;Activa PD4 para mover hacia abajo
	out PORTD, r16
	ret

PLOTTER_DERECHA_NO_BAJAR:
	ldi r16, 0b01000000					;Activa PD6 para mover hacia la derecha
	out PORTD, r16
	ret

PLOTTER_IZQUIERDA_NO_BAJAR:
	ldi r16, 0b10000000					;Activa PD7 para mover hacia la izquierda
	out PORTD, r16
	ret

PLOTTER_ARRIBA:
	ldi r16, 0b00100100					;Activa PD5 (arriba) y PD2 (bajar solenoide)
	out PORTD, r16
	ret

PLOTTER_ABAJO:
	ldi r16, 0b00010100					;Activa PD4 (abajo) y PD2 (bajar solenoide)
	out PORTD, r16
	ret

PLOTTER_DERECHA:
	ldi r16, 0b01000100					;Activa PD6 (derecha) y PD2 (bajar solenoide)
	out PORTD, r16
	ret

PLOTTER_IZQUIERDA:
	ldi r16, 0b10000100					;Activa PD7 (izquierda) y PD2 (bajar solenoide)
	out PORTD, r16
	ret

PLOTTER_ARRIBA_DERECHA:
	ldi r16, 0b01100100					;Activa PD5 (arriba), PD6 (derecha) y PD2 (bajar solenoide)
	out PORTD, r16
	ret

PLOTTER_ABAJO_DERECHA:
	ldi r16, 0b01010100					;Activa PD4 (abajo), PD6 (derecha) y PD2 (bajar solenoide)
	out PORTD, r16
	ret

DELAY:
;Se anidan varios bucles con diferentes registros para lograr un décimo exacto de segundo, y a través del registro r18 darle el valor deseado para cada figura / secuencia
    ldi r20, 100						;Carga el contador externo
DELAY_1:
    ldi  r24, 32						;Carga el contador medio
DELAY_2:
    ldi  r25, 166						;Carga el contador interno
DELAY_3:
    dec  r25							;Decrementa el contador interno
    brne DELAY_3						;Repite hasta llegar a 0
    dec  r24							;Decrementa el contador medio
    brne DELAY_2
    dec  r20							;Decrementa el contador externo
    brne DELAY_1
    dec  r18							;Decrementa el argumento de duración
    brne DELAY
    ret									;Retorno al final de la espera

.cseg
OPCIONES:
.db "Seleccione: 1 = Triangulo, 2 = Circulo, 3 = Cruz, T = Todas",0
