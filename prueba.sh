#!/bin/bash
./ejecutar.sh #Compilamos el programa y eliminamos las cosas inecesarias

cd procesos_bin

rm *.txt

echo -e "\nRealizando prueba con $1 procesos por nodo con $2 nodos\n\n"


# Ejecutamos as funcios de recibir
num_nodos=$2
for i in $(seq 1 $num_nodos)
do
    ./recibir $i $num_nodos & 
done


# Damos un tempo prudencial para que se ejecuten todas
sleep 1



# Ejecutamso todos os procesos $1 veces en $2 nodos
for j in $(seq 1 $num_nodos)
do 
    
  for i in $(seq 1 $1)
  do
    
    ./pagos $j & ./pagos $j & ./pagos $j & ./administracion $j & ./consultas $j &
    
  done
done


#Esperamos  a que terminen los procesos
echo -n "Procesando todo: ["
for i in {1..60}; do
  echo -n "#"
  sleep $3
done
echo "] Hecho!"



# Eliminamos los posibles procesos que no han terminado
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


# hacemos el CTRL+C de los procesos recibir
pids=$(pgrep -f "recibir")
if [ -z "$pids" ]; then
  echo "No hay procesos en ejecución de los tipos 'pagos', 'administracion', 'anulacion' , 'reservas' o 'consultas'."
else
  # Eliminar los procesos
  echo "ctrl procesos..."
  for i in $pids then 
  do
    kill -s SIGINT $i #Enviamos la señal de que 
    sleep 0.3
    echo "$i cancelado"
  done
  echo "Procesos Cancelados."
fi


# Esperamos a que terminen todos los procesos que hemos creado
wait



## Para guardar la informacion en carpetas
for i in $(seq 1 $num_nodos)
do
  rm -r "Nodo_${i}"
  mkdir "Nodo_${i}"
  echo "Nodo_${i}"
  mv datos_nodo_${i}_* Nodo_${i}
done

# Movemos esa carpeta a la carpeta Nodos_NUMERO_DE_NODOS
rm -r "Nodos_${i}"
mkdir Nodos_$2
mv Nodo* Nodos_$2


rm -r ../datos/Nodos_$2
mv Nodos_$2 ../datos


cd ..

./media.sh $2
paplay alert.wav
