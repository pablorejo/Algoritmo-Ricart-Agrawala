#!/bin/bash
gcc main.c -o main -Wall
./main $1 $2


# for ((i=1;i<=$1;i++))
# do
#     ./main $i $1
# done