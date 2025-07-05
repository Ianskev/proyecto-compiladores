#include <iostream>
#include <cstring>
#include <unordered_map>
#include "token.h"
#include "scanner.h"

using namespace std;

Scanner::Scanner(const char* s):input(s),first(0), current(0) { 
    // Mapa de palabras reservadas para Go
    keywords["package"] = Token::PACKAGE;
    keywords["main"] = Token::MAIN;
    keywords["import"] = Token::IMPORT;
    keywords["var"] = Token::VAR;
    keywords["type"] = Token::TYPE;
    keywords["func"] = Token::FUNC;
    keywords["struct"] = Token::STRUCT;
    keywords["if"] = Token::IF;
    keywords["else"] = Token::ELSE;
    keywords["for"] = Token::FOR;
    keywords["return"] = Token::RETURN;
    keywords["true"] = Token::TRUE;
    keywords["false"] = Token::FALSE;
    keywords["int"] = Token::ID; // tipos básicos como identificadores por simplicidad
    keywords["string"] = Token::ID;
}

bool is_white_space(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

Token* Scanner::nextToken() {
    Token* token;
    
    // Saltar espacios en blanco
    while (current < input.length() && is_white_space(input[current])) 
        current++;
    
    if (current >= input.length()) 
        return new Token(Token::END);
    
    char c = input[current];
    first = current;
    
    // Números
    if (isdigit(c)) {
        current++;
        while (current < input.length() && isdigit(input[current]))
            current++;
        token = new Token(Token::NUM, input, first, current - first);
    }
    
    // Literales de string
    else if (c == '"') {
        current++; // saltar comilla inicial
        while (current < input.length() && input[current] != '"') {
            if (input[current] == '\\' && current + 1 < input.length()) {
                current += 2; // saltar carácter escapado
            } else {
                current++;
            }
        }
        if (current < input.length()) current++; // saltar comilla final
        token = new Token(Token::STRING_LIT, input, first, current - first);
    }
    
    // Identificadores y palabras reservadas
    else if (isalpha(c) || c == '_') {
        current++;
        while (current < input.length() && (isalnum(input[current]) || input[current] == '_'))
            current++;
        string word = input.substr(first, current - first);
        
        // Verificar si es palabra reservada
        auto it = keywords.find(word);
        if (it != keywords.end()) {
            token = new Token(it->second, word, 0, word.length());
        } else {
            token = new Token(Token::ID, word, 0, word.length());
        }
    }
    
    // Operadores y símbolos de dos caracteres
    else if (current + 1 < input.length()) {
        string two_char = input.substr(current, 2);
        
        if (two_char == "++") {
            token = new Token(Token::INC, input, first, 2);
            current += 2;
        } else if (two_char == "--") {
            token = new Token(Token::DEC, input, first, 2);
            current += 2;
        } else if (two_char == "&&") {
            token = new Token(Token::AND, input, first, 2);
            current += 2;
        } else if (two_char == "||") {
            token = new Token(Token::OR, input, first, 2);
            current += 2;
        } else if (two_char == "==") {
            token = new Token(Token::EQ, input, first, 2);
            current += 2;
        } else if (two_char == "!=") {
            token = new Token(Token::NE, input, first, 2);
            current += 2;
        } else if (two_char == "<=") {
            token = new Token(Token::LE, input, first, 2);
            current += 2;
        } else if (two_char == ">=") {
            token = new Token(Token::GE, input, first, 2);
            current += 2;
        } else if (two_char == ":=") {
            token = new Token(Token::SHORT_ASSIGN, input, first, 2);
            current += 2;
        } else if (two_char == "+=") {
            token = new Token(Token::PLUS_ASSIGN, input, first, 2);
            current += 2;
        } else if (two_char == "-=") {
            token = new Token(Token::MINUS_ASSIGN, input, first, 2);
            current += 2;
        } else if (two_char == "*=") {
            token = new Token(Token::MUL_ASSIGN, input, first, 2);
            current += 2;
        } else if (two_char == "/=") {
            token = new Token(Token::DIV_ASSIGN, input, first, 2);
            current += 2;
        } else if (two_char == "%=") {
            token = new Token(Token::MOD_ASSIGN, input, first, 2);
            current += 2;
        } else {
            // Operadores de un carácter
            switch(c) {
                case '+': token = new Token(Token::PLUS, c); break;
                case '-': token = new Token(Token::MINUS, c); break;
                case '*': token = new Token(Token::MUL, c); break;
                case '/': token = new Token(Token::DIV, c); break;
                case '%': token = new Token(Token::MOD, c); break;
                case '<': token = new Token(Token::LT, c); break;
                case '>': token = new Token(Token::GT, c); break;
                case '=': token = new Token(Token::ASSIGN, c); break;
                case '!': token = new Token(Token::NOT, c); break;
                case '(': token = new Token(Token::LPAREN, c); break;
                case ')': token = new Token(Token::RPAREN, c); break;
                case '{': token = new Token(Token::LBRACE, c); break;
                case '}': token = new Token(Token::RBRACE, c); break;
                case '[': token = new Token(Token::LBRACKET, c); break;
                case ']': token = new Token(Token::RBRACKET, c); break;
                case ';': token = new Token(Token::SEMICOLON, c); break;
                case ',': token = new Token(Token::COMMA, c); break;
                case '.': token = new Token(Token::DOT, c); break;
                case ':': token = new Token(Token::COLON, c); break;
                default:
                    token = new Token(Token::ERR, c);
            }
            current++;
        }
    }
    
    // Operadores de un carácter (cuando no hay segundo carácter)
    else {
        switch(c) {
            case '+': token = new Token(Token::PLUS, c); break;
            case '-': token = new Token(Token::MINUS, c); break;
            case '*': token = new Token(Token::MUL, c); break;
            case '/': token = new Token(Token::DIV, c); break;
            case '%': token = new Token(Token::MOD, c); break;
            case '<': token = new Token(Token::LT, c); break;
            case '>': token = new Token(Token::GT, c); break;
            case '=': token = new Token(Token::ASSIGN, c); break;
            case '!': token = new Token(Token::NOT, c); break;
            case '(': token = new Token(Token::LPAREN, c); break;
            case ')': token = new Token(Token::RPAREN, c); break;
            case '{': token = new Token(Token::LBRACE, c); break;
            case '}': token = new Token(Token::RBRACE, c); break;
            case '[': token = new Token(Token::LBRACKET, c); break;
            case ']': token = new Token(Token::RBRACKET, c); break;
            case ';': token = new Token(Token::SEMICOLON, c); break;
            case ',': token = new Token(Token::COMMA, c); break;
            case '.': token = new Token(Token::DOT, c); break;
            case ':': token = new Token(Token::COLON, c); break;
            default:
                token = new Token(Token::ERR, c);
        }
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
            cout << *current << " [" << current->text << "]" << endl;
        }
        delete current;
    }
    cout << "TOKEN(END)" << endl;
    delete current;
}
