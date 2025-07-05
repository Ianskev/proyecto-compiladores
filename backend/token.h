#ifndef TOKEN_H
#define TOKEN_H

#include <string>

class Token {
public:
    enum Type {
        // Operadores aritméticos
        PLUS, MINUS, MUL, DIV, MOD,
        
        // Operadores de comparación
        LT, LE, GT, GE, EQ, NE,
        
        // Operadores lógicos
        AND, OR, NOT,
        
        // Operadores de asignación
        ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, MUL_ASSIGN, DIV_ASSIGN, MOD_ASSIGN,
        SHORT_ASSIGN, // :=
        
        // Operadores de incremento/decremento
        INC, DEC, // ++ --
        
        // Delimitadores
        LPAREN, RPAREN, // ( )
        LBRACE, RBRACE, // { }
        LBRACKET, RBRACKET, // [ ]
        SEMICOLON, COMMA, DOT, COLON, // ; , . :
        
        // Palabras reservadas - estructura del programa
        PACKAGE, MAIN, IMPORT,
        
        // Palabras reservadas - declaraciones
        VAR, TYPE, FUNC, STRUCT,
        
        // Palabras reservadas - control de flujo
        IF, ELSE, FOR, RETURN,
        
        // Literales
        NUM, STRING_LIT, TRUE, FALSE,
        
        // Identificadores
        ID,
        
        // Especiales
        END, ERR
    };

    Type type;
    std::string text;

    Token(Type type);
    Token(Type type, char c);
    Token(Type type, const std::string& source, int first, int last);

    friend std::ostream& operator<<(std::ostream& outs, const Token& tok);
    friend std::ostream& operator<<(std::ostream& outs, const Token* tok);
};

#endif // TOKEN_H