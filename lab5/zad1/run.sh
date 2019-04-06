#!/bin/bash

make clean
make

echo "Start:"
./main commands.txt
echo "End"