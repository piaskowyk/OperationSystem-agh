#!/usr/bin/env bash
make clean
make

mkdir tmp
touch ./tmp/a.txt
touch ./tmp/b.txt

echo "./tmp/a.txt 2" >> lista.txt
echo "./tmp/b.txt 4" >> lista.txt

echo -e "\e[32mStarting tester program\e[39m"
./tester ./tmp/a.txt 1 7 10 &
./tester ./tmp/b.txt 2 9 10 &

echo -e "\e[32mStarting main with mode 0\e[39m"
./main ./lista.txt 15 0 20 10
echo -e "\e[32mStarting main with mode 1\e[39m"
./main ./lista.txt 15 1 10 10

echo ""
echo "----------------------------------------------------------"
echo ""

echo -e "\e[32mStarting main with mode 0 - with restric limit\e[39m"
./main ./lista.txt 15 0 10 1
echo -e "\e[32mStarting main with mode 1 - with restric limit\e[39m"
./main ./lista.txt 15 1 1 1

killall -9 tester