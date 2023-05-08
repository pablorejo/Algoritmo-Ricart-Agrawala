#!/bin/bash
# $1 recibira el numero total de nodos y $2 se recibirá el numero total de procesos

dir=datos/Nodos_$1/Procesos_$2


direcctorio=$dir/*.txt
if [ -f "$direcctorio" ]; then
  rm $dir/*.txt
fi

echo -n "Haciendo la meidas: ["
for i in $(seq 1 $1)
do
  python3 media.py $dir/Nodo_$i/datos_nodo_${i}_pagos_anulacioens.txt >> $dir/datos_nodos_${1}_pagos_anulacioens.txt
  python3 media.py $dir/Nodo_$i/datos_nodo_${i}_administracion_reservas.txt >> $dir/datos_nodos_${1}_administracion_reservas.txt
  python3 media.py $dir/Nodo_$i/datos_nodo_${i}_consultas.txt  >> $dir/datos_nodos_${1}_consultas.txt
  echo -n "#"
done
echo "] Hecho!"
