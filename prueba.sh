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
echo "Esperando a que empiecen los procesos recibir"
sleep 1


# Ejecutamso todos os procesos $1 veces en $2 nodos
# echo -n "Ejecutando procesos: ["
# for j in $(seq 1 $num_nodos)
# do 
    
#   for i in $(seq 1 $1)
#   do
#     ./pagos $j & ./administracion $j & ./consultas $j &
#   done
#   echo -n "#"
# done
# echo "] Hecho!"


echo -n "Ejecutando procesos: ["
for j in $(seq 1 $1)
do 
    
  for i in $(seq 1 $num_nodos)
  do
    ./pagos $i & ./pagos $i & ./pagos $i & ./pagos $i & ./pagos $i & ./administracion $i & ./consultas $i &
  done
  echo -n "#"
done
echo "] Hecho!"


# # Esperamos  a que terminen los procesos
# echo -n "Procesando todo: ["
# for i in {1..60}; do
#   echo -n "#"
#   sleep $3
# done
# echo "] Hecho!"

wait $(pgrep -f "administracion|reservas")




# Eliminamos los posibles procesos que no han terminado
pids=$(pgrep -f "pagos|administracion|anulaciones|reservas|consultas")

if [ -z "$pids" ]; then
  echo "No hay procesos en ejecución de los tipos 'pagos', 'administracion', 'anulacion' , 'reservas' o 'consultas'."
else
  

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
  done
  echo "Procesos Cancelados."
fi


# Esperamos a que terminen todos los procesos que hemos creado
wait


## Para guardar la informacion en carpetas
for i in $(seq 1 $num_nodos)
do
  dir=Nodo_${i}

  if [ -f "$dir" ]; then
    rm -r "$dir"
  fi
  mkdir "$dir"
  mv datos_nodo_${i}_* $dir
done


# Movemos esa carpeta a la carpeta Nodos_NUMERO_DE_NODOS
if [ -f "Nodos_${i}/Procesos_$1" ]; then
  rm -r "Nodos_${i}/Procesos_$1"
fi


mkdir Nodos_$2

cd Nodos_$2

mkdir Procesos_$1

cd ..

mv Nodo* Nodos_$2/Procesos_$1


if [ -f "../datos/Nodos_$2" ]; then
  rm -r ../datos/Nodos_$2
fi

mv Nodos_$2 ../datos


cd ..




./media.sh $2 $1

# Guarda
dir2=datos/Nodos_$2

python3 media.py $dir2/Procesos_$1/datos_nodos_${2}_pagos_anulacioens.txt > $dir2/datos_nodos_${2}.txt
python3 media.py $dir2/Procesos_$1/datos_nodos_${2}_administracion_reservas.txt >> $dir2/datos_nodos_${2}.txt
python3 media.py $dir2/Procesos_$1/datos_nodos_${2}_consultas.txt  >> $dir2/datos_nodos_${2}.txt

paplay alert.wav
exit -1