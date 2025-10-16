#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

/* ===== UART ======== */
static void uart_init(uint32_t baud){
    uint16_t ubrr = (uint16_t)((F_CPU / (16UL*baud)) - 1UL);
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr & 0xFF);
    UCSR0A = 0;
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); // 8N1
}
static inline uint8_t uart_available(void){ return (UCSR0A & (1<<RXC0)); }
static inline uint8_t uart_read(void){ return UDR0; }
static void uart_write(uint8_t c){ while(!(UCSR0A & (1<<UDRE0))); UDR0 = c; }
static void uart_print(const char* s){ while(*s){ uart_write((uint8_t)*s++); } }
static void uart_println(const char* s){ uart_print(s); uart_write('\r'); uart_write('\n'); }

/* ======= PWM (Timer1 OC1A = PB1/D9) ========== */
static inline void pwm_init(void){
    DDRB  |= (1<<PB1);                       // PB1 salida
    TCCR1A = (1<<WGM11);
    TCCR1B = (1<<WGM13) | (1<<WGM12);        // Fast PWM, TOP=ICR1
    TCCR1A &= ~((1<<COM1A1)|(1<<COM1A0));    // OC1A desconectado
    TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10)); // timer parado
    ICR1 = 0; OCR1A = 0;
    PORTB &= ~(1<<PB1);
}
static inline void pwm_off(void){
    TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10));
    TCCR1A &= ~((1<<COM1A1)|(1<<COM1A0));
    OCR1A = 0;
    PORTB &= ~(1<<PB1);
}
static inline void pwm_set_freq(uint16_t f_hz){
    if(!f_hz){ pwm_off(); return; }
    uint32_t top = (F_CPU/(8UL*(uint32_t)f_hz)) - 1UL; // prescaler 8
    if(top > 65535UL) top = 65535UL;
    ICR1  = (uint16_t)top;
    OCR1A = (uint16_t)(top/2);              // ≈50% duty
    TCCR1A |=  (1<<COM1A1);                 // conectar OC1A
    TCCR1A &= ~(1<<COM1A0);
    TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10));
    TCCR1B |=  (1<<CS11);                   // prescaler 8
}

/* ============== Frecuencias =========== */
/* Piano (8 notas) */
#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523

/* Canciones */
#define REST   0
#define A3   220
#define AS3  233
#define B3   247

#define C4   262
#define CS4  277
#define D4   294
#define DS4  311
#define E4   330
#define F4   349
#define FS4  370
#define G4   392
#define GS4  415
#define A4   440
#define AS4  466
#define B4   494

#define C5   523
#define CS5  554
#define D5   587
#define DS5  622
#define E5   659
#define F5   698
#define FS5  740
#define G5   784
#define GS5  831
#define A5   880
#define AS5  932
#define B5   988

#define C6   1046
#define D6   1175
#define E6   1319
#define F6   1397
#define G6   1568
#define A6   1760
#define AS6  1865  
#define B6   1976

#define C7   2093
#define D7   2349
#define E7   2637
#define F7   2794
#define G7   3136
#define A7   3520
#define B7   3951

/* ========== Duraciones por BPM ============= */
/* OJITO: se tuvieron que renombra para evitar problemas con <math.h> (M_E) */
#define MARIO_BPM  200
#define MAR_Q   ((uint16_t)(60000UL/MARIO_BPM))
#define MAR_E   ((uint16_t)(30000UL/MARIO_BPM))
#define MAR_S   ((uint16_t)(15000UL/MARIO_BPM))
#define MAR_ED  ((uint16_t)(45000UL/MARIO_BPM))
#define MAR_QD  ((uint16_t)(90000UL/MARIO_BPM))

#define TETRIS_BPM  180
#define TET_Q   ((uint16_t)(60000UL/TETRIS_BPM))
#define TET_E   ((uint16_t)(30000UL/TETRIS_BPM))
#define TET_S   ((uint16_t)(15000UL/TETRIS_BPM))
#define TET_QD  ((uint16_t)(90000UL/TETRIS_BPM))

/* == Canciones == */
typedef struct { uint16_t f; uint16_t ms; } note_t;

