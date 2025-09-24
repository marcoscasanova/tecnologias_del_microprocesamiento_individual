; LABORATORIO_Problema_D.asm
; Created: 20/9/2025 15:54:22
; Author: Marcos Casanova, Luis Bouvier, Santiago Moizo

.equ F_CPU = 16000000						      ;Definimos la frecuencia con la que trabaja el microcontrolador
.equ baud = 9600							        ;Definimos la velocidad con la que trabaja el microcontrolador
.equ bps = (F_CPU/16/baud) - 1				;Definimos los baudios por segundo con los que trabaja el microcontrolador para una correcta comunicación con el mismo

.cseg										              ;Inicio del segmento de instrucciones
.org 0x00									            ;Direccion de inicio
    rjmp INICIO								        ;Salto relativo a etiqueta INICIO

INICIO:
	ldi r16, LOW(RAMEND)					      ;Carga la parte baja de la dirección final de RAM en r16
	out SPL, r16							          ;Configura el registro SPL (Stack Pointer Low) con ese valor
	ldi r16, HIGH(RAMEND)					      ;Carga la parte alta de la dirección final de RAM en r16
	out SPH, r16							          ;Configura el registro SPH (Stack Pointer High) con ese valor

	ldi r16, 0xFC							          ;Carga inmediatamente el valor hexadecimal FC en el registro r16
	out DDRD, r16							          ;Asignamos como salida los pines PD2, PD3, PD4, PD5, PD6 y PD7

	ldi r16, 0x00							          ;Carga inmediatamente el valor hexadecimal 00 en el registro r16
	out PORTD, r16							        ;Carga en nivel bajo (0) los pines anteriormente asignados como salida del puerto D

	rjmp MAIN_LOOP							        ;Salto relativo a la etiqueta MAIN_LOOP

MAIN_LOOP:
	rcall MAIN_LOOP							        ;Llamada relativa a la subrutina MAIN_LOOP

PLOTTER_BAJAR:
    sbi PORTD, 2							        ;Coloca un 1 en el puerto PD2
    rcall DELAY								        ;Llamada relativa a la rutina DELAY
    ret										            ;Retorna de la rutina

PLOTTER_SUBIR:
    sbi PORTD, 3							        ;Coloca un 1 en el puerto PD3
	rcall DELAY								          ;Llamada relativa a la rutina DELAY
    ret										            ;Retorna de la rutina

PLOTTER_ARRIBA:		
    sbi PORTD, 5							        ;Coloca un 1 en el puerto PD5
    rcall DELAY								        ;Llamada relativa a la rutina DELAY
    ret										            ;Retorna de la rutina

PLOTTER_ABAJO:
    sbi PORTD, 4							        ;Coloca un 1 en el puerto PD4
    rcall DELAY								        ;Llamada relativa a la rutina DELAY
    ret										            ;Retorna de la rutina

PLOTTER_DERECHA:
    sbi PORTD, 7							        ;Coloca un 1 en el puerto PD7
    rcall DELAY								        ;Llamada relativa a la rutina DELAY
    ret										            ;Retorna de la rutina

PLOTTER_IZQUIERDA:
    sbi PORTD, 6							        ;Coloca un 1 en el puerto PD6
    rcall DELAY								        ;Llamada relativa a la rutina DELAY
    ret	

DELAY:										            ;Anidamos 4 bucles para lograr un décimo de segundo con r18 = 1 y modificando el valor de este registro obtenemos un delay personalizado cuando se llame al mismo
    ldi r20, 100							        ;Carga inmediatamente el valor 100 en el registro r20 (bucle)
DELAY_1:
    ldi  r24, 32							        ;Carga inmediatamente el valor 32 en el registro r24 (bucle)
DELAY_2:	
    ldi  r25, 166							        ;Carga inmediatamente el valor 166 en el registro r25 (bucle)
DELAY_3:
    dec  r25								          ;Decrementa el valor del registro r25
    brne DELAY_3							        ;Salta a la etiqueta DELAY_3 si el valor de r25 no es 0
    dec  r24								          ;Decrementa el valor del registro r24
    brne DELAY_2							        ;Salta a la etiqueta DELAY_2 si el valor de r24 no es 0
    dec  r20								          ;Decrementa el valor del registro r20
    brne DELAY_1							        ;Salta a la etiqueta DELAY_1 si el valor de r20 no es 0
    dec  r18								          ;Decrementa el valor de registro r18
    brne DELAY								        ;Salta a la etiqueta DELAY si el valor de r18 no es 0
    ret										            ;Retorna de la rutina

MODO_REPOSO:
    cbi PORTD, 2							        ;Coloca un 0 en el puerto PD2
    sbi PORTD, 3							        ;Coloca un 1 en el puerto PD3
    cbi PORTD, 4							        ;Coloca un 0 en el puerto PD4
    cbi PORTD, 5							        ;Coloca un 0 en el puerto PD5
    cbi PORTD, 6							        ;Coloca un 0 en el puerto PD6
    cbi PORTD, 7							        ;Coloca un 0 en el puerto PD7
    ret										            ;Retorna de la rutina
