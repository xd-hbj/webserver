CXX=g++
OBJECT = main.o http_conn.o

server:
	g++ -o server $(OBJECT)
%.o:%.cpp
	$(CC) -c $< -o $@

clean:
	rm server