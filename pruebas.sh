#!/bin/bash

for p in 4 20 40 100; do  ### Estos son los nodos
  for n in 5 20 200 1000; do ### Estos son los procesos
    ./prueba.sh $n $p
    cd datos
    dir1="${p}_NODOS"
    mkdir $dir1
    cd $dir1
    dir2="prueba_nodos_${p}_procesos_${n}"
    mkdir $dir2
    cd ..
    mv *.txt $dir1/$dir2
    cd ..
  done
done
