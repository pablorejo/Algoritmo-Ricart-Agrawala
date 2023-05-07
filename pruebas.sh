#!/bin/bash

for p in 4 10 20 40 80 100; do
  for n in 5 20 100 200 400 1000; do
    ./prueba.sh $n $p
    cd datos
    dir="prueba_nodos_${p}_procesos_${n}"
    mkdir $dir
    mv *.txt $dir
    cd ..
  done
done
