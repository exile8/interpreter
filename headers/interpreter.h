#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stack>

using std::stack;

class Parser {
    vector<string> code;
    int row;
    int position;
    enum STATE {
        OKAY,
        ERROR
    };

    string getSubcodeline(int n);
    void shift(int n);
    void skipSpaces();

    stack<Oper *> opers;
    vector<Lexem *> newPolizline;

    bool getSequenceOfCommands();
    bool getCommand();
    bool getExpression();
    bool isEndOfLine();
    bool isReservedWord(string word);

    bool getNumber();
    bool getVariable();

    bool getAssignOperator();
    bool getBinaryOperator();
    bool getLeftBracket();
    bool getRightBracket();
    bool getLeftQBracket();
    bool getRightQBracket();

    bool getLabel();
    bool initLabel(string name);
    bool getGoto();
    bool getIfBlock();
    bool getWhileBlock();
    bool getIf();
    bool getElse();
    bool getThen();
    bool getEndif();
    bool getWhile();
    bool getEndwhile();

    void buildBracketExpr();
    void sortOpersRight(Oper *op);
    void sortOpersLeft(Oper *op);
    void putCommandInPoliz();
    void emptyOpersStack(STATE state = OKAY);
public:
    vector<vector<Lexem *>> poliz;
    bool buildPoliz(vector<string> code);
    void freePoliz(STATE state = OKAY);
};

int getRightArgument(Lexem *operand);

Lexem *performAssignment(Lexem *leftArg, int rightArg, Assign *assign);

Lexem *performDereference(Lexem *leftArg, int rightArg, Dereference *deref);

Lexem *performCalculation(Lexem *leftArg, int rightArg, Binary *binary);

Lexem *currentResult(stack<Lexem *> & eval, Lexem *op);

bool getCondition(Lexem *condition);

int jump(Goto *op, stack<Lexem *> & eval, size_t row);

int evaluatePoliz(vector<Lexem *> poliz, int row);

void print(vector<Lexem *> v);

void printMap();

#endif
