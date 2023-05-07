

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

