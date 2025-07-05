#include <iostream>
#include <cstring>
#include "token.h"
#include "scanner.h"

using namespace std;

Scanner::Scanner(const char* s):input(s),first(0), current(0) { }


bool is_white_space(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

Token* Scanner::nextToken() {
    Token* token;
    while (current < input.length() &&  is_white_space(input[current]) ) current++;
    if (current >= input.length()) return new Token(Token::END);
    char c  = input[current];
    first = current;
    
    if (isdigit(c)) {
        current++;
        while (current < input.length() && isdigit(input[current]))
            current++;
        token = new Token(Token::NUM, input, first, current - first);
    }
    else if (c == '"') {
        // String literal handling
        current++;
        while (current < input.length() && input[current] != '"') {
            if (input[current] == '\\' && current + 1 < input.length()) {
                // Skip escaped characters
                current += 2;
            } else {
                current++;
            }
        }
        if (current >= input.length()) {
            token = new Token(Token::ERR, "Unterminated string literal", 0, 22);
        } else {
            current++; // Skip closing quote
            token = new Token(Token::STRING_LIT, input, first + 1, current - first - 2);
        }
    }
    else if (isalpha(c) || c == '_') {
        current++;
        while (current < input.length() && (isalnum(input[current]) || input[current] == '_'))
            current++;
        string word = input.substr(first, current - first);
        
        // Go keywords
        if (word == "package") {
            token = new Token(Token::PACKAGE, word, 0, word.length());
        } else if (word == "import") {
            token = new Token(Token::IMPORT, word, 0, word.length());
        } else if (word == "func") {
            token = new Token(Token::FUNC, word, 0, word.length());
        } else if (word == "var") {
            token = new Token(Token::VAR, word, 0, word.length());
        } else if (word == "type") {
            token = new Token(Token::TYPE, word, 0, word.length());
        } else if (word == "struct") {
            token = new Token(Token::STRUCT, word, 0, word.length());
        } else if (word == "return") {
            token = new Token(Token::RETURN, word, 0, word.length());
        } else if (word == "if") {
            token = new Token(Token::IF, word, 0, word.length());
        } else if (word == "else") {
            token = new Token(Token::ELSE, word, 0, word.length());
        } else if (word == "for") {
            token = new Token(Token::FOR, word, 0, word.length());
        } else if (word == "true") {
            token = new Token(Token::TRUE, word, 0, word.length());
        } else if (word == "false") {
            token = new Token(Token::FALSE, word, 0, word.length());
        } else if (word == "print") { // Keep for compatibility
            token = new Token(Token::PRINT, word, 0, word.length());
        } else {
            token = new Token(Token::ID, word, 0, word.length());
        }
    }
    else if (strchr("+-*/()=;,<>!.{}[]:%&|", c)) {
        switch(c) {
            case '+':
                if (current + 1 < input.length()) {
                    if (input[current + 1] == '+') {
                        token = new Token(Token::INC, "++", 0, 2);
                        current++;
                    } else if (input[current + 1] == '=') {
                        token = new Token(Token::PLUSEQ, "+=", 0, 2);
                        current++;
                    } else {
                        token = new Token(Token::PLUS, c);
                    }
                } else {
                    token = new Token(Token::PLUS, c);
                }
                break;
                
            case '-':
                if (current + 1 < input.length()) {
                    if (input[current + 1] == '-') {
                        token = new Token(Token::DEC, "--", 0, 2);
                        current++;
                    } else if (input[current + 1] == '=') {
                        token = new Token(Token::MINUSEQ, "-=", 0, 2);
                        current++;
                    } else {
                        token = new Token(Token::MINUS, c);
                    }
                } else {
                    token = new Token(Token::MINUS, c);
                }
                break;
                
            case '*':
                if (current + 1 < input.length() && input[current + 1] == '=') {
                    token = new Token(Token::MULEQ, "*=", 0, 2);
                    current++;
                } else {
                    token = new Token(Token::MUL, c);
                }
                break;
                
            case '/':
                if (current + 1 < input.length() && input[current + 1] == '=') {
                    token = new Token(Token::DIVEQ, "/=", 0, 2);
                    current++;
                } else {
                    token = new Token(Token::DIV, c);
                }
                break;
                
            case '%':
                if (current + 1 < input.length() && input[current + 1] == '=') {
                    token = new Token(Token::MODEQ, "%=", 0, 2);
                    current++;
                } else {
                    token = new Token(Token::MOD, c);
                }
                break;
                
            case ',': token = new Token(Token::COMA, c); break;
            case '(': token = new Token(Token::PI, c); break;
            case ')': token = new Token(Token::PD, c); break;
            case '{': token = new Token(Token::LBRACE, c); break;
            case '}': token = new Token(Token::RBRACE, c); break;
            case '[': token = new Token(Token::LBRACKET, c); break;
            case ']': token = new Token(Token::RBRACKET, c); break;
            case '.': token = new Token(Token::DOT, c); break;
            case ':':
                if (current + 1 < input.length() && input[current + 1] == '=') {
                    token = new Token(Token::SHORTASSIGN, ":=", 0, 2);
                    current++;
                } else {
                    token = new Token(Token::COLON, c);
                }
                break;
                
            case '=':
                if (current + 1 < input.length() && input[current + 1] == '=') {
                    token = new Token(Token::EQ, "==", 0, 2);
                    current++;
                } else {
                    token = new Token(Token::ASSIGN, c);
                }
                break;
                
            case '<':
                if (current + 1 < input.length() && input[current + 1] == '=') {
                    token = new Token(Token::LE, "<=", 0, 2);
                    current++;
                } else {
                    token = new Token(Token::LT, c);
                }
                break;
                
            case '>':
                if (current + 1 < input.length() && input[current + 1] == '=') {
                    token = new Token(Token::GE, ">=", 0, 2);
                    current++;
                } else {
                    token = new Token(Token::GT, c);
                }
                break;
                
            case '!':
                if (current + 1 < input.length() && input[current + 1] == '=') {
                    token = new Token(Token::NE, "!=", 0, 2);
                    current++;
                } else {
                    token = new Token(Token::NOT, c);
                }
                break;
                
            case '&':
                if (current + 1 < input.length() && input[current + 1] == '&') {
                    token = new Token(Token::AND, "&&", 0, 2);
                    current++;
                } else {
                    token = new Token(Token::ERR, c);
                }
                break;
                
            case '|':
                if (current + 1 < input.length() && input[current + 1] == '|') {
                    token = new Token(Token::OR, "||", 0, 2);
                    current++;
                } else {
                    token = new Token(Token::ERR, c);
                }
                break;
                
            case ';': token = new Token(Token::PC, c); break;
            
            default:
                cout << "No debería llegar acá: " << c << endl;
                token = new Token(Token::ERR, c);
        }
        current++;
    }
    else {
        token = new Token(Token::ERR, c);
        current++;
    }
    return token;
}

void Scanner::reset() {
    first = 0;
    current = 0;
}

Scanner::~Scanner() { }

void test_scanner(Scanner* scanner) {
    Token* current;
    cout << "Iniciando Scanner:" << endl<< endl;
    while ((current = scanner->nextToken())->type != Token::END) {
        if (current->type == Token::ERR) {
            cout << "Error en scanner - carácter inválido: " << current->text << endl;
            break;
        } else {
            cout << *current << endl;
        }
        delete current;
    }
    cout << "TOKEN(END)" << endl;
    delete current;
}