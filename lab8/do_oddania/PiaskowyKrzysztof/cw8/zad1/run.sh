#!/bin/bash

make clean
make main

echo "Start testing:"
echo ""
echo "Thread count: 1, type: BLOCK"
./main 1 block ./image.pgm ./filter.txt ./result.pgm
echo "Thread count: 2, type: BLOCK"
./main 2 block ./image.pgm ./filter.txt ./result.pgm
echo "Thread count: 4, type: BLOCK"
./main 4 block ./image.pgm ./filter.txt ./result.pgm
echo "Thread count: 8, type: BLOCK"
./main 8 block ./image.pgm ./filter.txt ./result.pgm
echo "Thread count: 1, type: INTERLEAVED"
./main 1 interleaved ./image.pgm ./filter.txt ./result.pgm
echo "Thread count: 2, type: INTERLEAVED"
./main 2 interleaved ./image.pgm ./filter.txt ./result.pgm
echo "Thread count: 4, type: INTERLEAVED"
./main 4 interleaved ./image.pgm ./filter.txt ./result.pgm
echo "Thread count: 8, type: INTERLEAVED"
./main 8 interleaved ./image.pgm ./filter.txt ./result.pgm
echo ""
echo "End"