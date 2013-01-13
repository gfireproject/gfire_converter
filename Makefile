CC = g++

all:
	$(CC) main.cpp -o gfire_converter

clean:
	rm -f gfire_converter
