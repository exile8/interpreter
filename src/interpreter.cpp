#include <iostream>
#include "lexemes.h"
#include "interpreter.h"

using std::cout;
using std::endl;
using std::cerr;

string Parser::getSubcodeline(int n) {
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
        newPolizline.push_back(opers.top());
        opers.pop();
    }
    delete opers.top();
    opers.pop();
}

void Parser::sortOpersRight(Oper *op) {
    int curPriority = op->getPriority();
    while (!opers.empty() && opers.top()->getPriority() > curPriority) {
        newPolizline.push_back(opers.top());
        opers.pop();
    }
    opers.push(op);
}

void Parser::sortOpersLeft(Oper *op) {
    int curPriority = op->getPriority();
    while (!opers.empty() && opers.top()->getPriority() >= curPriority) {
        newPolizline.push_back(opers.top());
        opers.pop();
    }
    opers.push(op);
}

void Parser::emptyOpersStack() {
    while (!opers.empty() && opers.top()->getPriority() >= 0) {
        newPolizline.push_back(opers.top());
        opers.pop();
    }
}

void Parser::freePolizError() {
    emptyOpersStack();
    print(newPolizline);
    freePoliz();
    for (int i = 0; i < (int)newPolizline.size(); i++) {
        if (newPolizline[i] == nullptr) {
            continue;
        }
        delete newPolizline[i];
    }
    newPolizline.clear();
}

bool Parser::getNumber() {
    skipSpaces();
    int number;
    if (isdigit(code[row][position]) == false) {
        return false;
    }
    number = code[row][position] - '0';
    shift(1);
    while (isdigit(code[row][position])) {
        number = number * 10 + code[row][position] - '0';
        shift(1);
    }
    newPolizline.push_back(new Number(number));
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
    if (isalpha(code[row][position]) == false &&
        code[row][position] != '_') {
        return false;
    }
    name.push_back(code[row][position]);
    length++;
    while (isalpha(code[row][position + length]) ||
           isdigit(code[row][position + length]) ||
           code[row][position + length] == '_') {
        name.push_back(code[row][position + length]);
        length++;
    }
    if (isReservedWord(name)) {
        return false;
    }
    shift(length);
    newPolizline.push_back(new Variable(name));
    if (!opers.empty() && opers.top()->getType() == GOTO) {
        if (Goto::LabelTable.count(name) == 0) {
            Goto::LabelTable[name] = UNDEFINED;
        }
        newPolizline.push_back(opers.top());
        opers.pop();
    }
    return true;
}

bool Parser::getAssignOperator() {
    skipSpaces();
    string op = getSubcodeline(2);
    if (op.compare(OPERATOR_STRING[ASSIGN]) == 0) {
        shift(2);
        sortOpersRight(new Assign());
        return true;
    } else {
        return false;
    }
}

bool Parser::getLeftBracket() {
    skipSpaces();
    string op = getSubcodeline(1);
    if (op.compare(OPERATOR_STRING[LBRACKET]) == 0) {
        opers.push(new Binary(LBRACKET));
        shift(1);
        return true;
    } else {
        return false;
    }
}

bool Parser::getRightBracket() {
    skipSpaces();
    string op = getSubcodeline(1);
    if (op.compare(OPERATOR_STRING[RBRACKET]) == 0) {
        buildBracketExpr();
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
        if (PRIORITY[i] <= 0) {
            continue;
        }
        string op = getSubcodeline(OPERATOR_STRING[i].size());
        if (op.compare(OPERATOR_STRING[i]) == 0) {
            sortOpersLeft(new Binary(OPERATOR(i)));
            shift(OPERATOR_STRING[i].size());
            return true;
        }
    }
    return false;
}

bool Parser::initLabel(string name) {
    if (isReservedWord(name)) {
        return false;
    }
    if (Goto::LabelTable.count(name) > 0 && Goto::LabelTable[name] != UNDEFINED) {
        return false;
    } else {
        Goto::LabelTable[name] = row + 1;
        return true;
    }
}

bool Parser::getLabel() {
    skipSpaces();
    string name, op;
    int length = 0;
    if (isalpha(code[row][position]) == false &&
        code[row][position] != '_') {
        return false;
    }
    name.push_back(code[row][position]);
    length++;
    while (isalpha(code[row][position + length]) ||
           isdigit(code[row][position + length]) ||
           code[row][position + length] == '_') {
        name.push_back(code[row][position + length]);
        length++;
    }
    op.push_back(code[row][position + length]);
    if (op.compare(OPERATOR_STRING[COLON]) == 0) {
        name = getSubcodeline(length);
        if (initLabel(name) == false) {
            return false;
        } else {
            newPolizline.push_back(nullptr);
            shift(length + 1);
            return true;
        }
    }
    return false;
}

