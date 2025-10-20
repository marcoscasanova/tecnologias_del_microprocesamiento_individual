#include <Wire.h> // Librería para comunicación I2C
#include <LiquidCrystal_I2C.h> // Librería para controlar LCD por I2C
#include <Keypad.h> // Librería para manejar el teclado matricial
#include <EEPROM.h> // Librería para almacenar datos en memoria EEPROM

#define DIR_LCD 0x27 // Dirección I2C del módulo LCD
LiquidCrystal_I2C lcd(DIR_LCD, 16, 2); // Se crea el objeto LCD con dirección y tamaño 16x2

const int PIN_ZUMBADOR   = 13; // Pin conectado al buzzer
const int PIN_LED_VERDE  = 11; // Pin conectado al LED verde
const int PIN_LED_ROJO   = 12; // Pin conectado al LED rojo

const byte FILAS = 4, COLUMNAS = 4; // Tamaño del teclado matricial 4x4
char teclas[FILAS][COLUMNAS] = { // Mapa de teclas
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}
};
byte pinesFilas[FILAS] = {9, 8, 7, 6}; // Pines conectados a las filas R1–R4
byte pinesColumnas[COLUMNAS] = {5, 4, 3, 2}; // Pines conectados a las columnas C1–C4
Keypad teclado = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS); // Se crea el objeto teclado

#define MIN_LARGO     4 // Longitud mínima de la clave
#define MAX_LARGO     6 // Longitud máxima de la clave
#define MAX_INTENTOS  3 // Máximo número de intentos fallidos

#define EEPROM_DIR_MAGIC  0 // Dirección EEPROM para valor mágico
#define EEPROM_DIR_LARGO  1 // Dirección EEPROM para longitud de la clave
#define EEPROM_DIR_DATOS  2 // Dirección EEPROM donde empieza la clave guardada
#define EEPROM_MAGIC      0xA5 // Valor mágico para verificar si hay clave guardada

// Función para mostrar mensajes en el LCD
void lcdMensaje(const char* linea1, const char* linea2="") {
	lcd.clear(); // Limpia el LCD
	lcd.setCursor(0,0); lcd.print(linea1); // Muestra la primera línea
	lcd.setCursor(0,1); lcd.print(linea2); // Muestra la segunda línea (si existe)
}

// Función para generar un bip con el buzzer
void bip(uint16_t ms=80) {
	tone(PIN_ZUMBADOR, 2000, ms); // Emite tono de 2 kHz por el tiempo indicado
	delay(ms + 5); // Pequeño retardo adicional
}

// Función para encender LED verde durante cierto tiempo
void ledVerde(uint16_t ms=500) {
	digitalWrite(PIN_LED_VERDE, HIGH); // Enciende el LED verde
	delay(ms); // Espera
	digitalWrite(PIN_LED_VERDE, LOW); // Apaga el LED verde
}

// Función para encender LED rojo durante cierto tiempo
void ledRojo(uint16_t ms=500) {
	digitalWrite(PIN_LED_ROJO, HIGH); // Enciende el LED rojo
	delay(ms); // Espera
	digitalWrite(PIN_LED_ROJO, LOW); // Apaga el LED rojo
}

// Función para verificar si ya existe una clave válida en la EEPROM
bool eepromTieneClave() {
	uint8_t magia = EEPROM.read(EEPROM_DIR_MAGIC); // Se lee el valor mágico
	uint8_t largo = EEPROM.read(EEPROM_DIR_LARGO); // Se lee la longitud almacenada
	return (magia == EEPROM_MAGIC && largo >= MIN_LARGO && largo <= MAX_LARGO); // Se valida integridad
}

// Función para guardar una clave en EEPROM
void eepromGuardarClave(const char* clave, uint8_t largo) {
	EEPROM.write(EEPROM_DIR_MAGIC, EEPROM_MAGIC); // Se guarda el valor mágico
	EEPROM.write(EEPROM_DIR_LARGO, largo); // Se guarda la longitud de la clave
	for (uint8_t i=0; i<largo; i++) {
		EEPROM.write(EEPROM_DIR_DATOS + i, (uint8_t)clave[i]); // Se guarda cada carácter
	}
}

