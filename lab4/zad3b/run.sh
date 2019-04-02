#!/bin/bash

killall -9 sender
killall -9 catcher

clear
make clean
make

echo -e "\e[32mStarting test for signal communication via kill():\e[39m"
echo "Starting catcher"
./catcher 1 &
catcherPid=$(pidof catcher)
echo "Starting sender"
./sender $catcherPid 3 1

echo ""
echo "----------------------------------------------------"
echo ""

echo -e "\e[32mStarting test for signal communication via sigqueue():\e[39m"
echo "Starting catcher"
./catcher 2 &
catcherPid=$(pidof catcher)
echo "Starting sender"
./sender $catcherPid 3 2

echo ""
echo "----------------------------------------------------"
echo ""

echo -e "\e[32mStarting test for signal communication via RT signals:\e[39m"
echo "Starting catcher"
./catcher 3 &
catcherPid=$(pidof catcher)
echo "Starting sender"
./sender $catcherPid 3 3

echo ""
echo "THE END OD TESTS"