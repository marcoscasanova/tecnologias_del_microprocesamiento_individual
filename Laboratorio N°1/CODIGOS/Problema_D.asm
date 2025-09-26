; LABORATORIO_Problema_D.asm
; Created: 20/9/2025 15:54:22
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
