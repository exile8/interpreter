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
map<string, int> labels;

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
    int row;
public:
    Goto(OPERATOR opertype);
    void setRow(int row);
    int getRow();
    int getValue(const Variable & var) const;
};

Goto::Goto(OPERATOR opertype) : Oper(opertype) {
    row = UNDEFINED;
}

void Goto::setRow(int row) {
    Goto::row = row;
}

int Goto::getRow() {
    return row;
}

int Goto::getValue(const Variable & var) const {
    if (getType() == GOTO) {
        return labels[var.getName()];
    } else {
        cerr << "Error: invalid operation" << endl;
        return -1;
    }
}

class Parser {
    vector<string> code;
    int row;
    int position;

    string subcodeline(int n);
    void shift(int n);
    void skipSpaces();

    stack<Oper *> opers;
    vector<Lexem *> polizline;

    bool getSequenceOfCommands();
    bool getCommand();
    bool getExpression();
    bool endOfLine();
    bool isReservedWord(string word);

    bool getNumber();
    bool getVariable();

    bool getAssignOperator();
    bool getBinaryOperator();
    bool getLeftBracket();
    bool getRightBracket();

    bool getColon();
    bool initLabel();
    bool getGoto();
    bool getIfBlock();
    bool getElseBlock();
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
    void putOperInPoliz(Oper *op);
    void freeStack();
    void clear();
public:
    vector<vector<Lexem *>> poliz;
    bool buildPoliz(vector<string> code);
    void erase();
};

string Parser::subcodeline(int n) {
    string str;
    for (int i = 0; i < n && position + i < (int)code[row].size(); i++) {
        str.push_back(code[row][position + i]);
    }
    return str;
}

void Parser::shift(int n) {
    position += n;
}

