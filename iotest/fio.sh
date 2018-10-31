#!/bin/bash

# overwrite by user to specify the numjobs
MIN_THREAD=1
MAX_THREAD=1

# overwrite by user to specify io size, using KB as the unit
MIN_BS=4K
MAX_BS=4K

# IO queue depth, default to 128 which is a experience value
IO_DEPTH=128
RUN_TIME=30
FILE_SIZE=64G
FILE_NAME=""

function usage()
{
        echo "Usage: $SHELL_NAME [-J max_numjobs] [-j min_numjobs] [-B max_blocksize] [-b min_blocksize] [-t runtime] [-s size] FILENAME"
        echo "       -J the max numjobs would be created, should be power of 2"
        echo "       -j the min numjobs would be created, if you want test one block size,you can set max_numjobs=min_numjobs"
        echo "       -B the max blocksize would be used, must > 1KB and should be power of 2"
        echo "       -b the min blocksize would be used ,if you want test one block size,you can set max_blocksize=min_blocksize" 
	echo "	     -t specify how long should the test run"
	echo "       -s spefify the file size used to test"
	echo "	     FILENAME: specify the file name to test"
	
        exit 0
}

function check_env()
{
        if [ "$(whoami)" != "root" ]; then
                echo "Please run $SHELL_NAME use root!"
                exit 1
        fi
	
	if [ "$(which fio)" == "" ]; then
		echo "no fio found, Please install before testing!"
		exit 1
	fi
}


while getopts 'J:j:B:b:t:s:h' OPT; do
    case $OPT in
        J)
		MAX_THREAD="$OPTARG";;
        j)
		MIN_THREAD="$OPTARG";;
        B)
		MAX_BS="$OPTARG";;
        b)
		MIN_BS="$OPTARG";;
	t)
		RUN_TIME="$OPTARG";;	
	s)
		FILE_SIZE="$OPTARG";;
        ?)
            echo "Usage: `$SHELL_NAME $0` [options] filename"
    esac
done


# FILE_NAME must located at the end
shift $((OPTIND -1))
FILE_NAME=$1
if ! [ -e "$FILE_NAME" ]; then
	echo "Please specify a valide file name to test!"
	exit 2
fi

echo "Starting ..."
echo "The parameter used are showed as bellow:"
echo "		MAX_THREAD=$MAX_THREAD MIN_THREAD=$MIN_THREAD MAX_BS=$MAX_BS MIN_BS=$MIN_BS RUN_TIME=$RUN_TIME FILE_SIZE=$FILE_SIZE FILE_NAME=$FILE_NAME" 

# created fio template job file
TEMPLATE_JOB=`mktemp fio_template.XXX`
echo "[global]
ioengine=libaio
iodepth=$IO_DEPTH
size=$FILE_SIZE
direct=1
runtime=$RUN_TIME
filename=$FILE_NAME " > $TEMPLATE_JOB
echo "
[write]
rw=write
group_reporting
stonewall
[read]
rw=read
group_reporting
stonewall
[randwrite]
rw=randwrite
group_reporting
stonewall
[randread]
rw=randread
group_reporting
stonewall " >> $TEMPLATE_JOB

# do fio testing
min_block_size=`echo $MIN_BS | sed 's/\(.*\)\(.\)$/\1/'`
unit_min=`echo $MIN_BS | sed 's/\(.*\)\(.\)$/\2/'`
max_block_size=`echo $MAX_BS | sed 's/\(.*\)\(.\)$/\1/'`
TEMP_RES=`mktemp fio_tempres.XXX`

printf "\n"

for (( j=$MIN_THREAD; j<=$MAX_THREAD; j=j*2))
do
	printf "%45s\n" "IO test result for $j threads"
	echo "------------------------------------------------------------------"
	printf "%-16s%-18s%-18s%-18s%-18s \n" "" 顺序写 顺序读 随机写 随机读 

	for (( i=$min_block_size; i<=$max_block_size; i=i*2 ))
	do
		fio --numjobs=$j --bs=$i$unit_min $TEMPLATE_JOB | sed -n 's/.*aggrb=\([0-9]*.*\/s\)\, minb.*/\1/p'  > $TEMP_RES
		printf "%-15s" $i$unit_min
		for((k=1;k<=4;k++));
		do
			printf "%-15s" `sed -n "${k}p" $TEMP_RES`
		done
		printf "\n"
	done

	printf "\n"
done
 
#finish testing and clean env
rm -rf fio_temp*
