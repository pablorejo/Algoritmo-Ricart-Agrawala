#!/bin/bash
./ejecutar.sh #Compilamos el programa y eliminamos lass cosas inecesarias

cd procesos_bin

echo -e "\nRealizando prueba con $1 procesos por nodo con $2 nodos\n\n"

num_nodos=$2
for i in $(seq 1 $num_nodos)
do
    ./recibir $i $num_nodos & 
done

for j in $(seq 1 $1)
do 
    for i in $(seq 1 $num_nodos)
    do
        ./pagos $i &
    done
done


pids=$(pgrep -f "pagos|administracion|anulaciones|reservas|consultas")

if [ -z "$pids" ]; then
  echo "No hay procesos en ejecución de los tipos 'pagos', 'administracion', 'anulacion' , 'reservas' o 'consultas'."
else
  echo "Procesos encontrados:"
  echo "$pids"

  # Eliminar los procesos
  echo "Eliminando procesos..."
  kill $pids

  echo "Procesos eliminados."
fi


pids=$(pgrep -f "recibir")




if [ -z "$pids" ]; then
  echo "No hay procesos en ejecución de los tipos 'pagos', 'administracion', 'anulacion' , 'reservas' o 'consultas'."
else
  echo "Procesos encontrados:"
  echo "$pids"

  # Eliminar los procesos
  echo "ctrl procesos..."
  kill -s SIGINT $pids #Enviamos la señal de que 

  echo "Procesos eliminados."
fi

sleep 2
mv *.txt ../datos




