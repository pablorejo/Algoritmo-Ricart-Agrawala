- [Algoritmo-Ricart-Agrawala](#algoritmo-ricart-agrawala)
  - [Herramientas](#herramientas)
  - [ERRORES](#errores)
    - [MEMORIA COMPARTIDA](#memoria-compartida)
    - [BUZONES](#buzones)
# Algoritmo-Ricart-Agrawala

En esta práctica hablaremos sobre el algoritmo `Ricart-Agrawala` para proteger secciones seciones críticas de procesos que están distribuidos

## Herramientas

Vamos a usar varios tipos de herramientas con las que se debe tener una cierta base 
- Hilos
- Paso de mensajes
- Semáforos


## ERRORES
### MEMORIA COMPARTIDA
En caso de que no funcione bien los semaforos o la memoria compartida
Comprobar si la memoria compartida ya existe
```bash
ipcs -m
```
En caso de que exista la que ya tenemos creada la eliminamos con el comando:
```bash
ipcrm -m ID_MEMORIA_COMPARTIDA
```
El id de la memoria compartida se ve con el anterior comando y a ejecutar algun programa te lo muestra por pantalla

### BUZONES 
Es parecido a con memoria compartida. Tenemos que comprobar si existe el buzon y en ese caso eliminarlo
Este comando nos devuelve una lista con los buzones que estan en el sisteme. No debería haber ninguno
```bash
ipcs -q
```

El comando para eliminarlos será:
```bash
ipcrm -q ID_BUZON
```
Al igual que antes el id del buzon lo imprime el programa recibir


<!-- # Funcionamiento
## Recibir
El proceso recivir recibirá el mensaje
```c

``` -->