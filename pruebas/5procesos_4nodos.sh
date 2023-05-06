#!/bin/bash

cd ..
./ejecutar.sh #Compilamos el programa y eliminamos lass cosas inecesarias

cd procesos_bin

for i in {1..4}
do
    ./recibir $i 4 & 
done

for j in {1..5}
do 
    for i in {1..4}
    do
        ./pagos $i &
    done
done

echo -e "\n\nPrueba realizada\n"
