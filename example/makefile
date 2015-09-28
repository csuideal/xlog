root=$(pwd)../
inc=${root}/include/
lib=${root}/lib/

mylog : main.cpp eros_log.cpp vlog.cpp
	g++  -g  -o $@ $^ -I${inc} -L${lib} -lglog -lgflags
clean:
	rm mylog *.o

