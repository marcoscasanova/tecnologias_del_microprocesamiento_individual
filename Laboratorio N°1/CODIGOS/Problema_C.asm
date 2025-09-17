;--------------Constantes y Vectores----------------------
.equ LUT_LEN    = 64
.equ OCR2A_VAL  = 249          ; 16e6/(8*(249+1)) = 8 kHz

.def idx  = r16
.def tmp  = r17


.cseg
.org 0x0000
    rjmp RESET

.org OC2Aaddr                 ; vector TIMER2 COMPA
    rjmp TIMER2_COMPA_ISR

;------------------- RESET ------------------------------
RESET:
    ; Stack
    ldi  tmp, LOW(RAMEND)
    out  SPL, tmp
    ldi  tmp, HIGH(RAMEND)
    out  SPH, tmp

    ; Asegurar r1=0 por AVR, necesario que este en 0
    clr  r1

	;Configuracion de puertos y timers

    ; PORTD como salida total
    ldi  tmp, 0xFF
    out  DDRD, tmp

    ;---- TIMER2: CTC, prescaler=8
    ldi  tmp, (1<<WGM21)
    sts  TCCR2A, tmp          ; usar STS (I/O extendido)

    ldi  tmp, (1<<CS21)
    sts  TCCR2B, tmp          ; usar STS

    ldi  tmp, OCR2A_VAL
    sts  OCR2A, tmp           ; usar STS

    ldi  tmp, (1<<OCIE2A)
    sts  TIMSK2, tmp          ; usar STS (0x70)

    clr  idx

    sei

MAIN:
    rjmp MAIN

;------------------- ISR TIMER2 COMPA -------------------
TIMER2_COMPA_ISR:
    push r0
    push r1
    push r16
    push r17
    push ZH
    push ZL

    ldi  ZL, low(LUT*2)
    ldi  ZH, high(LUT*2)
    add  ZL, idx
    brcc NoCarry
    inc  ZH
NoCarry:

    ; r0 <- LUT[idx] desde FLASH
    lpm  r0, Z

    ; Sacar a PORTD (I/O bajo -> OUT válido)
    out  PORTD, r0

    inc  idx
    cpi  idx, LUT_LEN
    brlo NextOK
    clr  idx
NextOK:

    pop  ZL
    pop  ZH
    pop  r17
    pop  r16
    pop  r1
    pop  r0
    reti

;------------------- LUT de la señal (prueba con senosoidal) -------------
LUT:
    .db 0x80,0x8C,0x98,0xA5,0xB0,0xBC,0xC6,0xD0,0xDA,0xE2,0xEA,0xF0,0xF5,0xFA,0xFD,0xFE
    .db 0xFF,0xFE,0xFD,0xFA,0xF5,0xF0,0xEA,0xE2,0xDA,0xD0,0xC6,0xBC,0xB0,0xA5,0x98,0x8C
    .db 0x80,0x73,0x67,0x5A,0x4F,0x43,0x39,0x2F,0x25,0x1D,0x15,0x0F,0x0A,0x05,0x02,0x01
    .db 0x00,0x01,0x02,0x05,0x0A,0x0F,0x15,0x1D,0x25,0x2F,0x39,0x43,0x4F,0x5A,0x67,0x73

;mmmmmm MANGO
