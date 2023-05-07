#!/bin/bash
./ejecutar.sh #Compilamos el programa y eliminamos lass cosas inecesarias

cd procesos_bin

rm *.txt

echo -e "\nRealizando prueba con $1 procesos por nodo con $2 nodos\n\n"

num_nodos=$2
for i in $(seq 1 $num_nodos)
do
    ./recibir $i $num_nodos & 
done


for j in $(seq 1 $num_nodos)
do 
    
  for i in $(seq 1 $1)
  do
      ./pagos $j &
      ./administracion $j &
      ./consultas $j &
  done
done


echo -n "Procesando todo: ["
for i in {1..60}; do
  echo -n "#"
  sleep $3
done
echo "] Hecho!"

#Espera a que terminen todos menos los de recibir


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

  echo "Procesos Cancelados."
fi


wait



## Para guardar la informacion en carpetas
for i in $(seq 1 $num_nodos)
do
  rm -r "Nodo_${i}"
  mkdir "Nodo_${i}"
  echo "Nodo_${i}"
  mv datos_nodo_${i}_* Nodo_${i}
done

rm -r "Nodos_${i}"
mkdir Nodos_$2
mv Nodo* Nodos_$2


rm -r ../datos/Nodos_$2
mv Nodos_$2 ../datos


cd ..
paplay alert.wav



