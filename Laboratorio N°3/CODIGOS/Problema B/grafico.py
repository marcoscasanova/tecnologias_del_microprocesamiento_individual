import serial  # Se importa la librería 'serial' para establecer la comunicación por puerto serie con el microcontrolador
import matplotlib.pyplot as plt  # Se importa 'matplotlib' para graficar los datos en tiempo real
import time  # Se importa 'time' para gestionar retardos y medir tiempos
import re  # Se importa 're' para el uso de expresiones regulares en el filtrado de los datos recibidos
import threading  # Se importa 'threading' para permitir la ejecución simultánea de tareas (lectura y consola)

PUERTO = 'COM5'  # Se define el puerto serie donde está conectado el microcontrolador
BAUDRATE = 9600  # Se define la velocidad de transmisión en baudios
TIEMPO_MUESTREO = 0.5  # Se define el intervalo de muestreo entre lecturas sucesivas (en segundos)

temps, pms, pwms, acciones, tiempos = [], [], [], [], []  # Se crean listas vacías para almacenar temperatura, punto medio, PWM, acción y tiempo
punto_medio = 26  # Se establece el punto medio inicial de referencia
running = True  # Se define una bandera de control que mantiene la ejecución del programa activa

def enviar_punto_medio(ser, nuevo_pm):
    global punto_medio
    ser.reset_input_buffer()  # Se limpia el buffer de entrada del puerto serie
    ser.write(b'x')  # Se envía el carácter 'x' indicando solicitud de cambio de punto medio
    time.sleep(0.3)  # Se agrega un pequeño retardo para sincronizar la respuesta del microcontrolador
    limite_espera = time.time() + 2.0  # Se establece un tiempo máximo de espera de respuesta
    recibido = b""  # Se inicializa un buffer para almacenar la respuesta recibida

    while time.time() < limite_espera:  # Se espera hasta recibir el mensaje solicitado o agotar el tiempo límite
        if ser.in_waiting:  # Si hay datos disponibles en el buffer de entrada
            recibido += ser.read(ser.in_waiting)  # Se leen todos los bytes disponibles
            if b"Ingrese nuevo valor" in recibido:  # Si se detecta la cadena esperada
                break  # Se interrumpe el bucle
        time.sleep(0.05)  # Se agrega una breve pausa antes de volver a leer

    ser.write(f"{nuevo_pm}\r".encode())  # Se envía el nuevo valor del punto medio con retorno de carro
    time.sleep(0.5)  # Se espera para garantizar la correcta recepción
    punto_medio = nuevo_pm  # Se actualiza el valor global del punto medio
    print(f"\nPunto medio {nuevo_pm} enviado correctamente.\n")  # Se notifica en consola el envío exitoso

def hilo_consola(ser):
    global running
    while running:  # Se mantiene la lectura de comandos mientras el programa esté activo
        cmd = input().strip().lower()  # Se lee y normaliza el comando ingresado por el usuario

        if cmd == 'x':  # Si el comando es 'x', se solicita cambiar el punto medio
            try:
                nuevo_pm = int(input("Ingrese nuevo punto medio (10–50): "))  # Se solicita un valor numérico al usuario
                if 10 <= nuevo_pm <= 50:  # Se valida que esté dentro del rango permitido
                    enviar_punto_medio(ser, nuevo_pm)  # Se envía el nuevo valor al microcontrolador
                else:
                    print("Valor fuera de rango (10–50).")  # Se muestra un mensaje de error si no cumple el rango
            except ValueError:
                print("Valor inválido.")  # Se muestra un mensaje si la entrada no es numérica

        elif cmd in ['q', 'exit', 'salir']:  # Si el usuario desea salir del programa
            running = False  # Se cambia la bandera principal para detener la ejecución
            print("Finalizando por solicitud del usuario...")  # Se notifica en consola
            break  # Se interrumpe el bucle de la consola

def pwm_por_accion(accion):
    accion = accion.lower()  # Se convierte la cadena a minúsculas para uniformar las comparaciones
    if "bajo" in accion: return 85  # Devuelve 85 si la acción corresponde a un nivel bajo
    if "medio" in accion: return 170  # Devuelve 170 si la acción corresponde a un nivel medio
    if "alto" in accion: return 255  # Devuelve 255 si la acción corresponde a un nivel alto
    return 0  # Devuelve 0 si no se identifica ninguna acción conocida

ser = serial.Serial(PUERTO, BAUDRATE, timeout=1)  # Se inicializa la comunicación serial con los parámetros definidos
time.sleep(2)  # Se espera 2 segundos para permitir el reinicio del microcontrolador

print("Control de Temperatura - ATmega328P")  # Se muestra un encabezado informativo en consola
print("Comandos:")  
print("  x  → Cambiar punto medio (10–50)")  # Se explica el comando 'x'
print("  q  → Salir\n")  # Se explica el comando 'q' para salir del programa

