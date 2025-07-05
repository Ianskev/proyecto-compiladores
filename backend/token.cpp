#include <iostream>
#include "token.h"

using namespace std;

Token::Token(Type type):type(type) { text = ""; }

Token::Token(Type type, char c):type(type) { text = string(1, c); }

Token::Token(Type type, const string& source, int first, int last):type(type) {
    text = source.substr(first, last);
}

std::ostream& operator << ( std::ostream& outs, const Token & tok )
{
    switch (tok.type) {
        // Operadores aritméticos
        case Token::PLUS: outs << "TOKEN(PLUS)"; break;
        case Token::MINUS: outs << "TOKEN(MINUS)"; break;
        case Token::MUL: outs << "TOKEN(MUL)"; break;
        case Token::DIV: outs << "TOKEN(DIV)"; break;
        case Token::MOD: outs << "TOKEN(MOD)"; break;
        
        // Operadores de comparación
        case Token::LT: outs << "TOKEN(LT)"; break;
        case Token::LE: outs << "TOKEN(LE)"; break;
        case Token::GT: outs << "TOKEN(GT)"; break;
        case Token::GE: outs << "TOKEN(GE)"; break;
        case Token::EQ: outs << "TOKEN(EQ)"; break;
        case Token::NE: outs << "TOKEN(NE)"; break;
        
        // Operadores lógicos
        case Token::AND: outs << "TOKEN(AND)"; break;
        case Token::OR: outs << "TOKEN(OR)"; break;
        case Token::NOT: outs << "TOKEN(NOT)"; break;
        
        // Operadores de asignación
        case Token::ASSIGN: outs << "TOKEN(ASSIGN)"; break;
        case Token::PLUS_ASSIGN: outs << "TOKEN(PLUS_ASSIGN)"; break;
        case Token::MINUS_ASSIGN: outs << "TOKEN(MINUS_ASSIGN)"; break;
        case Token::MUL_ASSIGN: outs << "TOKEN(MUL_ASSIGN)"; break;
        case Token::DIV_ASSIGN: outs << "TOKEN(DIV_ASSIGN)"; break;
        case Token::MOD_ASSIGN: outs << "TOKEN(MOD_ASSIGN)"; break;
        case Token::SHORT_ASSIGN: outs << "TOKEN(SHORT_ASSIGN)"; break;
        
        // Operadores de incremento/decremento
        case Token::INC: outs << "TOKEN(INC)"; break;
        case Token::DEC: outs << "TOKEN(DEC)"; break;
        
        // Delimitadores
        case Token::LPAREN: outs << "TOKEN(LPAREN)"; break;
        case Token::RPAREN: outs << "TOKEN(RPAREN)"; break;
        case Token::LBRACE: outs << "TOKEN(LBRACE)"; break;
        case Token::RBRACE: outs << "TOKEN(RBRACE)"; break;
        case Token::LBRACKET: outs << "TOKEN(LBRACKET)"; break;
        case Token::RBRACKET: outs << "TOKEN(RBRACKET)"; break;
        case Token::SEMICOLON: outs << "TOKEN(SEMICOLON)"; break;
        case Token::COMMA: outs << "TOKEN(COMMA)"; break;
        case Token::DOT: outs << "TOKEN(DOT)"; break;
        case Token::COLON: outs << "TOKEN(COLON)"; break;
        
        // Palabras reservadas - estructura del programa
        case Token::PACKAGE: outs << "TOKEN(PACKAGE)"; break;
        case Token::MAIN: outs << "TOKEN(MAIN)"; break;
        case Token::IMPORT: outs << "TOKEN(IMPORT)"; break;
        
        // Palabras reservadas - declaraciones
        case Token::VAR: outs << "TOKEN(VAR)"; break;
        case Token::TYPE: outs << "TOKEN(TYPE)"; break;
        case Token::FUNC: outs << "TOKEN(FUNC)"; break;
        case Token::STRUCT: outs << "TOKEN(STRUCT)"; break;
        
        // Palabras reservadas - control de flujo
        case Token::IF: outs << "TOKEN(IF)"; break;
        case Token::ELSE: outs << "TOKEN(ELSE)"; break;
        case Token::FOR: outs << "TOKEN(FOR)"; break;
        case Token::RETURN: outs << "TOKEN(RETURN)"; break;
        
        // Literales
        case Token::NUM: outs << "TOKEN(NUM)"; break;
        case Token::STRING_LIT: outs << "TOKEN(STRING_LIT)"; break;
        case Token::TRUE: outs << "TOKEN(TRUE)"; break;
        case Token::FALSE: outs << "TOKEN(FALSE)"; break;
        
        // Identificadores
        case Token::ID: outs << "TOKEN(ID)"; break;
        
        // Especiales
        case Token::END: outs << "TOKEN(END)"; break;
        case Token::ERR: outs << "TOKEN(ERR)"; break;
        
        default: outs << "TOKEN(UNKNOWN)"; break;
    }
    return outs;
}

std::ostream& operator << ( std::ostream& outs, const Token* tok ) {
    return outs << *tok;
}