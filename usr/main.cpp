#include "lexemes.h"
#include "interpreter.h"

using std::cin;
using std::getline;

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
            i = evaluatePoliz(parser.poliz[i], i);
            printMap();
        }
        parser.freePoliz();
    }
    return 0;
}
