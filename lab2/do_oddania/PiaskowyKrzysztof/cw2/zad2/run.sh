#/bin/bash
make clean_all
make main1
make main2
make clean

echo "Search in directory /home, (using diropen)"
./main1 /home '>' 19/03/2019 00:00:00
echo ""
echo "----------------------------------------------------------------------"
echo ""
echo "Search in directory /home, (using ftw)"
./main2 /home '>' 19/03/2019 00:00:00

echo "End"