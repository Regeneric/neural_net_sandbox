CC = g++
CFLAGS = -ggdb -g3 -llua -ldl -lX11 -lGL -lpng -lstdc++fs -std=c++20 -lsfml-graphics -lsfml-window -lsfml-system

all: main.o neuron.o network.o
	$(CC) $(CFLAGS) ./build/main.o ./build/neuron.o ./build/network.o -o ./neural-net

main.o: main.cpp
	$(CC) $(CFLAGS) -c -o ./build/main.o main.cpp

neuron.o: ./Neuron.cpp ./headers/Neuron.hpp
	$(CC) $(CFLAGS) -c -o ./build/neuron.o ./Neuron.cpp

network.o: ./Network.cpp ./headers/Network.hpp
	$(CC) $(CFLAGS) -c -o ./build/network.o ./Network.cpp

clean:
	rm -rf ./build/*
	rm -rf ./neural-net
