#!/bin/bash
directorio=procesos_bin

mkdir $directorio

cd procesos

gcc recibir.c   -o recibir -Wall

gcc pagos.c -o pagos -Wall
cp pagos anulaciones

gcc administracion.c -o administracion -Wall
cp administracion reservas

# gcc consultas.c -o consultas -Wall

mv * ../$directorio

cd ../$directorio
 
mv *.c  ../procesos
mv *.h  ../procesos

cd ..

./del_procesos.sh
./del_mem.sh