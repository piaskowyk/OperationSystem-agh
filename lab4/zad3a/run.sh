!/usr/bin/env bash

make clean
make

echo -e "\e[32mStarting test for signal communicztion via kill():\e[39m"
echo "Starting catcher"
./catcher 1 &
catcherPid=$(pidof catcher)
echo "Starting sender"
./sender $catcherPid 50 1
killall -9 catcher