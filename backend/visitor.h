#ifndef VISITOR_GO_H
#define VISITOR_GO_H

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

class BasicType;
class StructType;
class IdentifierType;

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

class Visitor {
public:
    // Expresiones
    virtual void visit(BinaryExp* exp) = 0;
    virtual void visit(UnaryExp* exp) = 0;
    virtual void visit(NumberExp* exp) = 0;
    virtual void visit(StringExp* exp) = 0;
    virtual void visit(BoolExp* exp) = 0;
    virtual void visit(IdentifierExp* exp) = 0;
    virtual void visit(FieldAccessExp* exp) = 0;
    virtual void visit(IndexExp* exp) = 0;
    virtual void visit(SliceExp* exp) = 0;
    virtual void visit(FunctionCallExp* exp) = 0;
    virtual void visit(StructLiteralExp* exp) = 0;
    
    // Tipos
    virtual void visit(BasicType* type) = 0;
    virtual void visit(StructType* type) = 0;
    virtual void visit(IdentifierType* type) = 0;
    
    // Sentencias
    virtual void visit(ExprStmt* stmt) = 0;
    virtual void visit(AssignStmt* stmt) = 0;
    virtual void visit(ShortVarDecl* stmt) = 0;
    virtual void visit(IncDecStmt* stmt) = 0;
    virtual void visit(IfStmt* stmt) = 0;
    virtual void visit(ForStmt* stmt) = 0;
    virtual void visit(ReturnStmt* stmt) = 0;
    virtual void visit(VarDecl* stmt) = 0;
    
    // Declaraciones
    virtual void visit(TypeDecl* decl) = 0;
    virtual void visit(FuncDecl* decl) = 0;
    virtual void visit(Block* block) = 0;
    virtual void visit(ImportDecl* decl) = 0;
    virtual void visit(Program* program) = 0;
};

class PrintVisitor : public Visitor {
private:
    int indentLevel;
    void printIndent();
    void increaseIndent();
    void decreaseIndent();
    
public:
    PrintVisitor();
    
    // Expresiones
    void visit(BinaryExp* exp) override;
    void visit(UnaryExp* exp) override;
    void visit(NumberExp* exp) override;
    void visit(StringExp* exp) override;
    void visit(BoolExp* exp) override;
    void visit(IdentifierExp* exp) override;
    void visit(FieldAccessExp* exp) override;
    void visit(IndexExp* exp) override;
    void visit(SliceExp* exp) override;
    void visit(FunctionCallExp* exp) override;
    void visit(StructLiteralExp* exp) override;
    
    // Tipos
    void visit(BasicType* type) override;
    void visit(StructType* type) override;
    void visit(IdentifierType* type) override;
    
    // Sentencias
    void visit(ExprStmt* stmt) override;
    void visit(AssignStmt* stmt) override;
    void visit(ShortVarDecl* stmt) override;
    void visit(IncDecStmt* stmt) override;
    void visit(IfStmt* stmt) override;
    void visit(ForStmt* stmt) override;
    void visit(ReturnStmt* stmt) override;
    void visit(VarDecl* stmt) override;
    
    // Declaraciones
    void visit(TypeDecl* decl) override;
    void visit(FuncDecl* decl) override;
    void visit(Block* block) override;
    void visit(ImportDecl* decl) override;
    void visit(Program* program) override;
    
    void print(Program* program);
};

#endif // VISITOR_GO_H
