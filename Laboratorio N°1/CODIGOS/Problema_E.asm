; LABORATORIO_Problema_E.asm
; Created: 17/9/2025 16:48:55
; Author: Marcos Casanova, Luis Bouvier, Santiago Moizo

.equ F_CPU = 16000000						        ;Definimos la frecuencia con la que trabaja el microcontrolador
.equ baud = 9600							          ;Definimos la velocidad con la que trabaja el microcontrolador
.equ bps = (F_CPU/16/baud) - 1				  ;Definimos los baudios por segundo con los que trabaja el microcontrolador para una correcta comunicación con el mismo

.dseg										                ;Inicio del segmento de datos en la RAM
opcion: .byte 1								          ;Reservamos un byte de memoria en la RAM para la variable opcion

.cseg										                ;Inicio del segmento de instrucciones
.org 0x00									              ;Direccion de inicio
    rjmp INICIO								          ;Salto relativo a etiqueta INICIO

INICIO:
	ldi r16, LOW(RAMEND)					        ;Carga la parte baja de la dirección final de RAM en r16
	out SPL, r16							            ;Configura el registro SPL (Stack Pointer Low) con ese valor
	ldi r16, HIGH(RAMEND)					        ;Carga la parte alta de la dirección final de RAM en r16
	out SPH, r16							            ;Configura el registro SPH (Stack Pointer High) con ese valor

	ldi r16, 0xFC							            ;Carga inmediatamente el valor hexadecimal FC en el registro r16
	out DDRD, r16							            ;Asignamos como salida los pines PD2, PD3, PD4, PD5, PD6 y PD7

	ldi r16, 0x00							            ;Carga inmediatamente el valor hexadecimal 00 en el registro r16
	out PORTD, r16							          ;Carga en nivel bajo (0) los pines anteriormente asignados como salida del puerto D
	
	ldi r16, LOW(bps)						          ;Configura la parte baja del divisor de baudios (UBRR0L en 0xC4)
    sts UBRR0L, r16
    ldi r17, HIGH(bps)						      ;Configura la parte alta del divisor de baudios (UBRR0H en 0xC5)
    sts UBRR0H, r17

    ldi r16, (1<<RXEN0)|(1<<TXEN0)		  ;Habilita la recepción (RX) y transmisión (TX) de la UART
    sts UCSR0B, r16							        ;Guarda la configuración en el registro de control UART B
    ldi r16, (1<<UCSZ01)|(1<<UCSZ00)		;Configura el formato de datos de la UART en 8 bits
    sts UCSR0C, r16							        ;Guarda la configuración en el registro de control UART C
		
	rjmp MENU								              ;Salto relativo a la etiqueta MENU

MENU:
    ldi ZH, HIGH(OPCIONES)					    ;Carga en ZH la parte alta de la dirección del menu
    ldi ZL, LOW(OPCIONES)					      ;Carga en ZL la parte baja de la dirección del menu
    rjmp TX_OPCIONES						        ;Salto relativo a la etiqueta TX_OPCIONES

UART_TX:
    lds r17, UCSR0A							        ;Lee el dato que esta en el registro de UART UCSR0A y lo mete en el registro r17
    sbrs r17, UDRE0							        ;Salta la próxima instrucción si el bit UDRE0 = 1
    rjmp UART_TX							          ;Salto relativo a la etiqueta UART_TX
    sts UDR0, r16							          ;Transfiere el dato almacenado en r16 al registro de datos UDR0
    ret										              ;Retorna de la rutina

TX_OPCIONES:
    lpm r16, Z+								          ;Carga un byte desde la memoria de programa (puntero Z) en r16 e incrementa Z
    cpi r16, 0								          ;Compara inmediatamente el valor de r16 con 0
    breq MAIN_LOOP							        ;Salta a la etiqueta MAIN_LOOP si r16 es 0, sino continua con la siguiente instruccion
    rcall UART_TX							          ;Llamada relativa a la rutina UART_TX
    rjmp TX_OPCIONES						        ;Salto relativo a la etiqueta TX_OPCIONES

