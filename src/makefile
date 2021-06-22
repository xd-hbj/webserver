CXX=g++
CC=gcc
FLAG = -Wdeprecated
OBJECT = http_conn.o main.o webserver.o th.o timer.o utils.o
LIB = -lpthread -I../include/

server:$(OBJECT)
	$(CXX) $(FLAG) -o server $(OBJECT) $(LIB)

%.o:%.cpp
	$(CXX) $(FLAG) -c $< -o $@ $(LIB)

clean:
	rm server $(OBJECT)