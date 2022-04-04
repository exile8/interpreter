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
    GOTO, ASSIGN, COLON,
    LBRACKET, RBRACKET,
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
    "goto", ":=", ":",
    "(", ")",
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
    -1, 0, -1,
    -1, -1,
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

class Lexem {
public:
    Lexem() {}
    virtual ~Lexem() {}
};

void print(vector<Lexem *> v);

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
map<string, size_t> labels;

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

class Goto : public Oper {
public:
    Goto();
    int getValue(const Variable & var) const;
};

Goto::Goto() : Oper(GOTO) {
}

int Goto::getValue(const Variable & var) const {
    return labels[var.getName()];
}

class Parser {
    string codeline;
    size_t position;
    size_t row;
    string subcodeline(size_t n);
    void shift(size_t n);
    void skipSpaces();

    stack<Oper *> opers;
    vector<Lexem *> polizline;

    bool getCommand();
    bool getExpression();
    bool getNumber();
    bool getVariable();
    bool getAssignOperator();
    bool getBinaryOperator();
    bool getLeftBracket();
    bool getRightBracket();
    bool getColon();
    bool initLabel();
    bool getGoto();

    void buildBracketExpr();
    void sortOpersRight(Oper *op);
    void sortOpersLeft(Oper *op);
    void putOperInPoliz(Oper *op);
    void freeStack();
    void clear();

