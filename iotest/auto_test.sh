#!/bin/bash

for dev in disk ssd usb
do
	echo "doing IO test for $dev ......"
	fio ${dev}.job | sed -n 's/.*aggrb=\([0-9]*KB\).*/\1/p' > ${dev}.res
	./parse_res.sh $dev ${dev}.res	
done
