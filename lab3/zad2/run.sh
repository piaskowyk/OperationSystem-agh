#!/usr/bin/env bash

make clean
make

mkdir tmp
touch ./tmp/a.txt
touch ./tmp/b.txt

echo "./tmp/a.txt 2" >> lista.txt
echo "./tmp/b.txt 4" >> lista.txt

echo "Starting tester program"
./tester ./tmp/a.txt 1 5 10 &
./tester ./tmp/b.txt 1 5 10 &

echo "Starting main with mode 0"
./main ./lista.txt 60 0
echo "Starting main with mode 1"
./main ./lista.txt 60 1

killall -9 tester