bool Parser::getGoto() {
    skipSpaces();
    string op = getSubcodeline(4);
    if (op.compare(OPERATOR_STRING[GOTO]) == 0) {
        opers.push(new Goto(GOTO));
        shift(4);
        return true;
    } else {
        return false;
    }
}

bool Parser::getIf() {
    skipSpaces();
    string op = getSubcodeline(2);
    if (op.compare(OPERATOR_STRING[IF]) == 0) {
        opers.push(new Goto(IF));
        shift(2);
        return true;
    } else {
        return false;
    }
}

bool Parser::getElse() {
    skipSpaces();
    string op = getSubcodeline(4);
    if (op.compare(OPERATOR_STRING[ELSE]) == 0) {
        newPolizline.push_back(new Goto(ELSE));
        shift(4);
        return true;
    } else {
        cout << op << endl;
        return false;
    }
}

bool Parser::getWhile() {
    skipSpaces();
    string op = getSubcodeline(5);
    if (op.compare(OPERATOR_STRING[WHILE]) == 0) {
        opers.push(new Goto(WHILE));
        shift(5);
        return true;
    } else {
        return false;
    }
}

bool Parser::getThen() {
    skipSpaces();
    string op = getSubcodeline(4);
    if (op.compare(OPERATOR_STRING[THEN]) == 0) {
        newPolizline.push_back(opers.top());
        opers.pop();
        shift(4);
        return true;
    } else {
        return false;
    }
}

bool Parser::getEndif() {
    skipSpaces();
    string op = getSubcodeline(5);
    if (op.compare(OPERATOR_STRING[ENDIF]) == 0) {
        newPolizline.push_back(nullptr);
        shift(5);
        return true;
    } else {
        return false;
    }
}

bool Parser::getEndwhile() {
    skipSpaces();
    string op = getSubcodeline(8);
    if (op.compare(OPERATOR_STRING[ENDWHILE]) == 0) {
        newPolizline.push_back(new Goto(ENDWHILE));
        shift(8);
        return true;
    } else {
        cout << op << endl;
        cout << "Ha" << endl;
        return false;
    }
}

bool Parser::getLeftQBracket() {
    string op = getSubcodeline(1);
    if (op.compare(OPERATOR_STRING[LQBRACKET]) == 0) {
        opers.push(new Binary(LQBRACKET));
        shift(1);
        return true;
    } else {
        return false;
    }
}

bool Parser::getRightQBracket() {
    string op = getSubcodeline(1);
    if (op.compare(OPERATOR_STRING[RQBRACKET]) == 0) {
        shift(1);
        delete opers.top();
        opers.pop();
        newPolizline.push_back(new Dereference());
        return true;
    } else {
        return false;
    }
}

bool Parser::getExpression() {
    if (getNumber()) {
        if (getBinaryOperator()) {
            return getExpression();
        } else {
            emptyOpersStack();
            return true;
        }
    } else if (getVariable()) {
        if (getAssignOperator() || getBinaryOperator()) {
            return getExpression();
        } else if (getLeftQBracket() && getExpression() && getRightQBracket()) {
            if (getAssignOperator() || getBinaryOperator()) {
                return getExpression();
            } else {
                emptyOpersStack();
                return true;
            }
        } else {
            emptyOpersStack();
            return true;
        }
    } else if (getLeftBracket() && getExpression() && getRightBracket()) {
        if (getBinaryOperator()) {
            return getExpression();
        } else {
            emptyOpersStack();
            return true;
        }
    }
    return false;
}

void Parser::putCommandInPoliz() {
    poliz.push_back(newPolizline);
    print(newPolizline);
    newPolizline.clear();
    row++;
    position = 0;
}

bool Parser::isEndOfLine() {
    if (position == (int)code[row].size()) {
        return true;
    } else {
        return false;
    }
}

bool Parser::getIfBlock() {
    int ifRow;
    if (getIf() && getExpression() && getThen() && isEndOfLine()) {
        ifRow = row;
        putCommandInPoliz();
        if (!getSequenceOfCommands()) {
            if (getElse() && isEndOfLine()) {
                dynamic_cast<Goto *>(poliz[ifRow].back())->setRow(row + 1);
                ifRow = row;
                putCommandInPoliz();
                if (getSequenceOfCommands()) {
                    return false;
                }
            }
            if (getEndif() && isEndOfLine()) {
                putCommandInPoliz();
                dynamic_cast<Goto *>(poliz[ifRow].back())->setRow(row);
                return true;
            }
        }
    }
    return false;
}

bool Parser::getWhileBlock() {
    int whileRow;
    if (getWhile() && getExpression() && getThen() && isEndOfLine()) {
        whileRow = row;
        putCommandInPoliz();
        if (!getSequenceOfCommands() && getEndwhile() && isEndOfLine()) {
            dynamic_cast<Goto *>(newPolizline.front())->setRow(whileRow);
            putCommandInPoliz();
            dynamic_cast<Goto *>(poliz[whileRow].back())->setRow(row);
            return true;
        }
    }
    return false;
}