threading.Thread(target=hilo_consola, args=(ser,), daemon=True).start()  # Se inicia un hilo paralelo para escuchar comandos en consola

plt.ion()  # Se activa el modo interactivo para actualización dinámica de la gráfica
fig, ax1 = plt.subplots(figsize=(8,4))  # Se crea una figura con dimensiones de 8x4 pulgadas
line_temp, = ax1.plot([], [], 'r-', label='Temperatura [°C]')  # Se define la línea roja para la temperatura
line_pm,   = ax1.plot([], [], 'orange', linestyle='--', label='Punto Medio')  # Se define la línea discontinua naranja para el punto medio
ax1.set_xlabel('Tiempo [s]')  # Se etiqueta el eje X
ax1.set_ylabel('Temperatura [°C]', color='r')  # Se etiqueta el eje Y izquierdo
ax1.set_ylim(0, 70)  # Se fija el rango vertical del eje Y entre 0 y 70 °C
ax1.tick_params(axis='y', labelcolor='r')  # Se colorean las etiquetas del eje Y de rojo
ax1.grid(True)  # Se activa la cuadrícula en la gráfica

ax2 = ax1.twinx()  # Se crea un segundo eje Y para mostrar el valor PWM
line_pwm,  = ax2.plot([], [], 'g--', label='PWM')  # Se define la línea verde discontinua correspondiente al PWM
ax2.set_ylabel('PWM [0–255]', color='g')  # Se etiqueta el eje Y derecho
ax2.set_ylim(0, 255)  # Se fija el rango de valores del PWM
ax2.tick_params(axis='y', labelcolor='g')  # Se colorean las etiquetas del eje Y derecho en verde

lines = [line_temp, line_pm, line_pwm]  # Se agrupan las líneas de datos
labels = [l.get_label() for l in lines]  # Se obtienen las etiquetas de cada línea
ax1.legend(lines, labels, loc='upper right')  # Se agrega la leyenda en la esquina superior derecha

t0 = time.time()  # Se guarda el instante de inicio para calcular tiempos relativos
print("Leyendo datos en tiempo real...\n")  # Se informa al usuario que comenzó la lectura continua

try:
    while running:  # Se ejecuta el bucle principal mientras la bandera esté activa
        linea = ser.readline().decode('latin-1', errors='ignore').strip()  # Se lee una línea del puerto serie y se decodifica
        if linea:  # Si la línea contiene datos válidos
            match = re.search(r"Temp:(\d+)C\s*\|\s*PM:(\d+)\s*\|\s*(.*)", linea)  # Se busca el patrón con temperatura, punto medio y acción
            if match:
                temp = int(match.group(1))  # Se obtiene la temperatura en °C
                pm = int(match.group(2))  # Se obtiene el valor de punto medio
                accion = match.group(3).strip()  # Se obtiene la acción (por ejemplo, "Bajo", "Medio", "Alto")
                pwm = pwm_por_accion(accion)  # Se calcula el PWM correspondiente según la acción
                t = time.time() - t0  # Se calcula el tiempo transcurrido desde el inicio

                temps.append(temp)  # Se almacena la temperatura en la lista
                pms.append(pm)  # Se almacena el punto medio
                pwms.append(pwm)  # Se almacena el valor de PWM
                acciones.append(accion)  # Se guarda la descripción de la acción
                tiempos.append(t)  # Se guarda el tiempo relativo

                line_temp.set_data(tiempos, temps)  # Se actualiza la línea de temperatura
                line_pm.set_data(tiempos, pms)  # Se actualiza la línea de punto medio
                line_pwm.set_data(tiempos, pwms)  # Se actualiza la línea de PWM

                ax1.relim()  # Se recalculan los límites del eje izquierdo
                ax1.autoscale_view()  # Se actualiza la vista según los nuevos valores
                ax2.relim()  # Se recalculan los límites del eje derecho
                ax2.autoscale_view()  # Se actualiza la vista del eje derecho
                plt.pause(0.001)  # Se actualiza la gráfica en pantalla

                print(f"Temperatura={temp:2d}°C | PM={pm:2d} | PWM={pwm:3d} | Acción={accion}")  # Se muestra el estado actual en consola

        time.sleep(TIEMPO_MUESTREO)  # Se respeta el intervalo de muestreo antes de la siguiente lectura

except KeyboardInterrupt:
    print("Lectura finalizada por el usuario.")  # Se notifica si el programa se detiene manualmente con Ctrl+C

finally:
    running = False  # Se cambia la bandera para finalizar la ejecución
    ser.close()  # Se cierra el puerto serie
    plt.ioff()  # Se desactiva el modo interactivo de matplotlib
    plt.show()  # Se muestra la gráfica final antes de salir
