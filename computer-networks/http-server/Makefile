PROGRAMS = serwer
CXX = g++
CFLAGS = -Wall -Wextra -g -O2 -std=c++17

all: serwer

serwer: serwer.o aux.o
	$(CXX) $(CFLAGS) -o $@ $^

aux.o: aux.cpp aux.h
	$(CXX) $(CFLAGS) -c $<

serwer.o: serwer.cpp aux.h
	$(CXX) $(CFLAGS) -c $<

.PHONY: all clean

clean:
	rm -rf $(PROGRAMS) *.o