UART_RX:
	lds r17, UCSR0A							          ;Lee el dato que esta en el registro de UART UCSR0A y lo mete en el registro r17
    sbrs r17, RXC0							        ;Salta la próxima instrucción si el bit RXC0 = 1
    rjmp UART_RX							          ;Salto relativo a la rutina UART_RX
    lds r16, UDR0							          ;Lee el dato que esta en el registro de la UART UDR0 y lo mete en el registro r16
    ret										              ;Retorna de la rutina

MAIN_LOOP:
	rcall UART_RX							            ;Llamada relativa a la rutina UART_RX
    sts opcion, r16							        ;Guarda directamente el valor del registro r16 en la variable opcion
    rjmp SELECCIONAR_OPCION					    ;Salto relativo a la etiqueta SELECCIONAR_OPCION

SELECCIONAR_OPCION:
	lds r16, opcion							          ;Lee el dato que esta en la variable opcion y lo mete en el registro r16
	cpi r16, '1'							            ;Compara inmediatamente el valor del registro r16 con 1
	breq TRIANGULO							          ;Si el valor del registro r16 es 1 salta a la etiqueta TRIANGULO, sino continua con la siguiente instruccion
	cpi r16, '2'							            ;Compara inmediatamente el valor del registro r16 con 2
	breq CIRCULO							            ;Si el valor del registro r16 es 2 salta a la etiqueta CIRCULO, sino continua con la siguiente instruccion
	cpi r16, '3'							            ;Compara inmediatamente el valor del registro r16 con 3
	breq CRUZ								              ;Si el valor del registro r16 es 3 salta a la etiqueta CRUZ, sino continua con la siguiente instruccion
	cpi r16, 'T'							            ;Compara inmediatamente el valor del registro r16 con T
	breq SALTO_FIGURAS						        ;Si el valor del registro r16 es T salta a la etiqueta SALTO_FIGURAS, sino continua con la siguiente instruccion
	rjmp MAIN_LOOP							          ;Salto relativo a la etiqueta MAIN_LOOP

SALTO_FIGURAS:
    rjmp FIGURAS							          ;Salto relativo a la etiqueta FIGURAS

TRIANGULO:
	ldi r18, 30								            ;Carga inmediatamente el valor 50 en el registro r18 para configurar el delay en 5s
	rcall PLOTTER_BAJAR						        ;Llamada relativa a la rutina PLOTTER_BAJAR
	ldi r18, 30								            ;Carga inmediatamente el valor 50 en el registro r18 para configurar el delay en 5s
	rcall PLOTTER_IZQUIERDA					      ;Llamada relativa a la rutina PLOTTER_IZQUIERDA
	ldi r18, 30								            ;Carga inmediatamente el valor 50 en el registro r18 para configurar el delay en 5s
	rcall PLOTTER_ABAJO_DERECHA				    ;Llamada relativa a la rutina PLOTTER_ABAJO_DERECHA
	ldi r18, 30								            ;Carga inmediatamente el valor 50 en el registro r18 para configurar el delay en 5s
	rcall PLOTTER_ARRIBA_DERECHA			    ;Llamada relativa a la rutina PLOTTER_ARRIBA_DERECHA
	ldi r18, 30								            ;Carga inmediatamente el valor 50 en el registro r18 para configurar el delay en 5s
	rcall PLOTTER_SUBIR						        ;Llamada relativa a la rutina PLOTTER_SUBIR
	rcall MODO_REPOSO						          ;Llamada relativa a la rutina MODO_REPOSO
	rjmp MENU								              ;Salto relativo a la etiqueta MENU

