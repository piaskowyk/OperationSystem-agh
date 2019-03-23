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
export LD_LIBRARY_PATH=/media/DATA/studia/sysopy/lab1/do_oddania/PiaskowyKrzysztof/cw1/zad3b

echo "> Start test on main_static"
echo "> Start test on main_static" >> results3b.txt
./main_static $arg >> results3b.txt
echo "> Start test on main_shared"
echo "------------------------------------------------------------" >> results3b.txt
echo "> Start test on main_shared" >> results3b.txt
./main_shared $arg >> results3b.txt
echo "> Start test on main_dynamic"
echo "------------------------------------------------------------" >> results3b.txt
echo "> Start test on main_dynamic" >> results3b.txt
./main_dynamic $arg >> results3b.txt

echo "------------------------------------------------------------" >> results3b.txt
echo "Wnioski:" >> results3b.txt
echo "Na podstawie pomiarów czasów dla programów o różnych poziomach optymalizacji stwierdzam, że ma ona istotny wpływ na szybkość wykonania programu." >> results3b.txt

echo "END"


