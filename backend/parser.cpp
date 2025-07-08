#include "parser.h"
#include <iostream>
#include <stdexcept>
#include <cctype>

GoParser::GoParser(Scanner* sc) : scanner(sc), current(nullptr), previous(nullptr) {
    advance(); // Inicializa el primer token
}

GoParser::~GoParser() {
    // Los tokens son manejados por el Scanner
}

Program* GoParser::parse() {
    try {
        return parseProgram();
    } catch (const exception& e) {
        cerr << "Parse error: " << e.what() << endl;
        return nullptr;
    }
}

bool GoParser::match(Token::Type type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool GoParser::check(Token::Type type) {
    if (isAtEnd()) return false;
    return current->type == type;
}

bool GoParser::advance() {
    if (current == nullptr || current->type != Token::END) {
        previous = current;
        current = scanner->nextToken();
        return true;
    }
    return false;
}

bool GoParser::isAtEnd() {
    return current == nullptr || current->type == Token::END;
}

void GoParser::error(const string& message) {
    string errorMsg = "Parse error";
    if (current) {
        errorMsg += " at token '" + current->text + "'";
    }
    errorMsg += ": " + message;
    throw runtime_error(errorMsg);
}

// Program ::= "package" ID ImportDeclList TopLevelDeclList
Program* GoParser::parseProgram() {
    // Parseamos el package pero por las puras es porque solo acepta main xdd
    if (!match(Token::PACKAGE)) {
        error("Expected 'package' at start of program");
    }
    
    // Aceptamos un ID o MAIN como nombre del paquete
    // En Go, el nombre del paquete es obligatorio y debe ser un identificador válido.
    string packageName;
    if (check(Token::ID)) {
        packageName = current->text;
        advance();
    } else if (check(Token::MAIN)) {
        packageName = current->text;
        advance();
    } else {
        error("Expected package name after 'package'");
    }
    
    // Lista de declaraciones de importación
    list<ImportDecl*> imports = parseImportDeclList();
    
    // Parseamos las declaraciones de nivel superior
    list<VarDecl*> globalVars = parseGlobalVarDecls();
    list<TypeDecl*> types = parseTypeDecls();
    list<FuncDecl*> functions = parseFuncDecls();
    
    return new Program(packageName, imports, globalVars, types, functions);
}

// ImportDeclList ::= { ImportDecl }
list<ImportDecl*> GoParser::parseImportDeclList() {
    list<ImportDecl*> imports;
    
    while (check(Token::IMPORT)) {
        imports.push_back(parseImportDecl());
    }
    
    return imports;
}

// ImportDecl ::= "import" STRING_LIT
ImportDecl* GoParser::parseImportDecl() {
    if (!match(Token::IMPORT)) {
        error("Expected 'import'");
    }
    
    if (!check(Token::STRING_LIT)) {
        error("Expected string literal after 'import'");
    }
    
    string path = current->text;
    advance();
    
    return new ImportDecl(path);
}

// Parseamos las declaraciones de variables globales
list<VarDecl*> GoParser::parseGlobalVarDecls() {
    list<VarDecl*> vars;
    
    while (check(Token::VAR)) {
        vars.push_back(parseVarDecl());
    }
    
    return vars;
}

// Parseamos las declaraciones de tipo
list<TypeDecl*> GoParser::parseTypeDecls() {
    list<TypeDecl*> types;
    
    while (check(Token::TYPE)) {
        types.push_back(parseTypeDecl());
    }
    
    return types;
}

// Parseamos las declaraciones de función
list<FuncDecl*> GoParser::parseFuncDecls() {
    list<FuncDecl*> functions;
    
    while (check(Token::FUNC)) {
        functions.push_back(parseFuncDecl());
    }
    
    return functions;
}

// VarDecl ::= "var" IdentifierList Type [ "=" ExpressionList ]
VarDecl* GoParser::parseVarDecl() {
    if (!match(Token::VAR)) {
        error("Expected 'var'");
    }
    
    list<string> names = parseIdentifierList();
    Type* type = parseType();
    
    list<Exp*> values;
    if (match(Token::ASSIGN)) {
        values = parseExpressionList();
    }
    
    return new VarDecl(names, type, values);
}

// TypeDecl ::= "type" ID StructType
TypeDecl* GoParser::parseTypeDecl() {
    if (!match(Token::TYPE)) {
        error("Expected 'type'");
    }
    
    if (!check(Token::ID)) {
        error("Expected type name");
    }
    string typeName = current->text;
    advance();
    
    StructType* structType = parseStructType();
    
    return new TypeDecl(typeName, structType);
}

// FuncDecl ::= "func" ID "(" ParamList ")" [ Type ] Block
FuncDecl* GoParser::parseFuncDecl() {
    if (!match(Token::FUNC)) {
        error("Expected 'func'");
    }
    
    // Acceptamos un ID o MAIN como nombre de la función
    string funcName;
    if (check(Token::ID)) {
        funcName = current->text;
        advance();
    } else if (check(Token::MAIN)) {
        funcName = current->text;
        advance();
    } else {
        error("Expected function name");
    }
    
    if (!match(Token::LPAREN)) {
        error("Expected '(' after function name");
    }
    
    list<pair<string, Type*>> params = parseParamList();
    
    if (!match(Token::RPAREN)) {
        error("Expected ')' after parameter list");
    }
    
    Type* returnType = nullptr;
    if (!check(Token::LBRACE)) {
        returnType = parseType();
    }
    
    Block* body = parseBlock();
    
    return new FuncDecl(funcName, params, returnType, body);
}

// Type ::= ID | StructType
Type* GoParser::parseType() {
    if (check(Token::STRUCT)) {
        return parseStructType();
    } else if (check(Token::ID)) {
        string typeName = current->text;
        advance();
        
        // Chequeamos si es un tipo básico o un identificador
        if (typeName == "int" || typeName == "string" || typeName == "bool") {
            return new BasicType(typeName);
        } else {
            return new IdentifierType(typeName);
        }
    } else {
        error("Expected type");
        return nullptr;
    }
}

// StructType ::= "struct" "{" { FieldDecl } "}"
StructType* GoParser::parseStructType() {
    if (!match(Token::STRUCT)) {
        error("Expected 'struct'");
    }
    
    if (!match(Token::LBRACE)) {
        error("Expected '{' after 'struct'");
    }
    
    list<pair<string, Type*>> fields;
    
    while (!check(Token::RBRACE) && !isAtEnd()) {
        // Parseamos los nombres de los campos (pueden ser múltiples)
        list<string> fieldNames;
        
        if (!check(Token::ID)) {
            error("Expected field name");
        }
        fieldNames.push_back(current->text);
        advance();
        
        // Chequeamos si hay más nombres de campos separados por comas
        while (match(Token::COMMA) && check(Token::ID)) {
            fieldNames.push_back(current->text);
            advance();
        }
        
        // Ahora debemos tener el tipo del campo
        Type* fieldType = parseType();
        
        // Agregamos todos los campos con este tipo
        for (const string& name : fieldNames) {
            fields.push_back(make_pair(name, fieldType));
        }
    }
    
    if (!match(Token::RBRACE)) {
        error("Expected '}' after struct fields");
    }
    
    return new StructType(fields);
}

// ParamList ::= [ Param { "," Param } ]
list<pair<string, Type*>> GoParser::parseParamList() {
    list<pair<string, Type*>> params;
    
    if (!check(Token::RPAREN)) {
        while (true) {
            // Parseamos los nombres de los parámetros (pueden ser múltiples)
            list<string> paramNames;
            
            if (!check(Token::ID)) {
                error("Expected parameter name");
            }
            paramNames.push_back(current->text);
            advance();
            
            // Chequeamos si hay más nombres de parámetros separados por comas
            while (match(Token::COMMA) && check(Token::ID)) {
                paramNames.push_back(current->text);
                advance();
            }
            
            // Ahora debemos tener el tipo del parámetro
            Type* paramType = parseType();
            
            // Agregamos todos los parámetros con este tipo
            for (const string& name : paramNames) {
                params.push_back(make_pair(name, paramType));
            }
            
            // Chequeamos si hay más parámetros
            if (!match(Token::COMMA)) {
                break;
            }
        }
    }
    
    return params;
}

// Block ::= "{" StmtList "}"
Block* GoParser::parseBlock() {
    if (!match(Token::LBRACE)) {
        error("Expected '{'");
    }
    
    list<Stmt*> statements = parseStmtList();
    
    if (!match(Token::RBRACE)) {
        error("Expected '}'");
    }
    
    return new Block(statements);
}

// StmtList ::= { Stmt }
list<Stmt*> GoParser::parseStmtList() {
    list<Stmt*> statements;
    
    while (!check(Token::RBRACE) && !isAtEnd()) {
        Stmt* stmt = parseStmt();
        if (stmt) {
            statements.push_back(stmt);
        }
    }
    
    return statements;
}

// Stmt ::= VarDecl | SimpleStmt | IfStmt | ForStmt | ReturnStmt | Block
Stmt* GoParser::parseStmt() {
    if (check(Token::VAR)) {
        return parseVarDecl();
    } else if (check(Token::IF)) {
        return parseIfStmt();
    } else if (check(Token::FOR)) {
        return parseForStmt();
    } else if (check(Token::RETURN)) {
        return parseReturnStmt();
    } else if (check(Token::LBRACE)) {
        Block* block = parseBlock();
        return new ExprStmt(new NumberExp(0)); // Placeholder
    } else {
        return parseSimpleStmt();
    }
}

// SimpleStmt ::= Assignment | ShortVarDecl | ExprStmt | IncDecStmt
Stmt* GoParser::parseSimpleStmt() {
    if (check(Token::ID)) {
        // Parseamos una declaración simple que puede ser una asignación, declaración corta o expresión
        list<string> identifiers;
        identifiers.push_back(current->text);
        advance();
        
        // Chequeamos si hay más identificadores separados por comas
        while (match(Token::COMMA)) {
            if (!check(Token::ID)) {
                error("Expected identifier after ','");
            }
            identifiers.push_back(current->text);
            advance();
        }
        
        if (match(Token::SHORT_ASSIGN)) {
            // Declaración corta de variable
            list<Exp*> values = parseExpressionList();
            return new ShortVarDecl(identifiers, values);
        } else if (check(Token::ASSIGN) || check(Token::PLUS_ASSIGN) || 
                   check(Token::MINUS_ASSIGN) || check(Token::MUL_ASSIGN) ||
                   check(Token::DIV_ASSIGN) || check(Token::MOD_ASSIGN)) {
            // Asignación
            if (identifiers.size() != 1) {
                error("Multiple assignment targets not supported yet");
            }
            
            AssignOp op = ASSIGN_OP;
            if (match(Token::ASSIGN)) op = ASSIGN_OP;
            else if (match(Token::PLUS_ASSIGN)) op = PLUS_ASSIGN_OP;
            else if (match(Token::MINUS_ASSIGN)) op = MINUS_ASSIGN_OP;
            else if (match(Token::MUL_ASSIGN)) op = MUL_ASSIGN_OP;
            else if (match(Token::DIV_ASSIGN)) op = DIV_ASSIGN_OP;
            else if (match(Token::MOD_ASSIGN)) op = MOD_ASSIGN_OP;
            else error("Expected assignment operator");
            
            Exp* rhs = parseExpression();
            Exp* lhs = new IdentifierExp(identifiers.front());
            
            return new AssignStmt(lhs, rhs, op);
        } else if (check(Token::INC) || check(Token::DEC)) {
            // Incremento o decremento
            if (identifiers.size() != 1) {
                error("Increment/decrement only works with single variable");
            }
            
            bool isIncrement = true;
            if (match(Token::INC)) {
                isIncrement = true;
            } else if (match(Token::DEC)) {
                isIncrement = false;
            } else {
                error("Expected '++' or '--'");
            }
            
            return new IncDecStmt(identifiers.front(), isIncrement);
        } else {
            // Expresión simple
            if (identifiers.size() != 1) {
                error("Complex expressions not supported in this context");
            }
            
            // Esto es un hack la verdad necesita hacerse refactorización
            // para que las expresiones sean más robustas y no dependan de un solo identificador
            Exp* expr = parseExpressionFromIdentifier(identifiers.front());
            return new ExprStmt(expr);
        }
    } else {
        // Parseamos una expresión simple
        Exp* expr = parseExpression();
        return new ExprStmt(expr);
    }
}

// IfStmt ::= "if" Expression Block [ "else" Block ]
IfStmt* GoParser::parseIfStmt() {
    if (!match(Token::IF)) {
        error("Expected 'if'");
    }
    
    Exp* condition = parseExpression();
    Block* thenBlock = parseBlock();
    
    Block* elseBlock = nullptr;
    if (match(Token::ELSE)) {
        if (check(Token::IF)) {
            // else-if statement
            IfStmt* elseIfStmt = parseIfStmt();
            list<Stmt*> stmts;
            stmts.push_back(elseIfStmt);
            elseBlock = new Block(stmts);
        } else {
            // else block
            elseBlock = parseBlock();
        }
    }
    
    return new IfStmt(condition, thenBlock, elseBlock);
}

// ForStmt ::= "for" [ SimpleStmt ] ";" [ Expression ] ";" [ SimpleStmt ] Block
ForStmt* GoParser::parseForStmt() {
    if (!match(Token::FOR)) {
        error("Expected 'for'");
    }
    
    Stmt* init = nullptr;
    Exp* condition = nullptr;
    Stmt* post = nullptr;
    
    // Chequeamos si hay un bloque de llaves inmediatamente
    if (!check(Token::LBRACE)) {
        // Puede ser un bucle for simple o un bucle for con inicialización, condición y post
        Stmt* firstStmt = parseSimpleStmt();
        
        if (match(Token::SEMICOLON)) {
            // Esto es un bucle for con inicialización, condición y post
            init = firstStmt;
            
            if (!check(Token::SEMICOLON)) {
                condition = parseExpression();
            }
            
            if (!match(Token::SEMICOLON)) {
                error("Expected ';'");
            }
            
            if (!check(Token::LBRACE)) {
                post = parseSimpleStmt();
            }
        } else {
            // Esto es un bucle for simple sin inicialización, condición y post
            ExprStmt* exprStmt = dynamic_cast<ExprStmt*>(firstStmt);
            if (exprStmt) {
                condition = exprStmt->expression;
            } else {
                error("Invalid for loop condition");
            }
        }
    }
    
    Block* body = parseBlock();
    
    return new ForStmt(init, condition, post, body);
}

// ReturnStmt ::= "return" [ Expression ]
ReturnStmt* GoParser::parseReturnStmt() {
    if (!match(Token::RETURN)) {
        error("Expected 'return'");
    }
    
    Exp* expression = nullptr;
    if (!check(Token::SEMICOLON) && !check(Token::RBRACE) && !isAtEnd()) {
        expression = parseExpression();
    }
    
    return new ReturnStmt(expression);
}

// Expression ::= LogicalOrExpr
Exp* GoParser::parseExpression() {
    return parseLogicalOrExpr();
}

// LogicalOrExpr ::= LogicalAndExpr { "||" LogicalAndExpr }
Exp* GoParser::parseLogicalOrExpr() {
    Exp* expr = parseLogicalAndExpr();
    
    while (match(Token::OR)) {
        Exp* right = parseLogicalAndExpr();
        expr = new BinaryExp(expr, right, OR_OP);
    }
    
    return expr;
}

// LogicalAndExpr ::= EqualityExpr { "&&" EqualityExpr }
Exp* GoParser::parseLogicalAndExpr() {
    Exp* expr = parseEqualityExpr();
    
    while (match(Token::AND)) {
        Exp* right = parseEqualityExpr();
        expr = new BinaryExp(expr, right, AND_OP);
    }
    
    return expr;
}

// EqualityExpr ::= RelationalExpr { ("==" | "!=") RelationalExpr }
Exp* GoParser::parseEqualityExpr() {
    Exp* expr = parseRelationalExpr();
    
    while (true) {
        if (match(Token::EQ)) {
            Exp* right = parseRelationalExpr();
            expr = new BinaryExp(expr, right, EQ_OP);
        } else if (match(Token::NE)) {
            Exp* right = parseRelationalExpr();
            expr = new BinaryExp(expr, right, NE_OP);
        } else {
            break;
        }
    }
    
    return expr;
}

// RelationalExpr ::= AdditiveExpr { ("<" | "<=" | ">" | ">=") AdditiveExpr }
Exp* GoParser::parseRelationalExpr() {
    Exp* expr = parseAdditiveExpr();
    
    while (true) {
        if (match(Token::LT)) {
            Exp* right = parseAdditiveExpr();
            expr = new BinaryExp(expr, right, LT_OP);
        } else if (match(Token::LE)) {
            Exp* right = parseAdditiveExpr();
            expr = new BinaryExp(expr, right, LE_OP);
        } else if (match(Token::GT)) {
            Exp* right = parseAdditiveExpr();
            expr = new BinaryExp(expr, right, GT_OP);
        } else if (match(Token::GE)) {
            Exp* right = parseAdditiveExpr();
            expr = new BinaryExp(expr, right, GE_OP);
        } else {
            break;
        }
    }
    
    return expr;
}

// AdditiveExpr ::= MultiplicativeExpr { ("+" | "-") MultiplicativeExpr }
Exp* GoParser::parseAdditiveExpr() {
    Exp* expr = parseMultiplicativeExpr();
    
    while (true) {
        if (match(Token::PLUS)) {
            Exp* right = parseMultiplicativeExpr();
            expr = new BinaryExp(expr, right, PLUS_OP);
        } else if (match(Token::MINUS)) {
            Exp* right = parseMultiplicativeExpr();
            expr = new BinaryExp(expr, right, MINUS_OP);
        } else {
            break;
        }
    }
    
    return expr;
}

// MultiplicativeExpr ::= UnaryExpr { ("*" | "/" | "%") UnaryExpr }
Exp* GoParser::parseMultiplicativeExpr() {
    Exp* expr = parseUnaryExpr();
    
    while (true) {
        if (match(Token::MUL)) {
            Exp* right = parseUnaryExpr();
            expr = new BinaryExp(expr, right, MUL_OP);
        } else if (match(Token::DIV)) {
            Exp* right = parseUnaryExpr();
            expr = new BinaryExp(expr, right, DIV_OP);
        } else if (match(Token::MOD)) {
            Exp* right = parseUnaryExpr();
            expr = new BinaryExp(expr, right, MOD_OP);
        } else {
            break;
        }
    }
    
    return expr;
}

// UnaryExpr ::= ("+" | "-" | "!") UnaryExpr | PrimaryExpr
Exp* GoParser::parseUnaryExpr() {
    if (match(Token::PLUS)) {
        Exp* expr = parseUnaryExpr();
        return new UnaryExp(expr, UPLUS_OP);
    } else if (match(Token::MINUS)) {
        Exp* expr = parseUnaryExpr();
        return new UnaryExp(expr, UMINUS_OP);
    } else if (match(Token::NOT)) {
        Exp* expr = parseUnaryExpr();
        return new UnaryExp(expr, NOT_OP);
    } else {
        return parsePrimaryExpr();
    }
}

// PrimaryExpr ::= NUM | STRING_LIT | TRUE | FALSE | ID | "(" Expression ")" 
//               | ID "(" ExpressionList ")" | ID "{" ExpressionList "}"
//               | PrimaryExpr "." ID | PrimaryExpr "[" Expression "]"
Exp* GoParser::parsePrimaryExpr() {
    Exp* expr = nullptr;
    
    if (match(Token::NUM)) {
        int value = stoi(previous->text);
        expr = new NumberExp(value);
    } else if (match(Token::STRING_LIT)) {
        string value = previous->text;
        expr = new StringExp(value);
    } else if (match(Token::TRUE)) {
        expr = new BoolExp(true);
    } else if (match(Token::FALSE)) {
        expr = new BoolExp(false);
    } else if (check(Token::ID)) {
        string name = current->text;
        advance();
        
        if (match(Token::LPAREN)) {
            // Llamada a función
            list<Exp*> args = parseExpressionList();
            if (!match(Token::RPAREN)) {
                error("Expected ')' after function arguments");
            }
            expr = new FunctionCallExp(name, args);
        } else if (!name.empty() && isupper(name[0]) && match(Token::LBRACE)) {
            // Literal de estructura
            // Asumimos que es un literal de estructura si el nombre empieza con mayúscula
            list<Exp*> values = parseStructLiteralValues();
            if (!match(Token::RBRACE)) {
                error("Expected '}' after struct literal");
            }
            expr = new StructLiteralExp(name, values);
        } else {
            // Identificador simple
            expr = new IdentifierExp(name);
        }
    } else if (match(Token::LPAREN)) {
        expr = parseExpression();
        if (!match(Token::RPAREN)) {
            error("Expected ')' after expression");
        }
    } else {
        error("Expected expression");
        return nullptr;
    }
    
    // Manejamos el acceso a campos y la indexación
    // Esto permite acceder a campos de estructuras o indexar arrays/slices (No hecho)
    while (expr != nullptr) {
        if (match(Token::DOT)) {
            if (!check(Token::ID)) {
                error("Expected field name after '.'");
            }
            string fieldName = current->text;
            advance();
            expr = new FieldAccessExp(expr, fieldName);
        } else if (match(Token::LBRACKET)) {
            Exp* index = parseExpression();
            
            // Chequeamos si es una expresión de slice [start:end]
            if (match(Token::COLON)) {
                //  Esto es una expresión de slice
                Exp* end = nullptr;
                if (!check(Token::RBRACKET)) {
                    end = parseExpression();
                }
                if (!match(Token::RBRACKET)) {
                    error("Expected ']' after slice");
                }
                expr = new SliceExp(expr, index, end);
            } else {
                // Indexación regular
                if (!match(Token::RBRACKET)) {
                    error("Expected ']' after array index");
                }
                expr = new IndexExp(expr, index);
            }
        } else {
            break;
        }
    }
    
    return expr;
}

// ExpressionList ::= [ Expression { "," Expression } ]
list<Exp*> GoParser::parseExpressionList() {
    list<Exp*> expressions;
    
    if (!check(Token::RPAREN) && !check(Token::RBRACE) && !isAtEnd()) {
        expressions.push_back(parseExpression());
        
        while (match(Token::COMMA)) {
            expressions.push_back(parseExpression());
        }
    }
    
    return expressions;
}

// IdentifierList ::= ID { "," ID }
list<string> GoParser::parseIdentifierList() {
    list<string> identifiers;
    
    if (!check(Token::ID)) {
        error("Expected identifier");
    }
    identifiers.push_back(current->text);
    advance();
    
    while (match(Token::COMMA)) {
        if (!check(Token::ID)) {
            error("Expected identifier after ','");
        }
        identifiers.push_back(current->text);
        advance();
    }
    
    return identifiers;
}

// Helper for struct literals
StructLiteralExp* GoParser::parseStructLiteral(const string& typeName) {
    if (!match(Token::LBRACE)) {
        error("Expected '{' for struct literal");
    }
    
    list<Exp*> values = parseExpressionList();
    
    if (!match(Token::RBRACE)) {
        error("Expected '}' after struct literal");
    }
    
    return new StructLiteralExp(typeName, values);
}

// Helper for parsing expressions that start with an identifier
Exp* GoParser::parseExpressionFromIdentifier(const string& identifierName) {
    // Empieza con un identificador simple
    Exp* expr = new IdentifierExp(identifierName);
    
    // Manejamos el acceso a campos y la indexación
    while (true) {
        if (match(Token::DOT)) {
            if (!check(Token::ID)) {
                error("Expected field name after '.'");
            }
            string fieldName = current->text;
            advance();
            expr = new FieldAccessExp(expr, fieldName);
        } else if (match(Token::LBRACKET)) {
            Exp* index = parseExpression();
            
            // Chequeamos si es una expresión de slice [start:end]
            if (match(Token::COLON)) {
                // Esto es una expresión de slice
                Exp* end = nullptr;
                if (!check(Token::RBRACKET)) {
                    end = parseExpression();
                }
                if (!match(Token::RBRACKET)) {
                    error("Expected ']' after slice");
                }
                expr = new SliceExp(expr, index, end);
            } else {
                // Indexación regular
                if (!match(Token::RBRACKET)) {
                    error("Expected ']' after array index");
                }
                expr = new IndexExp(expr, index);
            }
        } else if (match(Token::LPAREN)) {
            // Function call - for simplicity, we'll create the function name from the expression
            string funcName = identifierName;
            
            // si el identificador es un acceso a campo, lo manejamos
            FieldAccessExp* fieldAccess = dynamic_cast<FieldAccessExp*>(expr);
            if (fieldAccess) {
                IdentifierExp* base = dynamic_cast<IdentifierExp*>(fieldAccess->object);
                if (base) {
                    funcName = base->name + "." + fieldAccess->field;
                }
            }
            
            list<Exp*> args = parseExpressionList();
            if (!match(Token::RPAREN)) {
                error("Expected ')' after function arguments");
            }
            expr = new FunctionCallExp(funcName, args);
            break;
        } else if (check(Token::LT) || check(Token::LE) || check(Token::GT) || 
                  check(Token::GE) || check(Token::EQ) || check(Token::NE) ||
                  check(Token::PLUS) || check(Token::MINUS) || check(Token::MUL) || 
                  check(Token::DIV) || check(Token::MOD) || check(Token::AND) || 
                  check(Token::OR)) {
            // Manejamos operaciones binarias para comporaciones y aritméticas
            BinaryOp op;
            
            if (match(Token::LT)) op = LT_OP;
            else if (match(Token::LE)) op = LE_OP;
            else if (match(Token::GT)) op = GT_OP;
            else if (match(Token::GE)) op = GE_OP;
            else if (match(Token::EQ)) op = EQ_OP;
            else if (match(Token::NE)) op = NE_OP;
            else if (match(Token::PLUS)) op = PLUS_OP;
            else if (match(Token::MINUS)) op = MINUS_OP;
            else if (match(Token::MUL)) op = MUL_OP;
            else if (match(Token::DIV)) op = DIV_OP;
            else if (match(Token::MOD)) op = MOD_OP;
            else if (match(Token::AND)) op = AND_OP;
            else if (match(Token::OR)) op = OR_OP;
            else break;
            
            // Parseamos la expresión a la derecha de la operación
            Exp* right = parseAdditiveExpr(); // Usamos precedencia
            expr = new BinaryExp(expr, right, op);
            
            // Continuamos parseando más operaciones
            continue;
        } else {
            break;
        }
    }
    
    return expr;
}

// Parsea los valores de un literal de estructura
list<Exp*> GoParser::parseStructLiteralValues() {
    list<Exp*> values;
    
    if (!check(Token::RBRACE) && !isAtEnd()) {
        // Si el primer token es un ID, asumimos que es un campo nombrado
        if (check(Token::ID)) {
            string firstId = current->text;
            advance();
            
            if (match(Token::COLON)) {
                // Esto es un campo nombrado
                Exp* firstValue = parseExpression();
                values.push_back(firstValue);
                
                // Continuamos con más campos nombrados
                while (match(Token::COMMA)) {
                    if (!check(Token::ID)) {
                        error("Expected field name");
                    }
                    advance(); // Avanzamos al siguiente ID
                    
                    if (!match(Token::COLON)) {
                        error("Expected ':' after field name");
                    }
                    
                    values.push_back(parseExpression());
                }
            } else {
                // Esto es un valor posicional
                IdentifierExp* firstExpr = new IdentifierExp(firstId);
                
                // Manejamos el acceso a campos en caso de que sea un campo nombrado
                while (true) {
                    if (match(Token::DOT)) {
                        if (!check(Token::ID)) {
                            error("Expected field name after '.'");
                        }
                        string fieldName = current->text;
                        advance();
                        firstExpr = (IdentifierExp*)new FieldAccessExp(firstExpr, fieldName);
                    } else {
                        break;
                    }
                }
                
                values.push_back(firstExpr);
                
                // Continuamos con más valores posicionales
                while (match(Token::COMMA)) {
                    values.push_back(parseExpression());
                }
            }
        } else {
            // Si no es un ID, asumimos que son valores posicionales
            values.push_back(parseExpression());
            while (match(Token::COMMA)) {
                values.push_back(parseExpression());
            }
        }
    }
    
    return values;
}