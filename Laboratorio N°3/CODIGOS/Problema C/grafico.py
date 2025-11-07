import serial  # Se importa la librería 'serial' para establecer comunicación por puerto serie con el microcontrolador
import matplotlib.pyplot as plt  # Se importa la librería 'matplotlib' para graficar los datos en tiempo real
import time  # Se importa la librería 'time' para manejar tiempos y retardos
import re  # Se importa la librería 're' para utilizar expresiones regulares en el filtrado de datos

PUERTO = 'COM5'  # Se define el puerto serie donde está conectado el microcontrolador
BAUDRATE = 9600  # Se define la velocidad de comunicación en baudios
TIEMPO_MUESTREO = 0.1  # Se define el intervalo de muestreo (en segundos) entre cada lectura

ser = serial.Serial(PUERTO, BAUDRATE, timeout=1)  # Se inicializa la comunicación serie con los parámetros definidos
time.sleep(2)  # Se espera 2 segundos para permitir que el microcontrolador se reinicie y estabilice la conexión

refs, acts, pwms, tiempos, sentidos = [], [], [], [], []  # Se inicializan listas vacías para almacenar los datos recibidos: referencias, valores actuales, PWM, tiempos y sentido de giro

plt.ion()  # Se habilita el modo interactivo de matplotlib para actualizar la gráfica en tiempo real
fig, ax = plt.subplots(figsize=(8,4))  # Se crea una figura y un eje con tamaño 8x4 pulgadas
line_ref, = ax.plot([], [], 'r-', label='Potenciometro 1')  # Se crea la línea roja correspondiente al potenciómetro 1 (referencia)
line_act, = ax.plot([], [], 'b-', label='Potenciometro 2')  # Se crea la línea azul correspondiente al potenciómetro 2 (valor actual)
line_pwm, = ax.plot([], [], 'g--', label='PWM')  # Se crea la línea verde discontinua correspondiente al valor PWM
ax.set_xlabel('Tiempo [s]')  # Se etiqueta el eje X como “Tiempo [s]”
ax.set_ylabel('Valor ADC / PWM')  # Se etiqueta el eje Y como “Valor ADC / PWM”
ax.set_ylim(0, 1050)  # Se establece el rango del eje Y entre 0 y 1050 (valores esperados del ADC)
ax.legend(loc='upper right')  # Se coloca la leyenda en la esquina superior derecha del gráfico
ax.grid(True)  # Se activa la cuadrícula del gráfico para mejor visualización

t0 = time.time()  # Se toma el tiempo inicial de referencia para calcular los tiempos relativos de muestreo
print("Leyendo datos... (Ctrl+C para detener)\n")  # Se muestra un mensaje indicando el inicio de la lectura en tiempo real

try:
    while True:  # Bucle principal de ejecución continua
        linea = ser.readline().decode(errors='ignore').strip()  # Se lee una línea desde el puerto serie y se decodifica eliminando espacios y errores
        if linea:  # Si la línea contiene datos válidos
            # Se busca el patrón de datos usando una expresión regular con formato: Ref:### | Act:### | PWM:### | Sent:Texto
            match = re.search(r"Ref:(\d+)\s*\|\s*Act:(\d+)\s*\|\s*PWM:(\d+)\s*\|\s*Sent:([A-Za-z]+)", linea)
            if match:  # Si se encuentra coincidencia con el formato esperado
                ref = int(match.group(1))  # Se obtiene el valor del potenciómetro de referencia
                act = int(match.group(2))  # Se obtiene el valor del potenciómetro actual
                pwm = int(match.group(3))  # Se obtiene el valor del PWM
                sentido = match.group(4)  # Se obtiene el estado o sentido del motor (por ejemplo: “Horario”, “Antihorario”)
                t = time.time() - t0  # Se calcula el tiempo transcurrido desde el inicio

                # Se almacenan los datos obtenidos en sus respectivas listas
                refs.append(ref)
                acts.append(act)
                pwms.append(pwm)
                tiempos.append(t)
                sentidos.append(sentido)

                # Se actualizan los datos de las líneas del gráfico con las nuevas lecturas
                line_ref.set_data(tiempos, refs)
                line_act.set_data(tiempos, acts)
                line_pwm.set_data(tiempos, pwms)

                # Se reajustan los límites de los ejes para adaptarse a los nuevos datos
                ax.relim()
                ax.autoscale_view()

                plt.pause(0.001)  # Se actualiza la gráfica con una pequeña pausa para refrescar la visualización

                # Se imprime en consola el valor de cada parámetro recibido
                print(f"Potenciometro 1={ref:4d} | Potenciometro 2={act:4d} | PWM={pwm:3d} | Estado={sentido}")

        time.sleep(TIEMPO_MUESTREO)  # Se espera el tiempo de muestreo antes de la siguiente lectura

except KeyboardInterrupt:  # Si el usuario interrumpe el programa con Ctrl+C
    print("\nLectura finalizada por el usuario.")  # Se muestra un mensaje indicando la finalización manual

finally:
    ser.close()  # Se cierra el puerto serie para liberar el recurso
    plt.ioff()  # Se desactiva el modo interactivo de matplotlib
    plt.show()  # Se mantiene visible la última gráfica al finalizar el programa
