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
