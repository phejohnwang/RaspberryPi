CXX = g++
FLAGS = -g -Wall -std=c++11 -pthread
EXTRA = -lwiringPi
TARGET = server

all: $(TARGET)

$(TARGET): server.o connection.o
	$(CXX) $(FLAGS) $(EXTRA) -o $(TARGET) server.o connection.o
	@rm -f *.o

# Object files

server.o: server.cpp
	$(CXX) $(FLAGS) $(EXTRA) -c server.cpp

connection.o: connection.cpp
	$(CXX) $(FLAGS) -c connection.cpp

clean:
	rm -f *.o
	rm -f server