    bool buildPolizline(string codeline);
public:
    vector<vector<Lexem *>> poliz;
    bool buildPoliz(vector<string> code);
    void erase();
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

void Parser::skipSpaces() {
    while (codeline[position] == ' ' || codeline[position] == '\t') {
        position++;
    }
}

void Parser::buildBracketExpr() {
    while (opers.top()->getType() != LBRACKET) {
        polizline.push_back(opers.top());
        opers.pop();
    }
    delete opers.top();
    opers.pop();
}

void Parser::sortOpersRight(Oper *op) {
    int curPriority = op->getPriority();
    while (!opers.empty() && opers.top()->getPriority() > curPriority) {
        polizline.push_back(opers.top());
        opers.pop();
    }
    opers.push(op);
}

void Parser::sortOpersLeft(Oper *op) {
    int curPriority = op->getPriority();
    while (!opers.empty() && opers.top()->getPriority() >= curPriority) {
        polizline.push_back(opers.top());
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
        polizline.push_back(opers.top());
        opers.pop();
    }
}

void Parser::clear() {
    freeStack();
    print(polizline);
    for (size_t i = 0; i < poliz.size(); i++) {
        for (size_t j = 0; j < poliz[i].size(); j++) {
            delete poliz[i][j];
        }
        poliz[i].clear();
    }
    for (size_t i = 0; i < polizline.size(); i++) {
        delete polizline[i];
    }
    polizline.clear();
    poliz.clear();
}

bool Parser::getNumber() {
    skipSpaces();
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
    polizline.push_back(new Number(number));
    return true;
}

bool Parser::getVariable() {
    skipSpaces();
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
    polizline.push_back(new Variable(name));
    if (!opers.empty() && opers.top()->getType() == GOTO) {
        if (labels.count(name) == 0) {
            labels[name] = UNDEFINED;
        }
        polizline.push_back(opers.top());
        opers.pop();
    }
    return true;
}

bool Parser::getAssignOperator() {
    skipSpaces();
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
    skipSpaces();
    if (codeline[position] == '(') {
        putOperInPoliz(new Binary(LBRACKET));
        shift(1);
        return true;
    } else {
        return false;
    }
}

bool Parser::getRightBracket() {
    skipSpaces();
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
    skipSpaces();
    for (size_t i = 0; i < n; i++) {
        if (PRIORITY[i] < 0) {
            continue;
        }
        string op = subcodeline(OPERATOR_STRING[i].size());
        if (op.compare(OPERATOR_STRING[i]) == 0) {
            putOperInPoliz(new Binary(OPERATOR(i)));
            shift(OPERATOR_STRING[i].size());
            return true;
        }
    }
    return false;
}

bool Parser::initLabel() {
    string name = dynamic_cast<Variable *>(polizline.back())->getName();
    if (labels.count(name) > 0 && labels[name] < row) {
        return false;
    } else {
        labels[name] = row;
        return true;
    }
}

bool Parser::getColon() {
    skipSpaces();
    if (codeline[position] == ':') {
        if (initLabel() == false) {
            return false;
        }
        delete polizline.back();
        polizline.pop_back();
        shift(1);
        return true;
    } else {
        return false;
    }
}

bool Parser::getGoto() {
    skipSpaces();
    string op = subcodeline(4);
    if (op.compare("goto") == 0) {
        opers.push(new Goto());
        shift(4);
        return true;
    } else {
        return false;
    }
}

bool Parser::getExpression() {
    if (getNumber()) {
        if (getAssignOperator() || getLeftBracket()) {
            return false;
        } else if (getBinaryOperator()) {
            return getExpression();
        } else {
            return true;
        }
    } else if (getGoto()) {
        return getVariable();
    } else if (getVariable()) {
        if (getLeftBracket()) {
            return false;
        } else if (getAssignOperator() || getColon() || getBinaryOperator()) {
            return getExpression();
        } else {
            return true;
        }
    } else if (getLeftBracket() && getExpression() && getRightBracket()) {
        if (getAssignOperator() || getLeftBracket()) {
            return false;
        } else if (getBinaryOperator()) {
            return getExpression();
        } else {
            return true;
        }
    }
    return false;
}

bool Parser::getCommand() {
    if (getExpression() && (position == codeline.size())) {
        freeStack();
        return true;
    } else {
        return false;
    }
}

bool Parser::buildPolizline(string codeline) {
    Parser::codeline = codeline;
    polizline.clear();
    position = 0;
    if (getCommand()) {
        print(polizline);
        return true;
    } else {
        clear();
        return false;
    }
}

void Parser::erase() {
    for (size_t i = 0; i < poliz.size(); i++) {
        for (size_t j = 0; j < poliz[i].size(); j++) {
            delete poliz[i][j];
        }
        poliz[i].clear();
    }
    poliz.clear();
}

bool Parser::buildPoliz(vector<string> code) {
    for (size_t i = 0; i < code.size(); i++) {
        row = i;
        if (buildPolizline(code[i]) == false) {
            cout << i << "!!!" << endl;
            cerr << "Error: wrong syntax" << endl;
            return false;
        }
        poliz.push_back(polizline);
    }
    return true;
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
    eval.pop();
    if (assign != nullptr) {
        Variable *left = dynamic_cast<Variable *>(eval.top());
        result = new Number(assign->getValue(*left, rightArg));
    } else if (dynamic_cast<Number *>(eval.top())) {
        int leftNum = dynamic_cast<Number *>(eval.top())->getValue();
        result = new Number(binary->getValue(leftNum, rightArg));
    } else {
        int leftVar = dynamic_cast<Variable *>(eval.top())->getValue();
        result = new Number(binary->getValue(leftVar, rightArg));
    }
    eval.pop();
    return result;
}

int evaluatePostfix(vector<Lexem *> postfix, size_t row) {
    int value;
    stack<Lexem *> eval;
    vector<Lexem *>::iterator it;
    vector<Number *> intermediate;
    for (it = postfix.begin(); it != postfix.end(); it++) {
        if (dynamic_cast<Number *>(*it) || dynamic_cast<Variable *>(*it)) {
            eval.push(*it);
        } else if (dynamic_cast<Goto *>(*it)) {
            int newRow = dynamic_cast<Goto *>(*it)->getValue(*(dynamic_cast<Variable *>(eval.top())));
            eval.pop();
            return newRow;
        } else {
            intermediate.push_back(currentResult(eval, dynamic_cast<Binary *>(*it),
                                        dynamic_cast<Assign *>(*it)));
            eval.push(intermediate.back());
        }
    }
    value = dynamic_cast<Number *>(eval.top())->getValue();
    cout << value << endl;
    for (size_t i = 0; i < intermediate.size() - 1; i++) {
        delete intermediate[i];
    }
    delete eval.top();
    return row + 1;
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
    vector<string> code;
    string codeline;
    while (getline(cin, codeline)) {
        code.push_back(codeline);
    }

    if (parser.buildPoliz(code)) {
        size_t i = 0;
        while (i < code.size()) {
            i = evaluatePostfix(parser.poliz[i], i);
            printMap();
        }
    }
    parser.erase();
    return 0;
}
