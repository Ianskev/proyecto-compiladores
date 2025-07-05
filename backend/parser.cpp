#include <iostream>
#include <stdexcept>
#include "token.h"
#include "scanner.h"
#include "exp.h"
#include "parser.h"

using namespace std;

bool Parser::match(Token::Type ttype) {
    if (check(ttype)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(Token::Type ttype) {
    if (isAtEnd()) return false;
    return current->type == ttype;
}

bool Parser::advance() {
    if (!isAtEnd()) {
        Token* temp = current;
        if (previous) delete previous;
        current = scanner->nextToken();
        previous = temp;
        if (check(Token::ERR)) {
            cout << "Error de análisis, carácter no reconocido: " << current->text << endl;
            exit(1);
        }
        return true;
    }
    return false;
}

bool Parser::isAtEnd() {
    return (current->type == Token::END);
}

Parser::Parser(Scanner* sc):scanner(sc) {
    previous = NULL;
    current = scanner->nextToken();
    if (current->type == Token::ERR) {
        cout << "Error en el primer token: " << current->text << endl;
        exit(1);
    }
}

VarDec* Parser::parseVarDec() {
    VarDec* vd = NULL;
    if (match(Token::VAR)) {
        if (!match(Token::ID)) {
            cout << "Error: se esperaba un identificador después de 'var'." << endl;
            exit(1);
        }
        string type = previous->text;
        list<string> ids;
        if (!match(Token::ID)) {
            cout << "Error: se esperaba un identificador después de 'var'." << endl;
            exit(1);
        }
        ids.push_back(previous->text);
        while (match(Token::COMA)) {
            if (!match(Token::ID)) {
                cout << "Error: se esperaba un identificador después de ','." << endl;
                exit(1);
            }
            ids.push_back(previous->text);
        }
        if (!match(Token::PC)) {
            cout << "Error: se esperaba un ';' al final de la declaración." << endl;
            exit(1);
        }
        vd = new VarDec(type, ids);
    }
    return vd;
}

VarDecList* Parser::parseVarDecList() {
    VarDecList* vdl = new VarDecList();
    VarDec* aux;
    aux = parseVarDec();
    while (aux != NULL) {
        vdl->add(aux);
        aux = parseVarDec();
    }
    return vdl;
}

StatementList* Parser::parseStatementList() {
    StatementList* sl = new StatementList();
    sl->add(parseStatement());
    while (match(Token::PC)) {
        sl->add(parseStatement());
    }
    return sl;
}


Body* Parser::parseBody() {
    VarDecList* vdl = parseVarDecList();
    StatementList* sl = parseStatementList();
    return new Body(vdl, sl);
}



Program* Parser::parseProgram() {
    Body* v = parseBody();
    return new Program(v);
}

list<Stm*> Parser::parseStmList() {
    list<Stm*> slist;
    slist.push_back(parseStatement());
    while(match(Token::PC)) {
        slist.push_back(parseStatement());
    }
    return slist;
}

Stm* Parser::parseStatement() {
    Stm* s = NULL;
    Exp* e = NULL;
    Body* tb = NULL; //true case
    Body* fb = NULL; //false case

    if (current == NULL) {
        cout << "Error: Token actual es NULL" << endl;
        exit(1);
    }

    if (match(Token::ID)) {
        string lex = previous->text;

        if (match(Token::ASSIGN)) {
            e = parseCExp();
            s = new AssignStatement(lex, e);
        }

    } else if (match(Token::PRINT)) {
        if (!match(Token::PI)) {
            cout << "Error: se esperaba un '(' después de 'print'." << endl;
            exit(1);
        }
        e = parseCExp();
        if (!match(Token::PD)) {
            cout << "Error: se esperaba un ')' después de la expresión." << endl;
            exit(1);
        }
        s = new PrintStatement(e);
    }
    else if (match(Token::IF)) {
        e = parseCExp();
        if (!match(Token::THEN)) {
            cout << "Error: se esperaba 'then' después de la expresión." << endl;
            exit(1);
        }
        
        tb = parseBody();

        if (match(Token::ELSE)) {
            fb = parseBody();
        }
        if (!match(Token::ENDIF)) {
            cout << "Error: se esperaba 'end' al final de la declaración." << endl;
            exit(1);
        }
        s = new IfStatement(e, tb, fb);

    }
    else if (match(Token::WHILE)) {
        e = parseCExp();
        if (!match(Token::DO)) {
            cout << "Error: se esperaba 'do' después de la expresión." << endl;
            exit(1);
        }
        tb = parseBody();
        if (!match(Token::ENDWHILE)) {
            cout << "Error: se esperaba 'endwhile' al final de la declaración." << endl;
            exit(1);
        }
        s = new WhileStatement(e, tb);

    }
    else {
        cout << "Error: Se esperaba un identificador o 'print', pero se encontró: " << *current << endl;
        exit(1);
    }
    return s;
}

GoProgram* Parser::parseGoProgram() {
    PackageDeclaration* package = parsePackage();
    list<ImportDeclaration*> imports = parseImports();
    Body* body = parseBody();
    return new GoProgram(package, imports, body);
}

PackageDeclaration* Parser::parsePackage() {
    if (!match(Token::PACKAGE)) {
        cout << "Error: Expected 'package' declaration at the beginning of the file." << endl;
        exit(1);
    }
    
    if (!match(Token::ID)) {
        cout << "Error: Expected package name after 'package'." << endl;
        exit(1);
    }
    
    string packageName = previous->text;
    return new PackageDeclaration(packageName);
}

list<ImportDeclaration*> Parser::parseImports() {
    list<ImportDeclaration*> imports;
    
    while (match(Token::IMPORT)) {
        if (match(Token::STRING_LIT)) {
            imports.push_back(new ImportDeclaration(previous->text));
        } else if (match(Token::PI)) {
            // Parse grouped imports
            while (!check(Token::PD) && !isAtEnd()) {
                if (match(Token::STRING_LIT)) {
                    imports.push_back(new ImportDeclaration(previous->text));
                } else {
                    cout << "Error: Expected string literal in import declaration." << endl;
                    exit(1);
                }
            }
            
            if (!match(Token::PD)) {
                cout << "Error: Expected ')' after grouped imports." << endl;
                exit(1);
            }
        } else {
            cout << "Error: Expected string literal or '(' after 'import'." << endl;
            exit(1);
        }
    }
    
    return imports;
}

StructDeclaration* Parser::parseStructType() {
    if (!match(Token::TYPE)) {
        cout << "Error: Expected 'type' keyword." << endl;
        exit(1);
    }
    
    if (!match(Token::ID)) {
        cout << "Error: Expected struct name after 'type'." << endl;
        exit(1);
    }
    
    string structName = previous->text;
    
    if (!match(Token::STRUCT)) {
        cout << "Error: Expected 'struct' keyword." << endl;
        exit(1);
    }
    
    if (!match(Token::LBRACE)) {
        cout << "Error: Expected '{' after 'struct'." << endl;
        exit(1);
    }
    
    list<pair<string, string>> fields;
    while (!check(Token::RBRACE) && !isAtEnd()) {
        if (!match(Token::ID)) {
            cout << "Error: Expected field name in struct declaration." << endl;
            exit(1);
        }
        
        string fieldName = previous->text;
        
        if (!match(Token::ID)) {
            cout << "Error: Expected field type in struct declaration." << endl;
            exit(1);
        }
        
        string fieldType = previous->text;
        fields.push_back(make_pair(fieldName, fieldType));
    }
    
    if (!match(Token::RBRACE)) {
        cout << "Error: Expected '}' at the end of struct declaration." << endl;
        exit(1);
    }
    
    return new StructDeclaration(structName, fields);
}

Exp* Parser::parseLogicalOrExpr() {
    Exp* left = parseLogicalAndExpr();
    
    while (match(Token::OR)) {
        Exp* right = parseLogicalAndExpr();
        left = new BinaryExp(left, right, OR_OP);
    }
    
    return left;
}

Exp* Parser::parseLogicalAndExpr() {
    Exp* left = parseEqualityExpr();
    
    while (match(Token::AND)) {
        Exp* right = parseEqualityExpr();
        left = new BinaryExp(left, right, AND_OP);
    }
    
    return left;
}

Exp* Parser::parseEqualityExpr() {
    Exp* left = parseRelExpr();
    
    while (match(Token::EQ) || match(Token::NE)) {
        BinaryOp op;
        if (previous->type == Token::EQ) {
            op = EQ_OP;
        } else {
            op = NE_OP;
        }
        
        Exp* right = parseRelExpr();
        left = new BinaryExp(left, right, op);
    }
    
    return left;
}

Exp* Parser::parseRelExpr() {
    Exp* left = parseExpression();
    
    while (match(Token::LT) || match(Token::LE) || match(Token::GT) || match(Token::GE)) {
        BinaryOp op;
        if (previous->type == Token::LT) {
            op = LT_OP;
        } else if (previous->type == Token::LE) {
            op = LE_OP;
        } else if (previous->type == Token::GT) {
            op = GT_OP;
        } else {
            op = GE_OP;
        }
        
        Exp* right = parseExpression();
        left = new BinaryExp(left, right, op);
    }
    
    return left;
}

Exp* Parser::parseCExp() {
    return parseLogicalOrExpr();
}

Exp* Parser::parseUnaryExpr() {
    if (match(Token::PLUS) || match(Token::MINUS) || match(Token::NOT)) {
        Token::Type op = previous->type;
        Exp* right = parseUnaryExpr();
        
        if (op == Token::NOT) {
            // Implement NOT operation (can be done as a comparison with 0)
            // This is simplified - in a full implementation you'd handle this differently
            return new BinaryExp(new NumberExp(0), right, EQ_OP);
        } else if (op == Token::MINUS) {
            // Implement unary minus as 0 - expr
            return new BinaryExp(new NumberExp(0), right, MINUS_OP);
        } else {
            // Unary plus is a no-op
            return right;
        }
    }
    
    return parsePrimaryExpr();
}

Exp* Parser::parsePrimaryExpr() {
    Exp* expr;
    
    if (match(Token::ID)) {
        string name = previous->text;
        
        // Check for struct field access
        if (match(Token::DOT)) {
            if (!match(Token::ID)) {
                cout << "Error: Expected field name after '.'." << endl;
                exit(1);
            }
            
            string fieldName = previous->text;
            expr = new StructFieldAccess(new IdentifierExp(name), fieldName);
        } else {
            expr = new IdentifierExp(name);
        }
    } else if (match(Token::NUM)) {
        expr = new NumberExp(stoi(previous->text));
    } else if (match(Token::TRUE)) {
        expr = new BoolExp(true);
    } else if (match(Token::FALSE)) {
        expr = new BoolExp(false);
    } else if (match(Token::STRING_LIT)) {
        expr = new StringExp(previous->text);
    } else if (match(Token::PI)) {
        expr = parseCExp();
        
        if (!match(Token::PD)) {
            cout << "Error: Expected ')' after expression." << endl;
            exit(1);
        }
    } else {
        cout << "Error: Unexpected token in expression." << endl;
        exit(1);
    }
    
    return expr;
}

