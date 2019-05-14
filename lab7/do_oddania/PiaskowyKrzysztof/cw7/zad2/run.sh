#!/bin/bash

make clean
make loader
make trucker

killall -9 trucker
killall -9 loader

echo "Start:"
gnome-terminal -- /bin/bash -c "./trucker 10 10 10 30; read"
sleep 6
./loader 2 1 10
echo "End"

#rm /dev/shm/* && clear && make clean && make trucker && ./trucker 10 10 10 30
