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

enum OPERATOR {
    LBRACKET, RBRACKET,
    OR,
    AND,
    BITOR,
    XOR,
    BITAND,
    EQ, ASSIGN,
    NEQ,
    LEQ, SHL,
    LT,
    GEQ, SHR,
    GT,
    PLUS, MINUS,
    MULT, DIV, MOD
};

string OPERATOR_STRING[] = {
    "(", ")",
    "||",
    "&&",
    "|",
    "^",
    "&",
    "==", ":=",
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
    1,
    2,
    3,
    4,
    5,
    6, 0,
    6,
    7, 8,
    7,
    7, 8,
    7,
    9, 9,
    10, 10, 10
};

class Lexem {
public:
    Lexem() {}
    virtual ~Lexem() {}
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
    Variable(string name);
    string getName() const;
    int getValue() const;
    void setValue(int value) const;
};

map<string, int> vars;

class Oper : public Lexem {
    OPERATOR opertype;
public:
    Oper(OPERATOR opertype);
    OPERATOR getType() const;
    int getPriority() const;
};

class Binary : public Oper {
public:
    Binary(OPERATOR opertype) : Oper(opertype) {}
    int getValue(int left, int right) const;
};

class Assign : public Oper {
public:
    Assign() : Oper(ASSIGN) {}
    int getValue(const Variable & left, int right) const;
};

class Parser {
    string codeline;
    size_t position;
    stack<Oper *> opers;
    string subcodeline(size_t n);
    void shift(size_t n);
    int openBrackets;

    bool getCommand();
    bool getExpression();
    bool getNumber();
    bool getVariable();
    bool getAssignOperator();
    bool getBinaryOperator();
    bool getLeftBracket();
    bool getRightBracket();

    void buildBracketExpr();
    void sortOpersRight(Oper *op);
    void sortOpersLeft(Oper *op);
    void putOperInPoliz(Oper *op);
    void freeStack();
    void clear();
public:
    vector<Lexem *> poliz;
    bool buildPoliz(string codeline);
};

string Parser::subcodeline(size_t n) {
    string str;
    for (size_t i = 0; i < n; i++) {
        str.push_back(codeline[position + i]);
    }
    return str;
}

void Parser::shift(size_t n) {
    position += n;
}

void Parser::buildBracketExpr() {
    while (opers.top()->getType() != LBRACKET) {
        poliz.push_back(opers.top());
        opers.pop();
    }
    delete opers.top();
    opers.pop();
}

void Parser::sortOpersRight(Oper *op) {
    int curPriority = op->getPriority();
    while (!opers.empty() && opers.top()->getPriority() > curPriority) {
        poliz.push_back(opers.top());
        opers.pop();
    }
    opers.push(op);
}

void Parser::sortOpersLeft(Oper *op) {
    int curPriority = op->getPriority();
    while (!opers.empty() && opers.top()->getPriority() >= curPriority) {
        poliz.push_back(opers.top());
        opers.pop();
    }
    opers.push(op);
}

void Parser::putOperInPoliz(Oper *op) {
    if (dynamic_cast<Assign *>(op)) {
        sortOpersRight(op);
    } else {
        switch (dynamic_cast<Binary *>(op)->getType()) {
            case LBRACKET:
                opers.push(op);
                break;
            case RBRACKET:
                delete op;
                buildBracketExpr();
                break;
            default:
                sortOpersLeft(op);
                break;
        }
    }
}

void Parser::freeStack() {
    while (!opers.empty()) {
        poliz.push_back(opers.top());
        opers.pop();
    }
}

void Parser::clear() {
    freeStack();
    for (size_t i = 0; i < poliz.size(); i++) {
        delete poliz[i];
    }
    poliz.clear();
}

bool Parser::getNumber() {
    while (getLeftBracket()) {
        openBrackets++;
    }
    int number = codeline[position] - '0';
    if (isdigit(codeline[position])) {
        shift(1);
    } else {
        return false;
    }
    while (isdigit(codeline[position])) {
        number = number * 10 + codeline[position] - '0';
        shift(1);
    }
    poliz.push_back(new Number(number));
    while (getRightBracket()) {
        openBrackets--;
    }
    return true;
}

bool Parser::getVariable() {
    while (getLeftBracket()) {
        openBrackets++;
    }
    string name(1, codeline[position]);
    if (isalpha(codeline[position]) || codeline[position] == '_') {
        shift(1);
    } else {
        return false;
    }
    while (isalpha(codeline[position]) || isdigit(codeline[position]) ||
           codeline[position] == '_') {
        name.push_back(codeline[position]);
        shift(1);
    }
    poliz.push_back(new Variable(name));
    while (getRightBracket()) {
        openBrackets--;
    }
    return true;
}

bool Parser::getAssignOperator() {
    string op = subcodeline(2);
    if (op.compare(":=") == 0) {
        shift(2);
        putOperInPoliz(new Assign());
        return true;
    } else {
        return false;
    }
}

bool Parser::getLeftBracket() {
    if (codeline[position] == '(') {
        putOperInPoliz(new Binary(LBRACKET));
        shift(1);
        return true;
    } else {
        return false;
    }
}

bool Parser::getRightBracket() {
    if (codeline[position] == ')') {
        putOperInPoliz(new Binary(RBRACKET));
        shift(1);
        return true;
    } else {
        return false;
    }
}

bool Parser::getBinaryOperator() {
    size_t n = sizeof(OPERATOR_STRING) / sizeof(string);
    for (size_t i = 0; i < n; i++) {
        string op = subcodeline(OPERATOR_STRING[i].size());
        if (op.compare(OPERATOR_STRING[i]) == 0) {
            putOperInPoliz(new Binary(OPERATOR(i)));
            shift(OPERATOR_STRING[i].size());
            return true;
        }
    }
    return false;
}