void Parser::skipSpaces() {
    while (code[row][position] == ' ' || code[row][position] == '\t') {
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
    while (!opers.empty() && opers.top()->getPriority() >= 0) {
        polizline.push_back(opers.top());
        opers.pop();
    }
}

void Parser::clear() {
    freeStack();
    print(polizline);
    for (int i = 0; i < (int)poliz.size(); i++) {
        for (int j = 0; j < (int)poliz[i].size(); j++) {
            if (poliz[i][j] == nullptr) {
                continue;
            }
            delete poliz[i][j];
        }
        poliz[i].clear();
    }
    for (int i = 0; i < (int)polizline.size(); i++) {
        if (polizline[i] == nullptr) {
            continue;
        }
        delete polizline[i];
    }
    polizline.clear();
    poliz.clear();
}

bool Parser::getNumber() {
    skipSpaces();
    int number = code[row][position] - '0';
    if (isdigit(code[row][position])) {
        shift(1);
    } else {
        return false;
    }
    while (isdigit(code[row][position])) {
        number = number * 10 + code[row][position] - '0';
        shift(1);
    }
    polizline.push_back(new Number(number));
    return true;
}

bool Parser::isReservedWord(string word) {
    int n = (int)sizeof(RESERVED) / sizeof(string);
    for (int i = 0; i < n; i++) {
        if (word.compare(RESERVED[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool Parser::getVariable() {
    skipSpaces();
    string name;
    int length = 0;
    if (isalpha(code[row][position]) || code[row][position] == '_') {
        length++;
    } else {
        return false;
    }
    while (isalpha(code[row][position + length]) || isdigit(code[row][position + length]) ||
           code[row][position + length] == '_') {
           length++;
    }
    name = subcodeline(length);
    if (isReservedWord(name)) {
        return false;
    }
    shift(length);
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
    if (code[row][position] == '(') {
        putOperInPoliz(new Binary(LBRACKET));
        shift(1);
        return true;
    } else {
        return false;
    }
}

bool Parser::getRightBracket() {
    skipSpaces();
    if (code[row][position] == ')') {
        putOperInPoliz(new Binary(RBRACKET));
        shift(1);
        return true;
    } else {
        return false;
    }
}

bool Parser::getBinaryOperator() {
    int n = (int)sizeof(OPERATOR_STRING) / sizeof(string);
    skipSpaces();
    for (int i = 0; i < n; i++) {
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
    if (labels.count(name) > 0 && labels[name] != UNDEFINED) {
        return false;
    } else {
        labels[name] = row + 1;
        return true;
    }
}

bool Parser::getColon() {
    skipSpaces();
    if (code[row][position] == ':') {
        if (initLabel() == false) {
            return false;
        }
        delete polizline.back();
        polizline.pop_back();
        polizline.push_back(nullptr);
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
        opers.push(new Goto(GOTO));
        shift(4);
        return true;
    } else {
        return false;
    }
}

bool Parser::getIf() {
    skipSpaces();
    string op = subcodeline(2);
    if (op.compare("if") == 0) {
        opers.push(new Goto(IF));
        shift(2);
        return true;
    } else {
        return false;
    }
}

bool Parser::getElse() {
    skipSpaces();
    string op = subcodeline(4);
    if (op.compare("else") == 0 && !opers.top() && opers.top()->getType() == IF) {
        dynamic_cast<Goto *>(opers.top())->setRow(row + 1);
        opers.pop();
        opers.push(new Goto(ELSE));
        polizline.push_back(opers.top());
        shift(4);
        return true;
    } else {
        return false;
    }
}

bool Parser::getWhile() {
    skipSpaces();
    string op = subcodeline(5);
    if (op.compare("while") == 0) {
        Goto *newWhile = new Goto(WHILE);
        newWhile->setRow(row);
        opers.push(newWhile);
        shift(5);
        return true;
    } else {
        return false;
    }
}

bool Parser::getThen() {
    skipSpaces();
    string op = subcodeline(4);
    if (op.compare("then") == 0) {
        polizline.push_back(opers.top());
        shift(4);
        return true;
    } else {
        return false;
    }
}

bool Parser::getEndif() {
    skipSpaces();
    string op = subcodeline(5);
    if (op.compare("endif") == 0 && !opers.empty() &&
    (opers.top()->getType() == IF || opers.top()->getType() == ELSE)) {
        dynamic_cast<Goto *>(opers.top())->setRow(row + 1);
        opers.pop();
        polizline.push_back(nullptr);
        shift(5);
        return true;
    } else {
        return false;
    }
}

bool Parser::getEndwhile() {
    skipSpaces();
    string op = subcodeline(8);
    if (op.compare("endwhile") == 0 && !opers.empty() && opers.top()->getType() == WHILE) {
        Goto *endwhile = new Goto(ENDWHILE);
        int loopRow = dynamic_cast<Goto *>(opers.top())->getRow();
        endwhile->setRow(loopRow);
        dynamic_cast<Goto *>(opers.top())->setRow(row + 1);
        opers.pop();
        polizline.push_back(endwhile);
        shift(8);
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
            freeStack();
            return true;
        }
    } else if (getGoto()) {
        return getVariable();
    } else if (getVariable()) {
        if (getLeftBracket()) {
            return false;
        } else if (getAssignOperator() || getBinaryOperator()) {
            return getExpression();
        } else if (getColon()) {
            return true;
        } else {
            freeStack();
            return true;
        }
    } else if (getLeftBracket() && getExpression() && getRightBracket()) {
        if (getAssignOperator() || getLeftBracket()) {
            return false;
        } else if (getBinaryOperator()) {
            return getExpression();
        } else {
            freeStack();
            return true;
        }
    }
    return false;
}

void Parser::putCommandInPoliz() {
    poliz.push_back(polizline);
    print(polizline);
    polizline.clear();
    row++;
    position = 0;
}

bool Parser::endOfLine() {
    if (position == (int)code[row].size()) {
        return true;
    } else {
        return false;
    }
}

bool Parser::getElseBlock() {
    if (getElse() && endOfLine()) {
        putCommandInPoliz();
        if (getSequenceOfCommands() && getEndif() && endOfLine()) {
            putCommandInPoliz();
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool Parser::getIfBlock() {
    if (getIf() && getExpression() && getThen() && endOfLine()) {
        putCommandInPoliz();
        if (getSequenceOfCommands()) {
            if (getElseBlock()) {
                return true;
            } else if (getEndif() && endOfLine()) {
                putCommandInPoliz();
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool Parser::getWhileBlock() {
    if (getWhile() && getExpression() && getThen() && endOfLine()) {
        putCommandInPoliz();
        if (getSequenceOfCommands() && getEndwhile() && endOfLine()) {
            putCommandInPoliz();
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool Parser::getCommand() {
    if (getExpression() && endOfLine()) {
        putCommandInPoliz();
        return true;
    } else {
        return false;
    }
}

void Parser::erase() {
    for (int i = 0; i < (int)poliz.size(); i++) {
        for (int j = 0; j < (int)poliz[i].size(); j++) {
            delete poliz[i][j];
        }
        poliz[i].clear();
    }
    poliz.clear();
}


bool Parser::getSequenceOfCommands() {
    if (row == (int)code.size()) {
        return true;
    } else if (getWhileBlock() || getIfBlock()) {
        return getSequenceOfCommands();
    } else if (getCommand()){
        return getSequenceOfCommands();
    } else {
        return true;
    }
}

bool Parser::buildPoliz(vector<string> code) {
    Parser::code = code;
    row = 0;
    position = 0;
    if (getSequenceOfCommands() && (row == (int)code.size())) {
        return true;
    } else {
        clear();
        cerr << "Syntax error: line " << row + 1 << endl;
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
    int n = (int)sizeof(OPERATOR_STRING) / sizeof(string);
    for (int i = 0; i < n; i++) {
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

bool getCondition(Lexem *condition) {
    Variable *var = dynamic_cast<Variable *>(condition);
    Number *num = dynamic_cast<Number *>(condition);
    if (var) {
        return !(var->getValue() == 0);
    }
    return !(num->getValue() == 0);
}

int jump(Goto *op, stack<Lexem *> & eval, size_t row) {
    OPERATOR type = op->getType();
    bool condition = false;
    if (type == GOTO) {
        return op->getValue(*(dynamic_cast<Variable *>(eval.top())));
    }
    if (eval.empty() == false) {
        condition = getCondition(eval.top());
    }
    if (type == IF || type == WHILE) {
        if (condition == false) {
            return op->getRow();
        } else {
            return row + 1;
        }
    }
    return op->getRow();
}

int evaluatePostfix(vector<Lexem *> postfix, int row) {
    int value, nextRow = row + 1;
    stack<Lexem *> eval;
    vector<Lexem *>::iterator it;
    vector<Number *> intermediate;
    for (it = postfix.begin(); it != postfix.end(); it++) {
        if (*it == nullptr) {
            continue;
        } else if (dynamic_cast<Number *>(*it) || dynamic_cast<Variable *>(*it)) {
            eval.push(*it);
        } else if (dynamic_cast<Goto *>(*it)) {
            nextRow = jump(dynamic_cast<Goto *>(*it), eval, row);
        } else {
            intermediate.push_back(currentResult(eval, dynamic_cast<Binary *>(*it),
                                        dynamic_cast<Assign *>(*it)));
            eval.push(intermediate.back());
        }
    }
    if (eval.empty() == false && dynamic_cast<Number *>(eval.top())) {
        value = dynamic_cast<Number *>(eval.top())->getValue();
        cout << value << endl;
        delete eval.top();
    }
    for (int i = 0; i < (int)intermediate.size() - 1; i++) {
        delete intermediate[i];
    }
    return nextRow;
}

void print(vector<Lexem *> v) {
    vector<Lexem *>::iterator it;
    int n = (int)sizeof(OPERATOR_STRING) / sizeof(string);
    for (it = v.begin(); it != v.end(); it++) {
        if (*it == nullptr) {
            continue;
        } else if (dynamic_cast<Number *>(*it)) {
            cout << "[" <<dynamic_cast<Number *>(*it)->getValue() << "] ";
        } else if (dynamic_cast<Variable *>(*it)) {
            cout << "[" << dynamic_cast<Variable *>(*it)->getName() << "] ";
        } else {
            for (int i = 0; i < n; i++) {
                if (dynamic_cast<Oper *>(*it)->getType() == OPERATOR(i)) {
                    cout << "[" << OPERATOR_STRING[i] << "] ";
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
        int i = 0;
        while (i < (int)code.size()) {
            i = evaluatePostfix(parser.poliz[i], i);
            printMap();
        }
    }
    parser.erase();
    return 0;
}
