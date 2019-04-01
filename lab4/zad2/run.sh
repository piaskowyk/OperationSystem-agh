#!/usr/bin/env bash

make clean
make

mkdir tmp
touch ./tmp/a.txt
touch ./tmp/b.txt

echo "./tmp/a.txt 2" >> lista.txt
echo "./tmp/b.txt 4" >> lista.txt

echo "Starting main with mode 1"
./main ./lista.txt