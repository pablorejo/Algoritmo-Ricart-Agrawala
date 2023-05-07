archivo = "datos.txt"  # Ruta del archivo de datos
suma = 0
contador = 0

# Abrir el archivo y leer los datos
with open(archivo, "r") as f:
    for linea in f:
        dato = float(linea.strip())  # Convertir la línea en un número de punto flotante
        suma += dato
        contador += 1

# Calcular la media
media = suma / contador

# Imprimir el resultado
print(f"La media es: {media}")
