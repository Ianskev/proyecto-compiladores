#ifndef PARSER_GO_H
#define PARSER_GO_H

#include "scanner.h"
#include "exp.h"

class GoParser {
private:
    Scanner* scanner;
    Token* current;
    Token* previous;
    
    bool match(Token::Type type);
    bool check(Token::Type type);
    bool advance();
    bool isAtEnd();
    void error(const string& message);
    
    // Métodos de parsing para cada regla de la gramática
    Program* parseProgram();
    
    // Import declarations
    list<ImportDecl*> parseImportDeclList();
    ImportDecl* parseImportDecl();
    
    // Top level declarations
    list<VarDecl*> parseGlobalVarDecls();
    list<TypeDecl*> parseTypeDecls();
    list<FuncDecl*> parseFuncDecls();
    
    VarDecl* parseVarDecl();
    TypeDecl* parseTypeDecl();
    FuncDecl* parseFuncDecl();
    
    // Types
    Type* parseType();
    StructType* parseStructType();
    
    // Function parameters
    list<pair<string, Type*>> parseParamList();
    
    // Blocks and statements
    Block* parseBlock();
    list<Stmt*> parseStmtList();
    Stmt* parseStmt();
    Stmt* parseSimpleStmt();
    
    // Specific statements
    IfStmt* parseIfStmt();
    ForStmt* parseForStmt();
    ReturnStmt* parseReturnStmt();
    
    // Expressions
    Exp* parseExpression();
    Exp* parseLogicalOrExpr();
    Exp* parseLogicalAndExpr();
    Exp* parseEqualityExpr();
    Exp* parseRelationalExpr();
    Exp* parseAdditiveExpr();
    Exp* parseMultiplicativeExpr();
    Exp* parseUnaryExpr();
    Exp* parsePrimaryExpr();
    
    // Expression lists
    list<Exp*> parseExpressionList();
    list<string> parseIdentifierList();
    
    // Struct literal parsing
    list<Exp*> parseStructLiteralValues();
    
    // Literals
    StructLiteralExp* parseStructLiteral(const string& typeName);
    
    // Helper for parsing expressions that start with an identifier
    Exp* parseExpressionFromIdentifier(const string& identifierName);
    
public:
    GoParser(Scanner* sc);
    Program* parse();
    ~GoParser();
};

#endif // PARSER_GO_H
