root=$(pwd)../
inc=${root}/include/
lib=-L ${root}/lib/glog -L${root}/lib/gflags #-L${root}/../src/bin/lib/

mylog : main.cpp eros_log.cpp vlog.cpp
	g++  -g  -o $@ $^ -I${inc} ${lib} -lglog -lgflags
clean:
	rm mylog *.o