/* Super Mario Bros-Clasic (~15 s) */
static const note_t MARIO[] = {
    {E7,MAR_E},{E7,MAR_E},{REST,MAR_E},{E7,MAR_E},{REST,MAR_E},{C7,MAR_E},{E7,MAR_E},{REST,MAR_E},
    {G7,MAR_E},{REST,MAR_Q},{G6,MAR_E},{REST,MAR_Q},
    {C7,MAR_E},{REST,MAR_Q},{G6,MAR_E},{REST,MAR_Q},{E6,MAR_E},{REST,MAR_Q},
    {A6,MAR_E},{REST,MAR_E},{B6,MAR_E},{REST,MAR_E},{AS6,MAR_E},{A6,MAR_E},
    {G6,MAR_E},{E7,MAR_E},{G7,MAR_E},{A7,MAR_E},{REST,MAR_E},{F7,MAR_E},{G7,MAR_E},{REST,MAR_E},
    {E7,MAR_E},{REST,MAR_E},{C7,MAR_E},{D7,MAR_E},{B6,MAR_E},{REST,MAR_Q},
    {C7,MAR_E},{REST,MAR_Q},{G6,MAR_E},{REST,MAR_Q},{E6,MAR_E},{REST,MAR_Q},
    {A6,MAR_E},{REST,MAR_E},{B6,MAR_E},{REST,MAR_E},{AS6,MAR_E},{A6,MAR_E},
    {G6,MAR_E},{E7,MAR_E},{G7,MAR_E},{A7,MAR_E},{REST,MAR_Q}
};
static const uint16_t MARIO_LEN = sizeof(MARIO)/sizeof(MARIO[0]);

/* Tetris (~15 s) */
static const note_t TETRIS[] = {
    // Frase A
    {E5,TET_E},{B4,TET_E},{C5,TET_E},{D5,TET_E},{C5,TET_E},{B4,TET_E},{A4,TET_E},{A4,TET_E},
    {C5,TET_E},{E5,TET_E},{D5,TET_E},{C5,TET_E},{B4,TET_E},{B4,TET_E},{REST,TET_E},
    // Frase B
    {C5,TET_E},{D5,TET_E},{E5,TET_E},{C5,TET_E},{D5,TET_E},{E5,TET_E},{FS5,TET_E},{D5,TET_E},
    {E5,TET_Q},{C5,TET_E},{A4,TET_E},{A4,TET_Q},
    // Frase C
    {D5,TET_E},{F5,TET_E},{A5,TET_E},{G5,TET_E},{F5,TET_E},{E5,TET_E},{C5,TET_E},{E5,TET_E},
    {D5,TET_Q},{C5,TET_E},{B4,TET_E},{B4,TET_Q},
    // Remate
    {C5,TET_E},{D5,TET_E},{E5,TET_E},{C5,TET_E},{D5,TET_E},{E5,TET_E},{FS5,TET_E},{D5,TET_E},
    {E5,TET_QD},{REST,TET_E}
};
static const uint16_t TETRIS_LEN = sizeof(TETRIS)/sizeof(TETRIS[0]);

/* ======== Botones (Piano) =========== */
static void keys_init(void){
    // PC0..PC3 (A0..A3) entradas con pull-up
    DDRC  &= ~((1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3));
    PORTC |=  ((1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3));
    // PD2..PD5 (D2..D5) entradas con pull-up
    DDRD  &= ~((1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5));
    PORTD |=  ((1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5));
}
static uint16_t read_note(void){
    if(!(PINC & (1<<PC0))) return NOTE_C4; // Do
    if(!(PINC & (1<<PC1))) return NOTE_D4; // Re
    if(!(PINC & (1<<PC2))) return NOTE_E4; // Mi
    if(!(PINC & (1<<PC3))) return NOTE_F4; // Fa
    if(!(PIND & (1<<PD2))) return NOTE_G4; // Sol
    if(!(PIND & (1<<PD3))) return NOTE_A4; // La
    if(!(PIND & (1<<PD4))) return NOTE_B4; // Si
    if(!(PIND & (1<<PD5))) return NOTE_C5; // Do5
    return 0;
}

/* ====== Estados / Modos ========= */
typedef enum { MODE_PIANO=0, MODE_SONG1=1, MODE_SONG2=2, MODE_PAUSE=3 } mode_t;
static volatile mode_t g_mode = MODE_PIANO;   // arranca en PIANO

