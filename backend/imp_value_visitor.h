#ifndef IMP_VALUE_VISITOR_H
#define IMP_VALUE_VISITOR_H

#include "imp_value.h"

// Forward declarations
class BinaryExp;
class UnaryExp;
class NumberExp;
class StringExp;
class BoolExp;
class IdentifierExp;
class FieldAccessExp;
class IndexExp;
class SliceExp;
class FunctionCallExp;
class StructLiteralExp;

class ExprStmt;
class AssignStmt;
class ShortVarDecl;
class IncDecStmt;
class IfStmt;
class ForStmt;
class ReturnStmt;
class VarDecl;

class TypeDecl;
class FuncDecl;
class Block;
class ImportDecl;
class Program;

class ImpValueVisitor {
public:
    virtual ImpValue visit(BinaryExp* exp) = 0;
    virtual ImpValue visit(UnaryExp* exp) = 0;
    virtual ImpValue visit(NumberExp* exp) = 0;
    virtual ImpValue visit(StringExp* exp) = 0;
    virtual ImpValue visit(BoolExp* exp) = 0;
    virtual ImpValue visit(IdentifierExp* exp) = 0;
    virtual ImpValue visit(FieldAccessExp* exp) = 0;
    virtual ImpValue visit(IndexExp* exp) = 0;
    virtual ImpValue visit(SliceExp* exp) = 0;
    virtual ImpValue visit(FunctionCallExp* exp) = 0;
    virtual ImpValue visit(StructLiteralExp* exp) = 0;
    
    virtual void visit(ExprStmt* stmt) = 0;
    virtual void visit(AssignStmt* stmt) = 0;
    virtual void visit(ShortVarDecl* stmt) = 0;
    virtual void visit(IncDecStmt* stmt) = 0;
    virtual void visit(IfStmt* stmt) = 0;
    virtual void visit(ForStmt* stmt) = 0;
    virtual void visit(ReturnStmt* stmt) = 0;
    virtual void visit(VarDecl* stmt) = 0;
    
    virtual void visit(TypeDecl* decl) = 0;
    virtual void visit(FuncDecl* decl) = 0;
    virtual void visit(Block* block) = 0;
    virtual void visit(ImportDecl* decl) = 0;
    virtual void visit(Program* program) = 0;
};

#endif
