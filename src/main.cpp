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
    "==", "=",
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
    int getValue(int left, int right) const;
    int getValue(const Variable & left, int right) const;
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

int Oper::getValue(int left, int right) const {
    switch (opertype) {
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
            break;
    }
    return -1;
}

int Oper::getValue(const Variable & left, int right) const {
    switch (opertype) {
        case OR:
            return left.getValue() || right;
        case AND:
            return left.getValue()  && right;
        case BITOR:
            return left.getValue() | right;
        case XOR:
            return left.getValue() ^ right;
        case BITAND:
            return left.getValue() & right;
        case EQ:
            return left.getValue() == right;
        case NEQ:
            return left.getValue() != right;
        case LEQ:
            return left.getValue() <= right;
        case SHL:
            return left.getValue() << right;
        case LT:
            return left.getValue() < right;
        case GEQ:
            return left.getValue() >= right;
        case SHR:
            return left.getValue() >> right;
        case GT:
            return left.getValue() > right;
        case PLUS:
            return left.getValue() + right;
        case MINUS:
            return left.getValue() - right;
        case MULT:
            return left.getValue() * right;
        case DIV:
            return left.getValue() / right;
        case MOD:
            return left.getValue() % right;
        case ASSIGN:
            left.setValue(right);
            return left.getValue();
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

string getSubstring(string::iterator & it, size_t size, const string::iterator & end) {
    string substring;
    for (size_t i = 0; i < size && (it + i) != end; i++) {
        substring.push_back(*(it + i));
    }
    return substring;
}

OPERATOR readOper(string::iterator & it, const string::iterator & end) {
    OPERATOR opertype;
    size_t n = sizeof(OPERATOR_STRING) / sizeof(string);
    for (size_t i = 0; i < n; i++) {
        string subCodeline = getSubstring(it, OPERATOR_STRING[i].size(), end);
        if (subCodeline.compare(OPERATOR_STRING[i]) == 0) {
            opertype = OPERATOR(i);
            it += OPERATOR_STRING[i].size() - 1;
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
            infix.push_back(new Oper(readOper(it, codeline.end())));
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

Number *currentResult(stack<Lexem *> & eval, Oper *operation) {
    int rightArg;
    Number *result;
    if (dynamic_cast<Number *>(eval.top())) {
        rightArg = dynamic_cast<Number *>(eval.top())->getValue();
    } else {
        rightArg = dynamic_cast<Variable *>(eval.top())->getValue();
    }
    delete eval.top();
    eval.pop();
    if (dynamic_cast<Number *>(eval.top())) {
        int leftArg = dynamic_cast<Number *>(eval.top())->getValue();
        result = new Number(operation->getValue(leftArg, rightArg));
    } else {
        Variable *left = dynamic_cast<Variable *>(eval.top());
        result = new Number(operation->getValue(*left, rightArg));
    }
    delete eval.top();
    eval.pop();
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
        printMap();
    }
    return 0;
}
