#!/bin/bash

for n in 4 20 50; do  ### Estos son los nodos
  for p in {1..5}; do ### Estos son los procesos

    ./prueba.sh ${5**p} $n
    
  done
done
