#!/bin/bash

killall -9 master_p 2>/dev/null
killall -9 slave_p 2>/dev/null

make clean
make

echo "Starting Master:"
./master_p pipe_master &

echo "Starting Slave 1:"
./slave_p pipe_master 5 &
echo "Starting Slave 2:"
./slave_p pipe_master 5 &
echo "Starting Slave 3:"
./slave_p pipe_master 5 &

sleep 30
killall -9 master_p
killall -9 slave_p
echo "END"