#!/bin/bash

echo -e "\nRealizando prueba con $1 procesos por nodo con $2 nodos\n\n"

cd ..
./ejecutar.sh #Compilamos el programa y eliminamos lass cosas inecesarias

cd procesos_bin

for i in {1..$2}
do
    ./recibir $i 4 & 
done

for j in {1..$1}
do 
    for i in {1..$2}
    do
        ./pagos $i &
    done
done

echo -e "\n\nPrueba realizada\n"
