#!/bin/bash

killall -9 sender
killall -9 catcher

make clean
make

echo -e "\e[32mStarting test for signal communicztion via kill():\e[39m"
echo "Starting catcher"
./catcher 1 &
catcherPid=$(pidof catcher)
echo "Starting sender"
./sender $catcherPid 10 1
killall -9 catcher

echo ""
echo "----------------------------------------------------"
echo ""

echo -e "\e[32mStarting test for signal communication via sigqueue():\e[39m"
echo "Starting catcher"
./catcher 2 &
catcherPid=$(pidof catcher)
echo "Starting sender"
./sender $catcherPid 10 2
killall -9 catcher

echo ""
echo "----------------------------------------------------"
echo ""

echo -e "\e[32mStarting test for signal communication via RT signals:\e[39m"
echo "Starting catcher"
./catcher 3 &
catcherPid=$(pidof catcher)
echo "Starting sender"
./sender $catcherPid 10 3
killall -9 catcher

echo ""
echo "THE END OD TESTS"