all: bin
	g++ src/main.cpp -o bin/main -Wall -Werror -fsanitize=leak,address -g
bin:
	mkdir bin
clean:
	rm -r bin