// Función para cargar la clave guardada desde EEPROM
void eepromCargarClave(char* claveSalida, uint8_t &largoSalida) {
	largoSalida = EEPROM.read(EEPROM_DIR_LARGO); // Se obtiene la longitud guardada
	for (uint8_t i=0; i<largoSalida; i++) {
		claveSalida[i] = (char)EEPROM.read(EEPROM_DIR_DATOS + i); // Se lee cada carácter
	}
	claveSalida[largoSalida] = '\0'; // Se agrega terminador de cadena
}

// Función para ingresar una clave desde el teclado
bool ingresarClave(const char* titulo, char* salida, uint8_t &largoSalida) {
	largoSalida = 0; // Inicializa longitud en cero
	memset(salida, 0, MAX_LARGO+1); // Limpia el buffer de salida

	lcdMensaje(titulo, "OK: #"); // Muestra mensaje inicial

	while (true) { // Bucle hasta validar entrada
		char t = teclado.getKey(); // Lee una tecla
		if (!t) { delay(10); continue; } // Si no hay tecla, espera y sigue

		if (t >= '0' && t <= '9') { // Si la tecla es un número
			if (largoSalida < MAX_LARGO) {
				salida[largoSalida++] = t; // Se almacena el número ingresado
			}
			} else if (t == '#') { // Si se presiona “#”, se confirma
			if (largoSalida >= MIN_LARGO && largoSalida <= MAX_LARGO) {
				bip(80); // Emite sonido de confirmación
				return true; // Retorna indicando éxito
				} else {
				lcdMensaje("Longitud invalida", "Debe ser 4 a 6"); // Mensaje de error
				bip(150); delay(700);
				lcdMensaje(titulo, "OK: #"); // Se restablece pantalla
				largoSalida = 0; salida[0] = '\0'; // Se reinicia la entrada
			}
		}

		lcd.setCursor(0,1); // Se posiciona en segunda línea
		for (uint8_t i=0; i<largoSalida; i++) lcd.print('*'); // Se muestra asteriscos por cada dígito
		for (uint8_t i=largoSalida; i<16; i++) lcd.print(' '); // Se limpia el resto de la línea
	}
}

// Función para comparar dos claves
bool clavesIguales(const char* a, uint8_t la, const char* b, uint8_t lb) {
	if (la != lb) return false; // Si los largos son distintos, no coinciden
	for (uint8_t i=0; i<la; i++) if (a[i] != b[i]) return false; // Se compara carácter por carácter
	return true; // Si todos coinciden, retorna verdadero
}

// Función de alarma tras 3 intentos fallidos
void alarma() {
	lcdMensaje("ALARMA! 3 fallos", "Bloqueo temporal"); // Mensaje de alerta
	for (uint8_t i=0; i<20; i++) { // Se repite 20 veces
		tone(PIN_ZUMBADOR, 2000, 200); // Emite tono corto
		digitalWrite(PIN_LED_ROJO, HIGH); // Enciende LED rojo
		delay(200);
		digitalWrite(PIN_LED_ROJO, LOW); // Apaga LED
		delay(50);
	}
	lcdMensaje("Intente nuevamente", ""); // Mensaje final
	delay(800);
}

// Flujo de cambio de clave
void flujoCambioClave() {
	char claveGuardada[MAX_LARGO + 1]; // Buffer para clave almacenada
	uint8_t largoGuardada = 0; // Variable para longitud guardada
	eepromCargarClave(claveGuardada, largoGuardada); // Carga la clave existente

	char claveActual[MAX_LARGO + 1]; // Buffer para clave actual
	uint8_t largoActual = 0;
	if (!ingresarClave("Clave actual:", claveActual, largoActual)) return; // Solicita clave actual

	if (!clavesIguales(claveActual, largoActual, claveGuardada, largoGuardada)) { // Si no coincide
		lcdMensaje("Clave incorrecta", ""); // Muestra error
		ledRojo(700); bip(150);
		delay(800);
		return;
	}

	char claveNueva[MAX_LARGO + 1]; // Buffer para nueva clave
	uint8_t largoNueva = 0;
	if (!ingresarClave("Nueva clave:", claveNueva, largoNueva)) return; // Solicita nueva clave

	char claveConfirma[MAX_LARGO + 1]; // Buffer para confirmación
	uint8_t largoConfirma = 0;
	if (!ingresarClave("Confirmar clave", claveConfirma, largoConfirma)) return; // Solicita confirmación

	if (!clavesIguales(claveNueva, largoNueva, claveConfirma, largoConfirma)) { // Si no coinciden
		lcdMensaje("No coincide", "Intente otra vez");
		ledRojo(600); bip(150);
		delay(900);
		return;
	}

	eepromGuardarClave(claveNueva, largoNueva); // Guarda la nueva clave
	lcdMensaje("Clave guardada", "");
	ledVerde(800); // Señal visual de éxito
	delay(900);
}

