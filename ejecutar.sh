#!/bin/bash
gcc recibir.c -o recibir -Wall
cd procesos
gcc pagos.c -o pagos -Wall
gcc anulaciones.c -o anulaciones -Wall
gcc administracion.c -o administracion -Wall
gcc reservas.c -o reservas -Wall
gcc consultas.c -o consultas -Wall

mv * ../procesos_bin

cd ../procesos_bin
 
mv *.c  ../procesos

cd ..