all: main

main: main.cpp
	g++ main.cpp -o main -lserial -pthread `pkg-config opencv --cflags` `pkg-config opencv --libs` -lalut -lopenal -std=c++14 
