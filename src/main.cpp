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

void appendOper(vector <Lexem *> & infix, char symbol) {
    for (size_t i = 0; i < sizeof(OPERATOR_STRING); i++) {
        if (symbol == OPERATOR_STRING[i]) {
            infix.push_back(new Oper(OPERATOR(i)));
        }
    }
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
            appendOper(infix, *it);
        }
    }
    if (prevSymIsDigit) {
        infix.push_back(new Number(number));
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

void sortOpers(vector<Lexem *> & postfix, stack<Lexem *> & opers, 
                                                int curPriority) {
    while (!opers.empty() && 
           dynamic_cast<Oper *>(opers.top())->getPriority() >=
                                                 curPriority) {
        postfix.push_back(opers.top());
        opers.pop();
    }
}

vector<Lexem *> buildPostfix(vector<Lexem *> infix) {
    vector<Lexem *> postfix;
    vector<Lexem *>::iterator it;
    stack <Lexem *> opers;
    for (it = infix.begin(); it != infix.end(); it++) {
        if (dynamic_cast<Number *>(*it)) {
            postfix.push_back(*it);
        } else {
            switch (dynamic_cast<Oper *>(*it)->getType()) {
                case LBRACKET:
                    opers.push(*it);
                    break;
                case RBRACKET:
                    delete *it;
                    buildBracketExpr(postfix, opers);
                    break;
                default:
                    sortOpers(postfix, opers,
                        dynamic_cast<Oper *>(*it)->getPriority());
                    opers.push(*it);
                    break;
            }
        }
    }
    while (!opers.empty()) {
        postfix.push_back(opers.top());
        opers.pop();
    }
    return postfix;
}

Number * currentResult(stack <Lexem *> & eval, Oper *operation) {
    Number *rightArg, *leftArg, *result;
    rightArg = dynamic_cast<Number *>(eval.top());
    eval.pop();
    leftArg = dynamic_cast<Number *>(eval.top());
    eval.pop();
    result = new Number(operation->getValue(*leftArg, *rightArg));
    delete rightArg;
    delete leftArg;
    delete operation;
    return result;
}

int evaluatePostfix(vector<Lexem *> postfix) {
    int value;
    stack<Lexem *> eval;
    vector<Lexem *>::iterator it;
    for (it = postfix.begin(); it != postfix.end(); it++) {
        if (dynamic_cast<Number *>(*it)) {
            eval.push(*it);
        } else {
            eval.push(currentResult(eval, dynamic_cast<Oper *>(*it)));
        }
    }
    value = dynamic_cast<Number *>(eval.top())->getValue();
    delete eval.top();
    return value;
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
