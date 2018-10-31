#!/bin/bash

for dev in disk_sdb ssd_sda usb_sdc
do
	echo "doing merge IO test for $dev ......"
	fio ${dev}.job | sed -n 's/.*aggrb=\([0-9]*KB\).*/\1/p' > ${dev}.res
	printf "%s %s %s \n" "merge IO result for $dev :" "randwrite: `sed -n '1p' ${dev}.res`" "randread: `sed -n '2p' ${dev}.res`"	
done
