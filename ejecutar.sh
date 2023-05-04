#!/bin/bash
directorio=procesos_bin

rm -r 
mkdir $directorio
gcc recibir.c -o recibir -Wall
mv recibir procesos_bin
cd procesos
gcc pagos.c -o pagos -Wall
cp pagos anulaciones

gcc administracion.c -o administracion -Wall
cp administracion reservas

# gcc consultas.c -o consultas -Wall

mv * ../$directorio

cd ../$directorio
 
mv *.c  ../procesos

cd ..