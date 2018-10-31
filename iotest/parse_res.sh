#!/bin/bash

if [ $# -lt 2 ]
then
	echo usage: ./parse_res.sh dev_type res_file
	exit -1
fi

printf "%45s\n" "IO test result for $1" 
echo "------------------------------------------------------------------"
printf "%-14s%-18s%-18s%-18s%-18s \n" "" 随机读 随机写 顺序读 顺序写

myres=$2
mytmp=1

for i in 1线 4线 16线
do
	printf "%-15s" $i
	for((j=1;j<=4;j++));
	do
		printf "%-15s" ` sed -n "${mytmp}p" $myres`
		mytmp=$(($mytmp+1))
	done
	printf "\n"
done

