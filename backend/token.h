#ifndef TOKEN_H
#define TOKEN_H

#include <string>

class Token {
public:
    enum Type {
        // Existing tokens
        PLUS, MINUS, MUL, DIV, NUM, ERR, PD, PI, END, ID, PRINT, ASSIGN, PC, LT, LE, EQ, IF, THEN, ELSE, ENDIF, WHILE, DO, ENDWHILE, COMA, IFEXP, VAR, FOR, ENDFOR, TRUE, FALSE, RETURN, FUN, ENDFUN,
        
        // Go-specific tokens
        PACKAGE, IMPORT, STRUCT, FUNC, STRING_LIT, COLON, DOT, LBRACE, RBRACE, LBRACKET, RBRACKET, 
        INC, DEC, GT, GE, NE, AND, OR, NOT, MOD, PLUSEQ, MINUSEQ, MULEQ, DIVEQ, MODEQ, SHORTASSIGN,
        TYPE, ELSE_IF
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