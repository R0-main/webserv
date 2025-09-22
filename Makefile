# Simple Makefile for C++98

CXX = c++
#-Wall -Wextra -Werror
CXXFLAGS =  -std=c++98
COMMON_SRCS =

all: server client

server: ./src/main.cpp $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) -o server ./src/main.cpp $(COMMON_SRCS)

start_client: clean client
	./client "Test Message"

start_server: clean server
	./server

clean:
	rm -f server client

.PHONY: all clean


