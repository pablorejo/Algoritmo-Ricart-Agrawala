#!/bin/bash
directorio=procesos_bin

mkdir $directorio
gcc recibir.c -o recibir -Wall
mv recibir procesos_bin
cd procesos
gcc pagos.c -o pagos -Wall
# gcc anulaciones.c -o anulaciones -Wall
# gcc administracion.c -o administracion -Wall
gcc reservas.c -o reservas -Wall
# gcc consultas.c -o consultas -Wall

mv * ../$directorio

cd ../$directorio
 
mv *.c  ../procesos

cd ..