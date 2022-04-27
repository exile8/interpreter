#ifndef LEXEMES_H
#define LEXEMES_H


#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <cctype>

using std::getline;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::stack;
using std::map;
using std::cerr;

#define UNDEFINED -1

enum OPERATOR {
    IF, THEN,
    ELSE, ENDIF,
    WHILE, ENDWHILE,
    GOTO, ASSIGN, COLON,
    LBRACKET, RBRACKET,
    LQBRACKET, RQBRACKET, DEREF,
    OR,
    AND,
    BITOR,
    XOR,
    BITAND,
    EQ,
    NEQ,
    LEQ, SHL,
    LT,
    GEQ, SHR,
    GT,
    PLUS, MINUS,
    MULT, DIV, MOD
};

string OPERATOR_STRING[] = {
    "if", "then",
    "else", "endif",
    "while", "endwhile",
    "goto", ":=", ":",
    "(", ")",
    "[", "]", "dereference",
    "||",
    "&&",
    "|",
    "^",
    "&",
    "==",
    "!=",
    "<=", "<<",
    "<",
    ">=", ">>",
    ">",
    "+", "-",
    "*", "/", "%"
};

int PRIORITY[] = {
    -1, -1,
    -1, -1,
    -1, -1,
    -1, 0, -1,
    -1, -1,
    -1, -1, -1,
    1,
    2,
    3,
    4,
    5,
    6,
    6,
    7, 8,
    7,
    7, 8,
    7,
    9, 9,
    10, 10, 10
};

string RESERVED[] = {
    "if", "else", "while",
    "then", "endif", "endwhile"
};

class Lexem {
public:
    Lexem();
    virtual ~Lexem();
};

class Number : public Lexem {
    int value;
public:
    Number(int value);
    int getValue() const;
};

class Variable : public Lexem {
    string name;
public:
    static map<string, int> VarTable;
    Variable(string name);
    string getName() const;
    int getValue() const;
    void setValue(int value) const;
};

class ArrayElem : public Lexem {
    string name;
    int index;
public:
    static map<string, vector<int>> ArrayTable;
    ArrayElem(string name, int index);
    int getValue() const;
    void setValue(int value) const;
};

class Oper : public Lexem {
    OPERATOR opertype;
public:
    Oper(OPERATOR opertype);
    OPERATOR getType() const;
    int getPriority() const;
};

class Binary : public Oper {
public:
    Binary(OPERATOR opertype);
    int getValue(int left, int right) const;
};

class Assign : public Oper {
public:
    Assign();
    int getValue(const Variable & left, int right) const;
    int getValue(const ArrayElem & left, int right) const;
};

class Goto : public Oper {
    int row;
public:
    static map<string, int> LabelTable;
    Goto(OPERATOR opertype);
    void setRow(int row);
    int getRow();
    int getValue(const Variable & var) const;
};

class Dereference : public Oper {
public:
    Dereference();
    ArrayElem *getValue(string name, int index) const;
};

#endif
