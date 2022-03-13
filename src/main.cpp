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
    PLUS, MINUS,
    MULTIPLY
};

char OPERATOR_STRING[] = {
    '(', ')',
    '+', '-',
    '*'
};

int PRIORITY[] = {
    -1, -1,
    0, 0,
    1
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

Oper::Oper(OPERATOR opertype) {
    Oper::opertype = opertype;
}

OPERATOR Oper::getType() const {
    return opertype;
}

int Oper::getPriority() const {
    for (size_t i = 0; i < sizeof(OPERATOR_STRING); i++) {
        if (opertype == OPERATOR(i)) {
            return PRIORITY[i];
        }
    }
    return -1;
}

int Oper::getValue(const Number & left, const Number & right) const {
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
    vector<Lexem *> infix;
    string::iterator it;
    int number = 0, prevSymIsDigit = 0;

    for (it = codeline.begin(); it != codeline.end(); it++) {
        if (*it == ' ' && *it == '\t') {
            continue;
        } else if (isDigit(*it)) {
            number = number * 10 + *it - '0';
            prevSymIsDigit = 1;
        } else {
            if (prevSymIsDigit) {
                infix.push_back(new Number(number));
                number = 0;
            }
            prevSymIsDigit = 0;
            for (size_t i = 0; i < sizeof(OPERATOR_STRING); i++) {
                if (*it == OPERATOR_STRING[i]) {
                    infix.push_back(new Oper(OPERATOR(i)));
                }
            }
        }
    }
    if (prevSymIsDigit) {
        infix.push_back(new Number(number));
    }
    return infix;
}

vector<Lexem *> buildPostfix(vector<Lexem *> infix) {
    vector<Lexem *> result;
    vector<Lexem *>::iterator it;
    stack <Lexem *> opers;
    for (it = infix.begin(); it != infix.end(); it++) {
        if (dynamic_cast<Number *>(*it)) {
            result.push_back(*it);
        } else {
            switch (dynamic_cast<Oper *>(*it)->getType()) {
                case LBRACKET:
                    opers.push(*it);
                    break;
                case RBRACKET:
                    delete *it;
                    while (dynamic_cast<Oper *>(opers.top())->getType() != LBRACKET) {
                        result.push_back(dynamic_cast<Oper *>(opers.top()));
                        opers.pop();
                    }
                    delete opers.top();
                    opers.pop();
                    break;
                default:
                    while (!opers.empty() && dynamic_cast<Oper *>(opers.top())->getPriority() >=
                           dynamic_cast<Oper *>(*it)->getPriority()) {
                        result.push_back(dynamic_cast<Oper *>(opers.top()));
                        opers.pop();
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
        if (dynamic_cast<Number *>(*it)) {
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
        } else {
            switch (dynamic_cast<Oper *>(*it)->getType()) {
                case PLUS:
                    cout << '+';
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
