#!/bin/bash
gcc main.c -o main -Wall
for ((i=1;i<=$1;i++))
do
    ./main $1
done