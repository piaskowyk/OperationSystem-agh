#!/usr/bin/env bash

arg1="
    create_table 10000 
    search_directory /usr *gimp* tmp_name 
    search_directory /usr Makefile tmp_name 
    search_directory /usr *2* mp_name 
    search_directory /usr/lib *1* tmp_name 
    remove_block 0 
    remove_block 0 
    remove_block 0 
    remove_block 0
    "

arg2="
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
arg3="
    create_table 100 
    search_directory /home *gimp* tmp_name 
    search_directory /usr Makefile tmp_name 
    search_directory /usr/lib *2* mp_name 
    search_directory /usr/lib *1* tmp_name 
    remove_block 0 
    remove_block 0 
    remove_block 0 
    remove_block 0
    "

rm raport2.txt
echo "> Start compilation"
make static
echo "> End compilation"
echo "> Start test no 1"
./main $arg1 >> raport2.txt
echo "> Start test no 2"
echo "------------------------------------------------------------" >> raport2.txt
./main $arg1 >> raport2.txt
echo "> Start test no 3"
echo "------------------------------------------------------------" >> raport2.txt
./main $arg1 >> raport2.txt

echo "END"