/* --- Prototipo --- */
static void handle_uart_byte(uint8_t b);

/* ========= Delay atento a UART ======= */
static inline void dly_ms_check(uint16_t ms, uint8_t in_song){
    while(ms--){
        if(uart_available()) handle_uart_byte(uart_read());
        if(in_song && !(g_mode==MODE_SONG1 || g_mode==MODE_SONG2)) return; // cortar si cambió
        if(g_mode==MODE_PAUSE) pwm_off();
        _delay_ms(1);
    }
}

static inline void play_note_ms(uint16_t f, uint16_t ms, uint8_t in_song){
    if(f==REST || ms==0){ pwm_off(); dly_ms_check(ms, in_song); return; }
    if(in_song && !(g_mode==MODE_SONG1 || g_mode==MODE_SONG2)) return;
    pwm_set_freq(f);
    if(ms > 6){
        dly_ms_check(ms - 4, in_song);
        pwm_off();
        dly_ms_check(4, in_song);
    }else{
        dly_ms_check(ms, in_song);
        pwm_off();
    }
}
static void play_song_for_ms(const note_t *song, uint16_t len, uint32_t total_ms){
    uint32_t elapsed = 0;
    while(elapsed < total_ms && (g_mode==MODE_SONG1 || g_mode==MODE_SONG2)){
        for(uint16_t i=0; i<len && elapsed<total_ms; i++){
            if(!(g_mode==MODE_SONG1 || g_mode==MODE_SONG2)) break;
            uint16_t d = song[i].ms;
            if(elapsed + d > total_ms) d = (uint16_t)(total_ms - elapsed);
            play_note_ms(song[i].f, d, 1);
            if(!(g_mode==MODE_SONG1 || g_mode==MODE_SONG2)) break;
            elapsed += d;
        }
    }
    pwm_off();
}

/* ============================ Comandos de la UART ============================ */
static void handle_uart_byte(uint8_t b){
    static uint8_t prev = 0;
    if(b=='H' || b=='h'){
        uart_println("Comandos: P=piano | 1/C1=Mario | 2/C2=Tetris | S=pause | H=help");
    }else if(b=='P' || b=='p'){
        g_mode = MODE_PIANO; uart_println("[Modo] Piano");
    }else if(b=='1'){
        g_mode = MODE_SONG1; uart_println("[Modo] Cancion 1 (Mario)");
    }else if(b=='2'){
        g_mode = MODE_SONG2; uart_println("[Modo] Cancion 2 (Tetris)");
    }else if(b=='S' || b=='s'){
        g_mode = MODE_PAUSE; pwm_off(); uart_println("[Modo] Pausa");
    }else if(prev=='C' || prev=='c'){
        if(b=='1'){ g_mode = MODE_SONG1; uart_println("[Modo] Cancion 1 (Mario)"); }
        else if(b=='2'){ g_mode = MODE_SONG2; uart_println("[Modo] Cancion 2 (Tetris)"); }
        prev = 0;
        return;
    }
    prev = b;
}

/* ============== MAIN ============ */
int main(void){
    uart_init(9600);
    pwm_init();
    keys_init();

    g_mode = MODE_PIANO;
    uart_println("UART 9600 8N1 listo");
    uart_println("P=piano | 1/C1=Mario | 2/C2=Tetris | S=pause | H=help");
    uart_println("Modo inicial: Piano");

    while(1){
        if(uart_available()) handle_uart_byte(uart_read());

        if(g_mode == MODE_PIANO){
            uint16_t f = read_note();
            if(f){
                _delay_ms(8); // debounce
                if(f == read_note()){
                    pwm_set_freq(f);
                    while(read_note()==f){
                        if(uart_available()){
                            handle_uart_byte(uart_read());
                            if(g_mode != MODE_PIANO) break;
                        }
                        _delay_ms(2);
                    }
                    pwm_off();
                }
            }else{
                pwm_off();
            }
        }else if(g_mode == MODE_SONG1){
            play_song_for_ms(MARIO, MARIO_LEN, 15000UL);
        }else if(g_mode == MODE_SONG2){
            play_song_for_ms(TETRIS, TETRIS_LEN, 15000UL);
        }else{ // PAUSE
            pwm_off();
            _delay_ms(2);
        }
    }
}
