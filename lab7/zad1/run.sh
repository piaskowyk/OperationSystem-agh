#!/bin/bash

make clean
make loader
make trucker

mkdir mem
touch mem/MEM_LINE
touch mem/MEM_LINE_PARAM
touch mem/SEM
killall -9 trucker
killall -9 loader

echo "Start:"
gnome-terminal -- /bin/bash -c "./trucker 10 10 10 30; read"
sleep 6
./loader 2 1 10
echo "End"