#!/bin/bash

make clean
make
echo ""
echo "-----------------------------------------------------------"
echo ""
./server &
./client
# ./client &

sleep 2
# kill $(pidof server)
# kill $(pidof client)

echo ""
killall -9 server
killall -9 client