bool Parser::getCommand() {
    if (((getGoto() && getVariable()) || getLabel() ||
          getExpression()) && isEndOfLine()) {
        putCommandInPoliz();
        return true;
    } else {
        return false;
    }
}

void Parser::freePoliz() {
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
    } else if (getWhileBlock() || getIfBlock() || getCommand()) {
        return getSequenceOfCommands();
    } else {
        return false;
    }
}

bool Parser::buildPoliz(vector<string> code) {
    Parser::code = code;
    row = 0;
    position = 0;
    if (getSequenceOfCommands() && (row == (int)code.size())) {
        return true;
    } else {
        freePolizError();
        cerr << "Syntax error: line " << row + 1 << endl;
        return false;
    }
}

Lexem *currentResult(stack<Lexem *> & eval, Lexem *op) {
    int rightArg;
    Lexem *result;
    Binary *binary = dynamic_cast<Binary *>(op);
    Assign *assign = dynamic_cast<Assign *>(op);
    Dereference *deref = dynamic_cast<Dereference *>(op);
    if (dynamic_cast<Number *>(eval.top())) {
        rightArg = dynamic_cast<Number *>(eval.top())->getValue();
    } else if (dynamic_cast<Variable *>(eval.top())){
        rightArg = dynamic_cast<Variable *>(eval.top())->getValue();
    } else {
        rightArg = dynamic_cast<ArrayElem *>(eval.top())->getValue();
    }
    eval.pop();
    if (eval.empty()) {
        result = new Number(rightArg);
        return result;
    }
    if (assign != nullptr) {
        if (dynamic_cast<Variable *>(eval.top())) {
            Variable *left = dynamic_cast<Variable *>(eval.top());
            result = new Number(assign->getValue(*left, rightArg));
        } else {
            ArrayElem *leftArr = dynamic_cast<ArrayElem *>(eval.top());
            result = new Number(assign->getValue(*leftArr, rightArg));
        }
    } else if (deref != nullptr) {
        string arrName = dynamic_cast<Variable *>(eval.top())->getName();
        result = deref->getValue(arrName, rightArg);
    } else if (dynamic_cast<Number *>(eval.top())) {
        int leftNum = dynamic_cast<Number *>(eval.top())->getValue();
        result = new Number(binary->getValue(leftNum, rightArg));
    } else if (dynamic_cast<Variable *>(eval.top())){
        int leftVar = dynamic_cast<Variable *>(eval.top())->getValue();
        result = new Number(binary->getValue(leftVar, rightArg));
    } else {
        int leftArrElem = dynamic_cast<ArrayElem *>(eval.top())->getValue();
        result = new Number(binary->getValue(leftArrElem, rightArg));
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
        Variable *label = dynamic_cast<Variable *>(eval.top());
        return op->getValue(*label);
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

int evaluatePoliz(vector<Lexem *> poliz, int row) {
    int value, nextRow = row + 1;
    stack<Lexem *> eval;
    vector<Lexem *> temporary;
    for (int i = 0; i < (int)poliz.size(); i++) {
        if (poliz[i] == nullptr) {
            continue;
        } else if (dynamic_cast<Number *>(poliz[i]) ||
                   dynamic_cast<Variable *>(poliz[i])) {
            eval.push(poliz[i]);
        } else if (dynamic_cast<Goto *>(poliz[i])) {
            nextRow = jump(dynamic_cast<Goto *>(poliz[i]), eval, row);
        } else {
            temporary.push_back(currentResult(eval, poliz[i]));
            eval.push(temporary.back());
        }
    }
    if (eval.empty() == false && dynamic_cast<Number *>(eval.top())) {
        value = dynamic_cast<Number *>(eval.top())->getValue();
        cout << value << endl;
    }
    for (int i = 0; i < (int)temporary.size(); i++) {
        delete temporary[i];
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
    map<string, vector<int>>::iterator it2;
    cout << "--------Variables--------" << endl;
    for (it = Variable::VarTable.begin(); it != Variable::VarTable.end(); it++) {
        cout << it->first << " = " << it->second << endl;
    }
    cout << "-------------------------" << endl;
    cout << "----------Arrays---------" << endl;
    for (it2 = ArrayElem::ArrayTable.begin(); it2 != ArrayElem::ArrayTable.end(); it2++) {
        cout << it2->first << ": ";
        for (int i = 0; i < (int)ArrayElem::ArrayTable[it2->first].size(); i++) {
            cout << "[" << ArrayElem::ArrayTable[it2->first][i] << "] ";
        }
        cout << endl;
    }
    cout << "-------------------------" << endl;
}
