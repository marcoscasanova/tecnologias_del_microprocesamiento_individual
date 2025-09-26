; ====================================================================
; ATmega328P @ 16 MHz
; Standby (D10) al inicio. 'A' -> ciclo infinito (modo 1/2/3 por USART).
; Secuencia por modo: AVANCE -> PAUSA -> PUZONADO -> DESCARGA -> PARADA(1s)
; Ligera:  AV 3s, PA 2s, PZ 4s, DC 3s
; Media:   AV 4s, PA 2s, PZ 5s, DC 4s
; Pesada:  AV 5s, PA 3s, PZ 6s, DC 3s
; Fases (pines):
;  AVANCE:    D12=1, D3=0, D11=0, D2=1, D9=1
;  PAUSA:     D12=0, D3=0, D11=0, D2=1, D9=1        ; <-- D2 HIGH (inactiva)
;  PUZONADO:  D11=1, D2=0, D12=0, D3=0, D9=1
;  DESCARGA:  D12=0, D3=1, D11=0, D2=1, D9=1
;  PARADA:    D12=0, D11=0, D8=1, D9=0 (1s), D2=1, D3=0  ; <-- D2 HIGH
; Timer0 ~10 ms.
; ====================================================================

.include "m328pdef.inc"

; -------------------- Pines --------------------
.equ avancePin     = PB4      ; D12
.equ pausaPin      = PB3      ; D11  (puzonadora)
.equ standbyPin    = PB2      ; D10
.equ activoPin     = PB1      ; D9
.equ finalizadoPin = PB0      ; D8

.equ cargaLigera   = PD7      ; D7
.equ cargaMedia    = PD6      ; D6
.equ cargaPesada   = PD5      ; D5
.equ auxD2         = PD2      ; D2 (activa LOW para puzonado)
.equ auxD3         = PD3      ; D3 (dirección cinta: LOW=avance, HIGH=descarga)

; -------------------- RAM -------------------------
.dseg
started:   .byte 1
mode_cur:  .byte 1
tAV_10:    .byte 2
tPA_10:    .byte 2
tPZ_10:    .byte 2
tDC_10:    .byte 2
.cseg

.org 0x0000
    rjmp RESET

; -------------------- RESET -----------------------
RESET:
    ldi  r16, high(RAMEND)
    out  SPH, r16
    ldi  r16, low(RAMEND)
    out  SPL, r16

    sbi  DDRB, avancePin
    sbi  DDRB, pausaPin
    sbi  DDRB, standbyPin
    sbi  DDRB, activoPin
    sbi  DDRB, finalizadoPin

    cbi  PORTB, avancePin
    cbi  PORTB, pausaPin
    cbi  PORTB, activoPin
    cbi  PORTB, finalizadoPin
    sbi  PORTB, standbyPin     ; D10=1 (espera)

    ldi  r16, (1<<cargaLigera)|(1<<cargaMedia)|(1<<cargaPesada)|(1<<auxD2)|(1<<auxD3)
    out  DDRD, r16
    ldi  r16, 0x00
    out  PORTD, r16            ; D2=LOW, D3=LOW al inicio (sin efecto hasta 'A')

    ; USART 9600 8N1
    ldi  r16, high(103)
    sts  UBRR0H, r16
    ldi  r16, low(103)
    sts  UBRR0L, r16
    ldi  r16, (1<<RXEN0)|(1<<TXEN0)
    sts  UCSR0B, r16
    ldi  r16, (1<<UCSZ01)|(1<<UCSZ00)
    sts  UCSR0C, r16

    ; Estado por defecto (Ligera)
    ldi  r16, 0
    sts  started, r16
    ldi  r16, 1
    sts  mode_cur, r16
    ; AV 3s, PA 2s, PZ 4s, DC 3s  (ticks de 10 ms)
    ldi  r24, low(300)   ; AV
    ldi  r25, high(300)
    sts  tAV_10,  r24
    sts  tAV_10+1,r25
    ldi  r24, low(200)   ; PA
    ldi  r25, high(200)
    sts  tPA_10,  r24
    sts  tPA_10+1,r25
    ldi  r24, low(400)   ; PZ
    ldi  r25, high(400)
    sts  tPZ_10,  r24
    sts  tPZ_10+1,r25
    ldi  r24, low(300)   ; DC
    ldi  r25, high(300)
    sts  tDC_10,  r24
    sts  tDC_10+1,r25

; -------------------- MAIN ------------------------
MAIN:
    lds  r17, UCSR0A
    sbrs r17, RXC0
    rjmp _no_rx
    lds  r18, UDR0

    lds  r16, started
    cpi  r16, 0
    brne _after_A

    cpi  r18, 'A'
    breq CMD_A_FIRST
    rjmp _no_rx

CMD_A_FIRST:
    ldi  r16, 1
    sts  started, r16
    cbi  PORTB, standbyPin
    sbi  PORTB, activoPin
    sbi  PORTD, cargaLigera
    cbi  PORTD, cargaMedia
    cbi  PORTD, cargaPesada
    rjmp _no_rx

_after_A:
    cpi  r18, '1'
    breq CMD_1
    cpi  r18, '2'
    breq CMD_2
    cpi  r18, '3'
    breq CMD_3
    rjmp _no_rx

; ---------- Tiempos por modo ----------
CMD_1:
    ldi  r16, 1
    sts  mode_cur, r16
    sbi  PORTD, cargaLigera
    cbi  PORTD, cargaMedia
    cbi  PORTD, cargaPesada
    ldi  r24, low(300)   ; AV
    ldi  r25, high(300)
    sts  tAV_10,  r24
    sts  tAV_10+1,r25
    ldi  r24, low(200)   ; PA
    ldi  r25, high(200)
    sts  tPA_10,  r24
    sts  tPA_10+1,r25
    ldi  r24, low(400)   ; PZ
    ldi  r25, high(400)
    sts  tPZ_10,  r24
    sts  tPZ_10+1,r25
    ldi  r24, low(300)   ; DC
    ldi  r25, high(300)
    sts  tDC_10,  r24
    sts  tDC_10+1,r25
    rjmp _no_rx

