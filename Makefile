INCLUDE=headers/
SRC=src/
LIB=lib/
CFLAGS=-Wall -Werror -fsanitize=leak,address -g
LDFLAGS=-fpic -shared

all: bin libinterpreter.so liblexemes.so
	g++ usr/main.cpp -I $(INCLUDE) -L $(LIB) $(CFLAGS) -linterpreter -llexemes -o bin/interpreter $(CFLAGS)

libinterpreter.so: $(LIB)
	g++ $(SRC)interpreter.cpp -o $(LIB)libinterpreter.so -I $(INCLUDE) $(LDFLAGS)
	
liblexemes.so: $(LIB)
	g++ $(SRC)lexemes.cpp -o $(LIB)liblexemes.so -I $(INCLUDE) $(LDFLAGS)

$(LIB):
	mkdir $(LIB)

bin:
	mkdir bin

clean:
	rm -r $(LIB)
	rm -r bin
