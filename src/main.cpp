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

enum OPERATOR {
    LBRACKET, RBRACKET,
    ASSIGN,
    PLUS, MINUS,
    MULTIPLY
};

char OPERATOR_STRING[] = {
    '(', ')',
    '=',
    '+', '-',
    '*'
};

int PRIORITY[] = {
    -1, -1,
    0,
    1, 1,
    2
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
    void setValue(int value);
};

map<string, int> vars;

class Oper : public Lexem {
    OPERATOR opertype;
public:
    Oper(OPERATOR opertype);
    OPERATOR getType() const;
    int getPriority() const;
    int getValue(int left, int right) const;
};

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

void Variable::setValue(int value) {
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
    for (size_t i = 0; i < sizeof(OPERATOR_STRING); i++) {
        if (opertype == OPERATOR(i)) {
            priority = PRIORITY[i];
        }
    }
    return priority;
}

int Oper::getValue(int left, int right) const {
    switch (opertype) {
        case PLUS:
            return left + right;
        case MINUS:
            return left - right;
        case MULTIPLY:
            return left * right;
        default:
            break;
    }
    return -1;
}

int readNumber(string::iterator & it, const string::iterator & end) {
    int number = 0;
    while (it != end && isdigit(*it)) {
        number = number * 10 + *it - '0';
        it++;
    }
    it--;
    return number;
}

string readVariable(string::iterator & it, const string::iterator & end) {
    string variable;
    while (it != end && (isalpha(*it) || isdigit(*it) || *it == '_')) {
        variable.push_back(*it);
        it++;
    }
    it--;
    return variable;
}

OPERATOR readOper(string::iterator & it) {
    OPERATOR opertype;
    for (size_t i = 0; i < sizeof(OPERATOR_STRING); i++) {
        if (*it == OPERATOR_STRING[i]) {
            opertype = OPERATOR(i);
            break;
        }
    }
    return opertype;
}

vector<Lexem *> parseLexem(string codeline) {
    vector<Lexem *> infix;
    string::iterator it;
    for (it = codeline.begin(); it != codeline.end(); it++) {
        if (*it == ' ' || *it == '\t') {
            continue;
        } else if (isdigit(*it)) {
            infix.push_back(new Number(readNumber(it, codeline.end())));
        } else if (isalpha(*it) || *it == '_') {
            infix.push_back(new Variable(readVariable(it, codeline.end())));
        } else {
            infix.push_back(new Oper(readOper(it)));
        }
    }
    return infix;
}

void buildBracketExpr(vector<Lexem *> & postfix, stack<Lexem *> & opers) {
    while (dynamic_cast<Oper *>(opers.top())->getType() != LBRACKET) {
        postfix.push_back(opers.top());
        opers.pop();
    }
    delete opers.top();
    opers.pop();
}

void sortOpersRight(vector<Lexem *> & postfix, stack<Lexem *> & opers, Lexem *operation) {
    int curPriority = dynamic_cast<Oper *>(operation)->getPriority();
    while (!opers.empty() &&
                dynamic_cast<Oper *>(opers.top())->getPriority() > curPriority) {
        postfix.push_back(opers.top());
        opers.pop();
    }
    opers.push(operation);
}

void sortOpersLeft(vector<Lexem *> & postfix, stack<Lexem *> & opers, Lexem *operation) {
    int curPriority = dynamic_cast<Oper *>(operation)->getPriority();
    while (!opers.empty() &&
                dynamic_cast<Oper *>(opers.top())->getPriority() >= curPriority) {
        postfix.push_back(opers.top());
        opers.pop();
    }
    opers.push(operation);
}

void putOpersInPostfix(vector<Lexem *> & postfix, stack<Lexem *> & opers, Lexem * operation) {
    switch (dynamic_cast<Oper *>(operation)->getType()) {
        case LBRACKET:
            opers.push(operation);
            break;
        case RBRACKET:
            delete operation;
            buildBracketExpr(postfix, opers);
            break;
        case ASSIGN:
            sortOpersRight(postfix, opers, operation);
            break;
        default:
            sortOpersLeft(postfix, opers, operation);
            break;
    }
}

vector<Lexem *> buildPostfix(vector<Lexem *> infix) {
    vector<Lexem *> postfix;
    vector<Lexem *>::iterator it;
    stack <Lexem *> opers;
    for (it = infix.begin(); it != infix.end(); it++) {
        if (dynamic_cast<Number *>(*it) || dynamic_cast<Variable *>(*it)) {
            postfix.push_back(*it);
        } else {
            putOpersInPostfix(postfix, opers, *it);
        }
    }
    while (!opers.empty()) {
        postfix.push_back(opers.top());
        opers.pop();
    }
    return postfix;
}

/*Number *currentResult(stack <Lexem *> & eval, Oper *operation) {
    Lexem *rightArg, *leftArg, *result;
    rightArg = eval.top();
    eval.pop();
    if (operation->getType() == ASSIGN) {
        Variable *leftArg = dynamic_cast<Variable *>(eval.top());
        eval.pop();
        if (dynamic_cast<Number *>(rightArg)) {
            leftArg.setValue(dynamic_cast<Number *>(rightArg)->getValue());
        } else {
            leftArg.setValue(dynamic_cast<Variable *>(rightArg)->getValue());
        }
    }
    result = new Number(operation->getValue(*leftArg->getValue(), *rightArg->getValue));
    delete leftArg;
    delete rightArg;
    delete operation;
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
            eval.push(currentResult(eval, dynamic_cast<Oper *>(*it)));
        }
    }
    value = dynamic_cast<Number *>(eval.top())->getValue();
    delete eval.top();
    return value;
}*/

void print(vector<Lexem *> v) {
    vector<Lexem *>::iterator it;
    for (it = v.begin(); it != v.end(); it++) {
        if (dynamic_cast<Number *>(*it)) {
            cout << dynamic_cast<Number *>(*it)->getValue();
        } else if (dynamic_cast<Variable *>(*it)) {
            cout << dynamic_cast<Variable *>(*it)->getName();
        } else {
            for (size_t i = 0; i < sizeof(OPERATOR_STRING); i++) {
                if (dynamic_cast<Oper *>(*it)->getType() == OPERATOR(i)) {
                    cout << OPERATOR_STRING[i];
                }
            }
        }
    }
    cout << endl;
}

/*void printMap() {
    map<string, Variable *>::iterator it;
    for (it = vars.begin(); it != vars.end(); it++) {
        cout << it->second->getName() << " ";
    }
    cout << endl;
}*/

void clear(vector<Lexem *> v) {
    vector<Lexem *>::iterator it;
    for (it = v.begin(); it != v.end(); it++) {
        delete (*it);
    }
}

int main() {
    string codeline;
    vector<Lexem *> infix;
    vector<Lexem *> postfix;
//    int value;

    while (getline(cin, codeline)) {
        infix = parseLexem(codeline);
        print(infix);
        postfix = buildPostfix(infix);
        print(postfix);
        clear(postfix);
//        value = evaluatePostfix(postfix);
//        cout << value << endl;
    }
//    printMap();
    return 0;
}
