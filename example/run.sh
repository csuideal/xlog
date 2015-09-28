#!/bin/sh

CURRENT_DIR=`pwd`
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:../lib/


 ./mylog 1 > 1.log  #  2>&1 &
# ./mylog 10 > 10.log  2>&1 & 

#nohup ./mylog 1 >/dev/null & 
#nohup ./mylog 10 >/dev/null & 
