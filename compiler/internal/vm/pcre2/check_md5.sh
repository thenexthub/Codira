#! /bin/bash

input_file=$1
output_file=$2

if [ -f $input_file ] && [ -f $output_file ];then
    input_file_md5=$(md5sum ${input_file} | awk '{print $1}')
    output_file_md5=$(md5sum ${output_file} | awk '{print $1}')
    if [ "${input_file_md5}"x == "${output_file_md5}"x ];then
        echo "ok" 
    fi
fi
