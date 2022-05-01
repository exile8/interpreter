#include "lexemes.h"

using std::cout;
using std::endl;
using std::cerr;

#define UNDEFINED -1

Lexem::Lexem() {
}

Lexem::~Lexem() {
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
    return VarTable[name];
}

void Variable::setValue(int value) const {
    VarTable[name] = value;
}

ArrayElem::ArrayElem(string name, int index) {
    ArrayElem::name = name;
    ArrayElem::index = index;
}

int ArrayElem::getValue() const {
    return ArrayTable[name][index];
}

void ArrayElem::setValue(int value) const {
    if ((int)ArrayTable[name].size() < index + 1) {
        ArrayTable[name].resize(index + 1);
    }
    ArrayTable[name][index] = value;
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

Binary::Binary(OPERATOR opertype) : Oper(opertype) {
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

Assign::Assign() : Oper(ASSIGN) {
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

int Assign::getValue(const ArrayElem & left, int right) const {
    if (getType() == ASSIGN) {
        left.setValue(right);
        return right;
    } else {
        cerr << "Error: invalid operation" << endl;
        return -1;
    }
}

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
        return LabelTable[var.getName()];
    } else {
        cerr << "Error: invalid operation" << endl;
        return -1;
    }
}

Dereference::Dereference() : Oper(DEREF) {
}

ArrayElem *Dereference::getValue(string name, int index) const {
    ArrayElem *elem = new ArrayElem(name, index);
    return elem;
}

map<string, int> Variable::VarTable;
map<string, vector<int>> ArrayElem::ArrayTable;
map<string, int> Goto::LabelTable;
