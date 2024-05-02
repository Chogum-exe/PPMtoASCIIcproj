#!/bin/sh
cc -o main imgTOascii.c
for file in *.ppm
do
  ./main $file > ${file%.*}.txt
done