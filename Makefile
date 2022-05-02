INCLUDE=headers/
SRC=src/
LIB=lib/
BIN=bin/
CFLAGS=-Wall -Werror -fsanitize=leak,address -g
LDFLAGS=-fpic -shared -g

all: $(BIN) libinterpreter.so liblexemes.so
	g++ usr/main.cpp -I $(INCLUDE) -L $(LIB) $(CFLAGS) -linterpreter -llexemes -o $(BIN)interpreter $(CFLAGS)

libinterpreter.so: $(LIB)
	g++ $(SRC)interpreter.cpp -o $(LIB)libinterpreter.so -I $(INCLUDE) $(LDFLAGS)
	
liblexemes.so: $(LIB)
	g++ $(SRC)lexemes.cpp -o $(LIB)liblexemes.so -I $(INCLUDE) $(LDFLAGS)

$(LIB):
	mkdir $(LIB)

$(BIN):
	mkdir $(BIN)

clean:
	rm -r $(LIB)
	rm -r $(BIN)
