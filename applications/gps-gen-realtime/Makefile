CXX=g++
CC=$(CXX)
RM=rm -f
CXXFLAGS=-g -std=c++11 -fpermissive -Wall -x c++ -I/usr/include
CFLAGS=$(CXXFLAGS)
LDFLAGS=-g -L/usr/lib/x86_64-linux-gnu
LDLIBS=-lpthread -lboost_system -lboost_thread -lboost_chrono -luhd

SRCS=main.cpp\
gpsgen.cpp\
uhd_tx_samples.cpp

OBJS=$(addsuffix .o,$(basename $(SRCS)))

all: gps-gen-realtime

gps-gen-realtime: $(OBJS)
	$(CXX) $(LDFLAGS) -o gps-gen-realtime $(OBJS) $(LDLIBS)

clean:
	$(RM) $(OBJS)
	$(RM) gps-gen-realtime
