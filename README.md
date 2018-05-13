iotc
===

## Building
iotc uses libaio to submit IOs, so libaio-dev need be installed before compiling.
To compile iotc, get to the top directory of this repository and excute:
~# make

## Usage
### dispatch sequence IOs
Dispatch three sequence write IOs to sdb to check the back merging of plug list:
~# ./iotc  -d /dev/sdb -s 

### dispatch reverse IOs
Dispatch three reverse write IOs to sdb to check the front merging of plug list:
~# ./iotc  -d /dev/sdb -r 

### dispatch interleave IOs
Dispatch three reverse write IOs to sdb to check the back merging of elevator queue:
~# ./iotc  -d /dev/sdb -r 

### test bio merge between process
To do this, we need apply pmerge_bio.patch into kernel source firstly. Then use bellow
command line to start two processes to dispatch 5 IOs respectively: 
~# ./pmerge_A & ./pmerge_B

### test request merge between process
To do this, we need apply pmerge_request.patch into kernel source firstly. Then use bellow
command line to start two processes to dispatch 5 IOs respectively: 
 ~# ./pmerge_A & ./pmerge_B

All test cases could use command line "blktrace -d /dev/sdb -o - | blkparse -i -" to check
the result.