CMD_2:
    ldi  r16, 2
    sts  mode_cur, r16
    cbi  PORTD, cargaLigera
    sbi  PORTD, cargaMedia
    cbi  PORTD, cargaPesada
    ldi  r24, low(400)   ; AV
    ldi  r25, high(400)
    sts  tAV_10,  r24
    sts  tAV_10+1,r25
    ldi  r24, low(200)   ; PA
    ldi  r25, high(200)
    sts  tPA_10,  r24
    sts  tPA_10+1,r25
    ldi  r24, low(500)   ; PZ
    ldi  r25, high(500)
    sts  tPZ_10,  r24
    sts  tPZ_10+1,r25
    ldi  r24, low(400)   ; DC
    ldi  r25, high(400)
    sts  tDC_10,  r24
    sts  tDC_10+1,r25
    rjmp _no_rx

CMD_3:
    ldi  r16, 3
    sts  mode_cur, r16
    cbi  PORTD, cargaLigera
    cbi  PORTD, cargaMedia
    sbi  PORTD, cargaPesada
    ldi  r24, low(500)   ; AV
    ldi  r25, high(500)
    sts  tAV_10,  r24
    sts  tAV_10+1,r25
    ldi  r24, low(300)   ; PA
    ldi  r25, high(300)
    sts  tPA_10,  r24
    sts  tPA_10+1,r25
    ldi  r24, low(600)   ; PZ
    ldi  r25, high(600)
    sts  tPZ_10,  r24
    sts  tPZ_10+1,r25
    ldi  r24, low(300)   ; DC
    ldi  r25, high(300)
    sts  tDC_10,  r24
    sts  tDC_10+1,r25
    rjmp _no_rx

_no_rx:
    lds  r16, started
    cpi  r16, 1
    breq DO_CYCLE
    rjmp MAIN

; -------------------- Ciclo continuo -------------------
DO_CYCLE:
    ; ===== AVANCE =====  (D12=1, D3=0, D11=0, D2=1, D9=1)
    sbi  PORTB, activoPin
    cbi  PORTB, finalizadoPin
    sbi  PORTB, avancePin        ; D12=1
    cbi  PORTB, pausaPin         ; D11=0
    cbi  PORTD, auxD3            ; D3=0
    sbi  PORTD, auxD2            ; D2=1 (inactiva puzonadora)
    lds  r24, tAV_10
    lds  r25, tAV_10+1
    rcall wait_Nx10ms_T0

    ; ===== PAUSA =====   (D12=0, D3=0, D11=0, D2=1, D9=1)  ; D2 HIGH
    cbi  PORTB, avancePin
    cbi  PORTB, pausaPin
    cbi  PORTD, auxD3
    sbi  PORTD, auxD2            ; D2=1 (inactiva)
    lds  r24, tPA_10
    lds  r25, tPA_10+1
    rcall wait_Nx10ms_T0

    ; ===== PUZONADO ==== (D11=1, D2=0, D12=0, D3=0, D9=1)
    cbi  PORTB, avancePin
    sbi  PORTB, pausaPin         ; D11=1
    cbi  PORTD, auxD2            ; D2=0 (activa)
    cbi  PORTD, auxD3            ; D3=0
    lds  r24, tPZ_10
    lds  r25, tPZ_10+1
    rcall wait_Nx10ms_T0

    ; ===== DESCARGA ==== (D12=0, D3=1, D11=0, D2=1, D9=1)
    cbi  PORTB, pausaPin
    cbi  PORTB, avancePin
    sbi  PORTD, auxD3            ; D3=1
    sbi  PORTD, auxD2            ; D2=1 (inactiva)
    lds  r24, tDC_10
    lds  r25, tDC_10+1
    rcall wait_Nx10ms_T0

    ; ===== PARADA 1s ==== (D8=1, D9=0, D12=0, D11=0, D2=1, D3=0)
    cbi  PORTB, avancePin
    cbi  PORTB, pausaPin
    cbi  PORTB, activoPin
    sbi  PORTB, finalizadoPin
    sbi  PORTD, auxD2            ; D2=1 (inactiva)
    cbi  PORTD, auxD3            ; D3=0
    ldi  r24, low(100)           ; 1 s = 100 * 10 ms
    ldi  r25, high(100)
    rcall wait_Nx10ms_T0
    cbi  PORTB, finalizadoPin
    sbi  PORTB, activoPin

    rjmp MAIN

; --------------- Delays con TIMER0 -----------------
wait_10ms_T0:
    ldi  r16, 0
    out  TCCR0B, r16
    ldi  r16, (1<<TOV0)
    out  TIFR0, r16
    ldi  r16, 0
    out  TCNT0, r16
    ldi  r16, (1<<CS01)|(1<<CS00)
    out  TCCR0B, r16
    ldi  r20, 10
w10_loop:
    in   r21, TIFR0
    sbrs r21, TOV0
    rjmp w10_loop
    ldi  r21, (1<<TOV0)
    out  TIFR0, r21
    dec  r20
    brne w10_loop
    ldi  r16, 0
    out  TCCR0B, r16
    ret

wait_Nx10ms_T0:
    mov  r22, r24
    or   r22, r25
    breq wN_end
wN_loop:
    rcall wait_10ms_T0
    sbiw r24, 1
    brne wN_loop
wN_end:
    ret