CIRCULO:
    ; Bajamos el lápiz
	ldi r18, 30
    rcall PLOTTER_BAJAR

    ; --- Cuadrante 1: arriba + derecha ---
    ldi r18, 10
    rcall PLOTTER_ARRIBA
    ldi r18, 5
    rcall PLOTTER_ARRIBA_DERECHA
    ldi r18, 10
    rcall PLOTTER_ARRIBA
    ldi r18, 5
    rcall PLOTTER_ARRIBA_DERECHA

    ; --- Cuadrante 2: derecha + abajo ---
    ldi r18, 10
    rcall PLOTTER_DERECHA
    ldi r18, 5
    rcall PLOTTER_ABAJO_DERECHA
    ldi r18, 10
    rcall PLOTTER_DERECHA
    ldi r18, 5
    rcall PLOTTER_ABAJO_DERECHA

    ; --- Cuadrante 3: abajo + izquierda ---
    ldi r18, 10
    rcall PLOTTER_ABAJO
    ldi r18, 5
    rcall PLOTTER_ABAJO_IZQUIERDA
    ldi r18, 10
    rcall PLOTTER_ABAJO
    ldi r18, 5
    rcall PLOTTER_ABAJO_IZQUIERDA

    ; --- Cuadrante 4: izquierda + arriba ---
    ldi r18, 10
    rcall PLOTTER_IZQUIERDA
    ldi r18, 5
    rcall PLOTTER_ARRIBA_IZQUIERDA
    ldi r18, 10
    rcall PLOTTER_IZQUIERDA
    ldi r18, 5
    rcall PLOTTER_ARRIBA_IZQUIERDA

    ; Levantamos lápiz y reposo
    rcall PLOTTER_SUBIR
    rcall MODO_REPOSO
    rjmp MENU

CRUZ:
	ldi r18, 30								            ;Carga inmediatamente el valor 50 en el registro r18 para configurar el delay en 5s
	rcall PLOTTER_BAJAR						        ;Llamada relativa a la rutina PLOTTER_BAJAR
	ldi r18, 30								            ;Carga inmediatamente el valor 50 en el registro r18 para configurar el delay en 5s
	rcall PLOTTER_ABAJO						        ;Llamada relativa a la rutina PLOTTER_ABAJO
	ldi r18, 30								            ;Carga inmediatamente el valor 50 en el registro r18 para configurar el delay en 5s
	rcall PLOTTER_SUBIR						        ;Llamada relativa a la rutina PLOTTER_SUBIR
	ldi r18, 30								            ;Carga inmediatamente el valor 25 en el registro r18 para configurar el delay en 2.5s
	rcall PLOTTER_ARRIBA					        ;Llamada relativa a la rutina PLOTTER_ARRIBA
	ldi r18, 30								            ;Carga inmediatamente el valor 25 en el registro r18 para configurar el delay en 2.5s
	rcall PLOTTER_DERECHA					        ;Llamada relativa a la rutina PLOTTER_DERECHA
	ldi r18, 30								            ;Carga inmediatamente el valor 50 en el registro r18 para configurar el delay en 5s
	rcall PLOTTER_BAJAR						        ;Llamada relativa a la rutina PLOTTER_BAJAR
	ldi r18, 30								            ;Carga inmediatamente el valor 50 en el registro r18 para configurar el delay en 5s
	rcall PLOTTER_IZQUIERDA					      ;Llamada relativa a la rutina PLOTTER_IZQUIERDA
	ldi r18, 30								            ;Carga inmediatamente el valor 50 en el registro r18 para configurar el delay en 5s
	rcall PLOTTER_SUBIR						        ;Llamada relativa a la rutina PLOTTER_SUBIR
	rcall MODO_REPOSO						          ;Llamada relativa a ala rutina MODO_REPOSO
	rjmp MENU								              ;Salto relativo a la etiqueta MENU

FIGURAS:				
	rcall TRIANGULO							          ;Llamada relativa a a la rutina TRIANGULO
	ldi r18, 100							            ;Carga inmediatamente el valor 100 en el registro r18 para configurar el delay en 10s
	rcall PLOTTER_IZQUIERDA					      ;Llamada relativa a a la rutina PLOTTER_IZQUIERDA
	rcall CIRCULO							            ;Llamada relativa a a la rutina CIRCULO
	ldi r18, 100							            ;Carga inmediatamente el valor 100 en el registro r18 para configurar el delay en 10s
	rcall PLOTTER_IZQUIERDA					      ;Llamada relativa a a la rutina PLOTTER_IZQUIERDA
	rcall CRUZ								            ;Llamada relativa a a la rutina CRUZ
	rjmp MENU								              ;Salto relativo a la etiqueta MENU

PLOTTER_BAJAR:
    sbi PORTD, 2							          ;Coloca un 1 en el puerto PD2
    rcall DELAY								          ;Llamada relativa a la rutina DELAY
    ret										              ;Retorna de la rutina

PLOTTER_SUBIR:
    sbi PORTD, 3							          ;Coloca un 1 en el puerto PD3
	rcall DELAY								            ;Llamada relativa a la rutina DELAY
    ret										              ;Retorna de la rutina

