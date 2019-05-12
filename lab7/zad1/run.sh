#!/bin/bash

make clean
make loader
make trucker

# mkdir mem
# touch mem/MEM_LINE
# touch mem/MEM_LINE_PARAM
# touch mem/SEM_LINE
# touch mem/SEM_LINE_PARAM

echo "Start:"
./trucker 10 10 10 &
./loader 1
echo "End"