#!/usr/bin/env bash

arg="
    create_table 500 
    search_directory /usr/lib *gimp* tmp_name 
    search_directory /usr/lib Makefile tmp_name 
    search_directory /home/mleko *2* mp_name 
    search_directory /usr/lib *1* tmp_name 
    remove_block 0 
    remove_block 0 
    remove_block 0 
    remove_block 0
    "
export LD_LIBRARY_PATH=/media/DATA/studia/sysopy/lab1/do_oddania/PiaskowyKrzysztof/cw1/zad3a

echo "> Start test on main_static"
echo "> Start test on main_static" >> results3a.txt
./main_static $arg >> results3a.txt
echo "> Start test on main_shared"
echo "------------------------------------------------------------" >> results3a.txt
echo "> Start test on main_shared" >> results3a.txt
./main_shared $arg >> results3a.txt
echo "> Start test on main_dynamic"
echo "------------------------------------------------------------" >> results3a.txt
echo "> Start test on main_dynamic" >> results3a.txt
./main_dynamic $arg >> results3a.txt

echo "------------------------------------------------------------" >> results3a.txt
echo "Wnioski:" >> results3a.txt
echo "Na podstawie czasów wykonania programu stwierdzam że sposób ładowania programu nie ma wpływu na szybkość wykonania programu." >> results3a.txt

echo "END"


