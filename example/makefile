root=$(pwd)./thirdparty
inc=-I${root}/glog/include/ -I${root}/gflags/include/ -I${root}/libuuid/include/
lib=-L ${root}/glog/lib -lglog  -L${root}/gflags/lib -lgflags  -lpthread -L${root}/libuuid/lib -luuid


mylog : main.cpp vlog.cpp log.cpp
	g++  -g  -o $@ $^ ${inc} ${lib} -lglog -lgflags
clean:
	rm mylog *.o

