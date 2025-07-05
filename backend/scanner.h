#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <unordered_map>
#include "token.h"

class Scanner {
private:
    std::string input;
    int first, current;
    std::unordered_map<std::string, Token::Type> keywords;
public:
    Scanner(const char* in_s);
    Token* nextToken();
    void reset();
    ~Scanner();
};

void test_scanner(Scanner* scanner);

#endif // SCANNER_H