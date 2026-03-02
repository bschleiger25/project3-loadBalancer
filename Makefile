CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2

TARGET = loadbalancer
SRCS = main.cpp Request.cpp RequestQueue.cpp WebServer.cpp LoadBalancer.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

run: $(TARGET)
	./$(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	-rm -f *.o *.exe $(TARGET)

.PHONY: all run clean
