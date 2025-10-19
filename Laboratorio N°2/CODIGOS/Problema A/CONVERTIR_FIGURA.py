import re # Expresiones regulares
import sys # Argumentos de línea de comandos

# Mapeo de funciones a códigos
MAPA_DIRECCIONES = {
    "PLOTTER_DERECHA": "D",
    "PLOTTER_IZQUIERDA": "I",
    "PLOTTER_ARRIBA": "U",
    "PLOTTER_ABAJO": "A",
    "PLOTTER_SUBIR": "S",
    "PLOTTER_BAJAR": "B",
    "PLOTTER_DERECHA_NO_BAJAR": "d",
    "PLOTTER_IZQUIERDA_NO_BAJAR": "i",
    "PLOTTER_ABAJO_NO_BAJAR": "a",
    "PLOTTER_ARRIBA_NO_BAJAR": "u"
}

# Convierte una línea de código a formato de paso
def convertir_linea(linea):
    match = re.search(r'(PLOTTER_[A-Z_]+).*?_delay_ms\((\d+)\)', linea) # Busca función y tiempo
    # Si no hay coincidencia, retorna None
    if not match:
        return None
    funcion, tiempo = match.groups() # Extrae función y tiempo
    # Verifica si la función está en el mapa
    if funcion not in MAPA_DIRECCIONES:
        print(f"[ADVERTENCIA] Dirección desconocida: {funcion}") # Función no mapeada
        return None # Retorna None
    codigo = MAPA_DIRECCIONES[funcion] # Obtiene el código correspondiente
    return f"{{'{codigo}', {tiempo}}}" # Formatea el paso

# Convierte un archivo de entrada a un archivo de salida
def convertir_archivo(entrada, salida):
    with open(entrada, "r", encoding="utf-8") as f: # Abre archivo de entrada
        lineas = f.readlines() # Lee todas las líneas

    pasos = [] # Lista para almacenar los pasos convertidos
    for linea in lineas: # Itera sobre cada línea
        paso = convertir_linea(linea) # Convierte la línea
        if paso: # Si se obtuvo un paso válido
            pasos.append(paso) # Agrega el paso a la lista

    # Escribe los pasos convertidos en el archivo de salida
    with open(salida, "w", encoding="utf-8") as out:
        out.write("const Paso figura[] PROGMEM = {\n    ")

        for i, p in enumerate(pasos): # Itera sobre los pasos convertidos
            out.write(p) # Escribe el paso
            # Escribe la coma y el salto de línea si es necesario
            if i != len(pasos) - 1:
                out.write(", ") # Coma entre pasos
            # Añade un salto de línea cada 10 pasos para mejor legibilidad
            if (i + 1) % 10 == 0 and i != len(pasos) - 1:
                out.write("\n    ") # Salto de línea y sangría

        out.write("\n};\n") # Cierra la definición del array

    print(f"Conversión completa. {len(pasos)} pasos generados → {salida}")

# Punto de entrada del script
if __name__ == "__main__":
    # Verifica argumentos de línea de comandos
    if len(sys.argv) < 3: 
        print("Uso: python convertir_figura.py <entrada.c> <salida.c>") # Muestra uso correcto
    else:
        convertir_archivo(sys.argv[1], sys.argv[2]) # Llama a la función de conversión con los archivos proporcionados