PLOTTER_ARRIBA:		
    sbi PORTD, 5							          ;Coloca un 1 en el puerto PD5
    rcall DELAY								          ;Llamada relativa a la rutina DELAY
    ret										              ;Retorna de la rutina

PLOTTER_ABAJO:
    sbi PORTD, 4							          ;Coloca un 1 en el puerto PD4
    rcall DELAY								          ;Llamada relativa a la rutina DELAY
    ret										              ;Retorna de la rutina

PLOTTER_DERECHA:
    sbi PORTD, 7							          ;Coloca un 1 en el puerto PD7
    rcall DELAY								          ;Llamada relativa a la rutina DELAY
    ret										              ;Retorna de la rutina

PLOTTER_IZQUIERDA:
    sbi PORTD, 6							          ;Coloca un 1 en el puerto PD6
    rcall DELAY								          ;Llamada relativa a la rutina DELAY
    ret										              ;Retorna de la rutina

PLOTTER_ARRIBA_DERECHA:
    sbi PORTD, 5							          ;Coloca un 1 en el puerto PD5
    sbi PORTD, 7							          ;Coloca un 1 en el puerto PD7
    rcall DELAY								          ;Llamada relativa a la rutina DELAY
    ret										              ;Retorna de la rutina

PLOTTER_ARRIBA_IZQUIERDA:
    sbi PORTD, 5							          ;Coloca un 1 en el puerto PD5
    sbi PORTD, 6							          ;Coloca un 1 en el puerto PD6
    rcall DELAY								          ;Llamada relativa a la rutina DELAY
    ret	

PLOTTER_ABAJO_DERECHA:
    sbi PORTD, 4							          ;Coloca un 1 en el puerto PD4
    sbi PORTD, 7							          ;Coloca un 1 en el puerto PD7
    rcall DELAY								          ;Llamada relativa a la rutina DELAY
    ret										              ;Retorna de la rutina

PLOTTER_ABAJO_IZQUIERDA:
    sbi PORTD, 4							          ;Coloca un 1 en el puerto PD4
    sbi PORTD, 6							          ;Coloca un 1 en el puerto PD7
    rcall DELAY								          ;Llamada relativa a la rutina DELAY
    ret	

DELAY:										              ;Anidamos 4 bucles para lograr un décimo de segundo con r18 = 1 y modificando el valor de este registro obtenemos un delay personalizado cuando se llame al mismo
    ldi r20, 100							          ;Carga inmediatamente el valor 100 en el registro r20 (bucle)
DELAY_1:
    ldi  r24, 32							          ;Carga inmediatamente el valor 32 en el registro r24 (bucle)
DELAY_2:	
    ldi  r25, 166							          ;Carga inmediatamente el valor 166 en el registro r25 (bucle)
DELAY_3:
    dec  r25								            ;Decrementa el valor del registro r25
    brne DELAY_3							          ;Salta a la etiqueta DELAY_3 si el valor de r25 no es 0
    dec  r24								            ;Decrementa el valor del registro r24
    brne DELAY_2							          ;Salta a la etiqueta DELAY_2 si el valor de r24 no es 0
    dec  r20								            ;Decrementa el valor del registro r20
    brne DELAY_1							          ;Salta a la etiqueta DELAY_1 si el valor de r20 no es 0
    dec  r18								            ;Decrementa el valor de registro r18
    brne DELAY								          ;Salta a la etiqueta DELAY si el valor de r18 no es 0
    ret									              	;Retorna de la rutina

MODO_REPOSO:
    cbi PORTD, 2							          ;Coloca un 0 en el puerto PD2
    sbi PORTD, 3							          ;Coloca un 1 en el puerto PD3
    cbi PORTD, 4							          ;Coloca un 0 en el puerto PD4
    cbi PORTD, 5							          ;Coloca un 0 en el puerto PD5
    cbi PORTD, 6							          ;Coloca un 0 en el puerto PD6
    cbi PORTD, 7							          ;Coloca un 0 en el puerto PD7
    ret										              ;Retorna de la rutina

OPCIONES:
.db "Seleccione: 1 = Triangulo, 2 = Circulo, 3 = Cruz, T = Todas",0	;Mensaje de opciones disponibles
