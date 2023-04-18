#!/bin/bash
cd procesos
gcc recibir.c -o recibir -Wall
gcc proceso.c -o proceso -Wall
gcc pagos.c -o pagos -Wall
gcc anulacion.c -o anulacion -Wall
gcc administracion.c -o administracion -Wall
gcc reservas.c -o reservas -Wall
gcc consultas.c -o consultas -Wall

mv * ../procesos_bin

cd ../procesos_bin
rm *.c