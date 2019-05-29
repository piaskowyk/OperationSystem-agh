#!/bin/bash

make clean
make main

echo "Start testing:"
./main 10 2 2 2
echo "End"