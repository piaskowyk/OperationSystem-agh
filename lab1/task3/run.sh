#!/usr/bin/env bash

arg="create_table 123 search_directory /usr/lib *gimp* tmp_name search_directory /usr/lib *gimp1* tmp_name search_directory /usr/lib *gimp2*
tmp_name search_directory /usr/lib *gimp3* tmp_name remove_block 0 remove_block 0 remove_block 0 remove_block 0"

export LD_LIBRARY_PATH=/media/DATA/studia/sysopy/lab1/task3:$LD_LIBRARY_PATH

#make static_o0
#make static_o1
#make static_o2
#make static_o3
#make static_os

#make shared_o0
#make shared_o1
#make shared_o2
#make shared_o3
#make shared_os

#./main_shared_o1 $arg

make dynamic_o0
./main_dynamic_o0 $arg