// Menú principal después del acceso
void menuPrincipal() {
	lcdMensaje("Bienvenido", "A:Cambiar  B:Salir"); // Muestra opciones
	while (true) {
		char t = teclado.getKey(); // Lee una tecla
		if (!t) { delay(10); continue; }
		if (t == 'A') {
			flujoCambioClave(); // Cambiar clave
			lcdMensaje("Bienvenido", "A:Cambiar  B:Salir");
			} else if (t == 'B' || t == '#') {
			return; // Salir del menú
		}
	}
}

// Función para solicitar clave y verificar acceso
bool solicitarYVerificar() {
	char claveIngresada[MAX_LARGO + 1];
	uint8_t largoIngresada = 0;
	if (!ingresarClave("Ingrese clave:", claveIngresada, largoIngresada)) return false; // Solicita clave

	char claveGuardada[MAX_LARGO + 1];
	uint8_t largoGuardada = 0;
	eepromCargarClave(claveGuardada, largoGuardada); // Carga la clave guardada

	if (clavesIguales(claveIngresada, largoIngresada, claveGuardada, largoGuardada)) { // Si coincide
		lcdMensaje("Acceso concedido", "");
		ledVerde(600); bip(80);
		delay(600);
		return true;
		} else { // Si no coincide
		lcdMensaje("Clave incorrecta", "");
		ledRojo(500); bip(120);
		delay(600);
		return false;
	}
}

// Evento que suena cada vez que se presiona una tecla
void eventoTeclado(KeypadEvent t) {
	if (teclado.getState() == PRESSED) { // Si se detecta presión de tecla
		tone(PIN_ZUMBADOR, 2000, 40); // Se emite un bip corto
	}
}

// Configuración inicial del sistema
void setup() {
	pinMode(PIN_LED_VERDE, OUTPUT); // LED verde como salida
	pinMode(PIN_LED_ROJO, OUTPUT); // LED rojo como salida
	pinMode(PIN_ZUMBADOR, OUTPUT); // Buzzer como salida
	digitalWrite(PIN_LED_VERDE, LOW); // Se apaga LED verde
	digitalWrite(PIN_LED_ROJO, LOW); // Se apaga LED rojo

	lcd.init(); // Inicializa el LCD
	lcd.backlight(); // Enciende la retroiluminación

	if (!eepromTieneClave()) { // Si no hay clave guardada
		const char* porDefecto = "1234"; // Se define clave por defecto
		eepromGuardarClave(porDefecto, 4); // Se guarda en EEPROM
	}

	teclado.addEventListener(eventoTeclado); // Se asocia evento de teclado

	lcdMensaje("Cerradura lista", "OK: #"); // Mensaje de inicio
	delay(800);
}

// Bucle principal del programa
void loop() {
	lcdMensaje("Ingrese clave:", "OK: #"); // Pantalla de inicio
	uint8_t intentos = 0; // Contador de intentos fallidos

	while (true) {
		bool ok = solicitarYVerificar(); // Se solicita y verifica clave
		if (ok) { // Si es correcta
			menuPrincipal(); // Ingresa al menú principal
			lcdMensaje("Bloqueado", "OK: #");
			delay(700);
			intentos = 0; // Se reinicia contador
			} else { // Si es incorrecta
			intentos++;
			if (intentos >= MAX_INTENTOS) { // Si supera el límite
				alarma(); // Activa alarma
				intentos = 0;
				lcdMensaje("Ingrese clave:", "OK: #");
			}
		}
	}
}
