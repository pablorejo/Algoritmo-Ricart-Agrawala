#!/bin/bash

dir=datos/Nodos_$1

rm $dir/*.txt
for i in $(seq 1 $1)
do
  python3 media.py $dir/Nodo_$i//datos_nodo_${i}_pagos_anulacioens.txt >> $dir/datos_nodos_${1}_pagos_anulacioens.txt
  python3 media.py $dir/Nodo_$i//datos_nodo_${i}_administracion_reservas.txt >> $dir/datos_nodos_${1}_administracion_reservas.txt
  python3 media.py $dir/Nodo_$i//datos_nodo_${i}_consultas.txt  >> $dir/datos_nodos_${1}_consultas.txt
  python3 media.py $dir/Nodo_$i//datos_nodo_${i}_administracion_reservas.txt
done