bool Parser::getExpression() {
    if (getNumber()) {
        if (position == codeline.size()) {
            return true;
        } else if (getAssignOperator()) {
            return false;
        } else {
            return getBinaryOperator() && getExpression();
        }
    } else if (getVariable()) {
        if (position == codeline.size()) {
            return true;
        } else if (getAssignOperator() || getBinaryOperator()) {
            return getExpression();
        }
    }
    return false;
}

bool Parser::getCommand() {
    if (getExpression() && openBrackets == 0) {
        freeStack();
        return true;
    } else {
        return false;
    }
}

bool Parser::buildPoliz(string codeline) {
    Parser::codeline = codeline;
    position = 0;
    openBrackets = 0;
    poliz.clear();
    if (getCommand()) {
        return true;
    } else {
        clear();
        cerr << "Error: wrong syntax" << endl;
        return false;
    }
}

Number::Number(int value) {
    Number::value = value;
}

int Number::getValue() const {
    return value;
}

Variable::Variable(string name) {
    Variable::name = name;
}

string Variable::getName() const {
    return name;
}

int Variable::getValue() const {
    return vars[name];
}

void Variable::setValue(int value) const {
    vars[name] = value;
}

Oper::Oper(OPERATOR opertype) {
    Oper::opertype = opertype;
}

OPERATOR Oper::getType() const {
    return opertype;
}

int Oper::getPriority() const {
    int priority;
    size_t n = sizeof(OPERATOR_STRING) / sizeof(string);
    for (size_t i = 0; i < n; i++) {
        if (opertype == OPERATOR(i)) {
            priority = PRIORITY[i];
        }
    }
    return priority;
}

int Binary::getValue(int left, int right) const {
    switch (getType()) {
        case OR:
            return left || right;
        case AND:
            return left && right;
        case BITOR:
            return left | right;
        case XOR:
            return left ^ right;
        case BITAND:
            return left & right;
        case EQ:
            return left == right;
        case NEQ:
            return left != right;
        case LEQ:
            return left <= right;
        case SHL:
            return left << right;
        case LT:
            return left < right;
        case GEQ:
            return left >= right;
        case SHR:
            return left >> right;
        case GT:
            return left > right;
        case PLUS:
            return left + right;
        case MINUS:
            return left - right;
        case MULT:
            return left * right;
        case DIV:
            return left / right;
        case MOD:
            return left % right;
        default:
            cerr << "Error: invalid operation" << endl;
            break;
    }
    return -1;
}

int Assign::getValue(const Variable & left, int right) const {
    if (getType() == ASSIGN) {
        left.setValue(right);
        return right;
    } else {
        cerr << "Error: invalid operation" << endl;
        return -1;
    }
}


Number *currentResult(stack<Lexem *> & eval, Binary *binary, Assign *assign) {
    int rightArg;
    Number *result;
    if (dynamic_cast<Number *>(eval.top())) {
        rightArg = dynamic_cast<Number *>(eval.top())->getValue();
    } else {
        rightArg = dynamic_cast<Variable *>(eval.top())->getValue();
    }
    delete eval.top();
    eval.pop();
    if (assign != nullptr) {
        Variable *left = dynamic_cast<Variable *>(eval.top());
        result = new Number(assign->getValue(*left, rightArg));
        delete assign;
    } else if (dynamic_cast<Number *>(eval.top())) {
        int leftNum = dynamic_cast<Number *>(eval.top())->getValue();
        result = new Number(binary->getValue(leftNum, rightArg));
        delete binary;
    } else {
        int leftVar = dynamic_cast<Variable *>(eval.top())->getValue();
        result = new Number(binary->getValue(leftVar, rightArg));
        delete binary;
    }
    delete eval.top();
    eval.pop();
    return result;
}

int evaluatePostfix(vector<Lexem *> postfix) {
    int value;
    stack<Lexem *> eval;
    vector<Lexem *>::iterator it;
    for (it = postfix.begin(); it != postfix.end(); it++) {
        if (dynamic_cast<Number *>(*it) || dynamic_cast<Variable *>(*it)) {
            eval.push(*it);
        } else {
            eval.push(currentResult(eval, dynamic_cast<Binary *>(*it),
                                        dynamic_cast<Assign *>(*it)));
        }
    }
    value = dynamic_cast<Number *>(eval.top())->getValue();
    delete eval.top();
    return value;
}

void print(vector<Lexem *> v) {
    vector<Lexem *>::iterator it;
    size_t n = sizeof(OPERATOR_STRING) / sizeof(string);
    for (it = v.begin(); it != v.end(); it++) {
        if (dynamic_cast<Number *>(*it)) {
            cout << dynamic_cast<Number *>(*it)->getValue();
        } else if (dynamic_cast<Variable *>(*it)) {
            cout << dynamic_cast<Variable *>(*it)->getName();
        } else {
            for (size_t i = 0; i < n; i++) {
                if (dynamic_cast<Oper *>(*it)->getType() == OPERATOR(i)) {
                    cout << OPERATOR_STRING[i];
                }
            }
        }
    }
    cout << endl;
}

void printMap() {
    map<string, int>::iterator it;
    cout << "--------Variables--------" << endl;
    for (it = vars.begin(); it != vars.end(); it++) {
        cout << it->first << " = " << it->second << endl;
    }
    cout << "-------------------------" << endl;
}

int main() {
    Parser parser;
    string codeline;
    int value;
    while (getline(cin, codeline)) {
        if (parser.buildPoliz(codeline)) {
            print(parser.poliz);
            value = evaluatePostfix(parser.poliz);
            cout << value << endl;
            printMap();
        }
    }
    return 0;
}
