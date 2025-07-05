#ifndef VISITOR_H
#define VISITOR_H
#include "exp.h"
#include <list>

// Forward declarations for existing node types
class BinaryExp;
class NumberExp;
class BoolExp;
class IdentifierExp;
class AssignStatement;
class PrintStatement;
class IfStatement;
class WhileStatement;
class VarDec;
class VarDecList;
class StatementList;
class Body;
class Program;

// Forward declarations for new Go-specific node types
class StringExp;
class StructDeclaration;
class StructFieldAccess;
class ImportDeclaration;
class PackageDeclaration;
class GoProgram;

class Visitor {
public:
    // Existing visit methods
    virtual int visit(BinaryExp* exp) = 0;
    virtual int visit(NumberExp* exp) = 0;
    virtual int visit(BoolExp* exp) = 0;
    virtual int visit(IdentifierExp* exp) = 0;
    virtual void visit(AssignStatement* stm) = 0;
    virtual void visit(PrintStatement* stm) = 0;
    virtual void visit(IfStatement* stm) = 0;
    virtual void visit(WhileStatement* stm) = 0;
    virtual void visit(VarDec* stm) = 0;
    virtual void visit(VarDecList* stm) = 0;
    virtual void visit(StatementList* stm) = 0;
    virtual void visit(Body* b) = 0;
    
    // New visit methods for Go-specific node types
    virtual int visit(StringExp* exp) = 0;
    virtual void visit(StructDeclaration* stm) = 0;
    virtual int visit(StructFieldAccess* exp) = 0;
    virtual void visit(ImportDeclaration* stm) = 0;
    virtual void visit(PackageDeclaration* stm) = 0;
    virtual void visit(GoProgram* prog) = 0;
};

class PrintVisitor : public Visitor {
public:
    void imprimir(Program* program);
    void imprimirGo(GoProgram* program);
    
    // Existing implementation methods
    int visit(BinaryExp* exp) override;
    int visit(NumberExp* exp) override;
    int visit(BoolExp* exp) override;
    int visit(IdentifierExp* exp) override;
    void visit(AssignStatement* stm) override;
    void visit(PrintStatement* stm) override;
    void visit(IfStatement* stm) override;
    void visit(WhileStatement* stm) override;
    void visit(VarDec* stm) override;
    void visit(VarDecList* stm) override;
    void visit(StatementList* stm) override;
    void visit(Body* b) override;
    
    // New implementation methods for Go-specific node types
    int visit(StringExp* exp) override;
    void visit(StructDeclaration* stm) override;
    int visit(StructFieldAccess* exp) override;
    void visit(ImportDeclaration* stm) override;
    void visit(PackageDeclaration* stm) override;
    void visit(GoProgram* prog) override;
};

#endif