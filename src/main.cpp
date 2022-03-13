#include <iostream>
#include <string>
#include <vector>
#include <stack>

using std::getline;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::stack;

enum OPERATOR {
    LBRACKET, RBRACKET,
    ASSIGN,
    PLUS, MINUS,
    MULTIPLY
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

class Oper : public Lexem {
    OPERATOR opertype;
public:
    Oper(OPERATOR opertype);
    OPERATOR getType() const;
    int getPriority() const;
    int getValue(const Number & left, const Number & right) const;
};

Number::Number(int value) {
    Number::value = value;
}

int Number::getValue() const {
    return value;
}

Variable::Variable(const string & name) {
    Variable::name = name;
}

string Variable::getName() const {
    return name;
}

int Variable::getValue() const {
    return value;
}

void Variable::setValue(int value) {
    Variable::value = value;
}

Oper::Oper(OPERATOR opertype) {
    Oper::opertype = opertype;
}

OPERATOR Oper::getType() const {
    return opertype;
}

int Oper::getPriority() const {
    switch (opertype) {
        case LBRACKET:
            return PRIORITY[0];
        case RBRACKET:
            return PRIORITY[1];
        case ASSIGN:
            return PRIORITY[2];
        case PLUS:
            return PRIORITY[3];
        case MINUS:
            return PRIORITY[4];
        case MULTIPLY:
            return PRIORITY[5];
    }
    return -1;
}

int Oper::getValue(const Variable & left, const Number & right) const {
    switch (opertype) {
        case PLUS:
            return left.getValue() + right.getValue();
        case MINUS:
            return left.getValue() - right.getValue();
        case MULTIPLY:
            return left.getValue() * right.getValue();
        default:
            break;
    }
    return -1;
}

bool isDigit(char symbol) {
    return symbol >= '0' && symbol <= '9';
}

vector<Lexem *> parseLexem(string codeline) {
    vector<Lexem *> result;
    string::iterator it;
    int number = 0, prevSymIsDigit = 0;

    for (it = codeline.begin(); it != codeline.end(); it++) {
        if (isDigit(*it)) {
            number = number * 10 + *it - '0';
            prevSymIsDigit = 1;
        } else if (*it != ' ' && *it != '\t'){
            if (prevSymIsDigit) {
                result.push_back(new Number(number));
                number = 0;
            }
            prevSymIsDigit = 0;
            switch (*it) {
                case '(':
                    result.push_back(new Oper(LBRACKET));
                    break;
                case ')':
                    result.push_back(new Oper(RBRACKET));
                    break;
                case '+':
                    result.push_back(new Oper(PLUS));
                    break;
                case '-':
                    result.push_back(new Oper(MINUS));
                    break;
                case '*':
                    result.push_back(new Oper(MULTIPLY));
                    break;
            }
        }
    }
    if (prevSymIsDigit) {
        result.push_back(new Number(number));
    }
    return result;
}

vector<Lexem *> buildPostfix(vector<Lexem *> infix) {
    vector<Lexem *> result;
    vector<Lexem *>::iterator it;
    stack <Lexem *> opers;
    Oper *currentTop;
    for (it = infix.begin(); it != infix.end(); it++) {
        if (dynamic_cast<Number *>(*it) || dynamic_cast<Variable *>(*it)) {
            result.push_back(*it);
        } else {
            switch (dynamic_cast<Oper *>(*it)->getType()) {
                case LBRACKET:
                    opers.push(*it);
                    break;
                case RBRACKET:
                    delete *it;
                    currentTop = dynamic_cast<Oper *>(opers.top());
                    while (currentTop->getType() != LBRACKET) {
                        result.push_back(currentTop);
                        opers.pop();
                        currentTop = dynamic_cast<Oper *>(opers.top());
                    }
                    delete opers.top();
                    opers.pop();
                    break;
                default:
                    currentTop = dynamic_cast<Oper *>(opers.top());
                    while (!opers.empty() && currentTop->getPriority() >=
                           dynamic_cast<Oper *>(*it)->getPriority()) {
                        result.push_back(currentTop);
                        opers.pop();
                        currentTop = dynamic_cast<Oper *>(opers.top());
                    }
                    opers.push(*it);
            }
        }
    }
    while (!opers.empty()) {
        result.push_back(opers.top());
        opers.pop();
    }
    return result;
}

int evaluatePostfix(vector<Lexem *> postfix) {
    int result;
    stack<Lexem *> eval;
    Number *leftArg, *rightArg;
    vector<Lexem *>::iterator it;
    for (it = postfix.begin(); it != postfix.end(); it++) {
        if (dynamic_cast<Number *>(*it) || dynamic_cast<Variable *>(*it)) {
            eval.push(*it);
        } else {
            rightArg = dynamic_cast<Number *>(eval.top());
            eval.pop();
            leftArg = dynamic_cast<Number *>(eval.top());
            eval.pop();
            eval.push(new Number(dynamic_cast<Oper *>(*it)->getValue(*leftArg, *rightArg)));
            delete rightArg;
            delete leftArg;
            delete *it;
        }
    }
    result = dynamic_cast<Number *>(eval.top())->getValue();
    delete eval.top();
    return result;
}

void print(vector<Lexem *> v) {
    vector<Lexem *>::iterator it;
    for (it = v.begin(); it != v.end(); it++) {
        if (dynamic_cast<Number *>(*it)) {
            cout << dynamic_cast<Number *>(*it)->getValue();
        } else if (dynamic_cast<Oper *>(*it)) {
            switch (dynamic_cast<Oper *>(*it)->getType()) {
                case PLUS:
                    cout << '+';
                    break;
                case ASSIGN:
                    cout << '=';
                    break;
                case MINUS:
                    cout << '-';
                    break;
                case MULTIPLY:
                    cout << '*';
                    break;
                case LBRACKET:
                    cout << '(';
                    break;
                case RBRACKET:
                    cout << ')';
                    break;
            }
        } else {
            cout << dynamic_cast<Variable *>(*it)->getName();
        }
    }
    cout << endl;
}

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
    int value;

    while (getline(cin, codeline)) {
        infix = parseLexem(codeline);
        print(infix);
        postfix = buildPostfix(infix);
        print(postfix);
        value = evaluatePostfix(postfix);
        cout << value << endl;
    }
    return 0;
}
