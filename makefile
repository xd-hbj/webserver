CXX=g++
CC=gcc
OBJECT = http_conn.o main.o webserver.o threadpool.o
LIB = -lpthread

server:$(OBJECT)
	$(CXX) -o server $(OBJECT) $(LIB)

%.o:%.cpp
	$(CXX) -c $< -o $@ $(LIB)

clean:
	rm server $(OBJECT)