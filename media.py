import sys

# Comprobamos si se ha pasado el fichero como argumento
if len(sys.argv) < 2:
    print("Error: No se ha especificado el fichero")
    sys.exit(1)

# Abrimos el fichero en modo lectura
try:
    with open(sys.argv[1], 'r') as f:
        # Leemos todas las líneas del fichero y las convertimos a números
        numeros = list(map(float, f.readlines()))
except FileNotFoundError:
    print("Error: No se ha encontrado el fichero")
    sys.exit(1)
except ValueError:
    print("Error: El fichero no contiene números válidos")
    sys.exit(1)

# Calculamos la media de los números
if (len(numeros) != 0):
    media = sum(numeros) / len(numeros)
else:
    media = 0 ## Estos procesos no terminaron su ejecucion y no sabemos el motivo

# Imprimimos la media
print(media)
