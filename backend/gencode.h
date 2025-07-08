#ifndef GENCODE_H
#define GENCODE_H

#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
#include "exp.h"
#include "imp_value_visitor.h"
#include "environment.hh"

using namespace std;

// Declaración adelantada
class OffsetCalculator;

class GoCodeGen : public ImpValueVisitor {
private:
    Environment<ImpValue> env;
    int current_offset;
    std::unordered_map<std::string, int> stack_offsets;
    std::unordered_map<std::string, std::string> string_literals;
    int label_counter;
    int string_counter;
    std::ostream& output;
    
    // Métodos auxiliares
    string new_label();
    void generate_prologue();
    void generate_epilogue();
    void generate_string_literals();
    
    // Clase amiga para calcular offsets
    friend class OffsetCalculator;
    
public:
    GoCodeGen(std::ostream& out = std::cout);
    void generateCode(Program* program);
    
    // Visitantes de expresiones
    ImpValue visit(BinaryExp* exp) override;
    ImpValue visit(UnaryExp* exp) override;
    ImpValue visit(NumberExp* exp) override;
    ImpValue visit(StringExp* exp) override;
    ImpValue visit(BoolExp* exp) override;
    ImpValue visit(IdentifierExp* exp) override;
    ImpValue visit(FieldAccessExp* exp) override;
    ImpValue visit(IndexExp* exp) override;
    ImpValue visit(SliceExp* exp) override;
    ImpValue visit(FunctionCallExp* exp) override;
    ImpValue visit(StructLiteralExp* exp) override;
    
    // Visitantes de sentencias
    void visit(ExprStmt* stmt) override;
    void visit(AssignStmt* stmt) override;
    void visit(ShortVarDecl* stmt) override;
    void visit(IncDecStmt* stmt) override;
    void visit(IfStmt* stmt) override;
    void visit(ForStmt* stmt) override;
    void visit(ReturnStmt* stmt) override;
    void visit(VarDecl* stmt) override;
    
    // Visitantes de declaraciones
    void visit(TypeDecl* decl) override;
    void visit(FuncDecl* decl) override;
    void visit(Block* block) override;
    void visit(ImportDecl* decl) override;
    void visit(Program* program) override;
};

// Clase separada para calcular offsets de pila
class OffsetCalculator : public ImpValueVisitor {
private:
    Environment<ImpValue> env;
    int current_offset;
    std::unordered_map<std::string, int> stack_offsets;

public:
    OffsetCalculator();
    std::unordered_map<std::string, int> calculateOffsets(Program* program);
    int getStackSize() const { return -current_offset; }
    int getCurrentOffset() const { return current_offset; } // Método para obtener el offset actual
    
    // Visitantes de expresiones (implementación mínima solo para recorrido)
    ImpValue visit(BinaryExp* exp) override;
    ImpValue visit(UnaryExp* exp) override;
    ImpValue visit(NumberExp* exp) override;
    ImpValue visit(StringExp* exp) override;
    ImpValue visit(BoolExp* exp) override;
    ImpValue visit(IdentifierExp* exp) override;
    ImpValue visit(FieldAccessExp* exp) override;
    ImpValue visit(IndexExp* exp) override;
    ImpValue visit(SliceExp* exp) override;
    ImpValue visit(FunctionCallExp* exp) override;
    ImpValue visit(StructLiteralExp* exp) override;
    
    // Visitantes de sentencias
    void visit(ExprStmt* stmt) override;
    void visit(AssignStmt* stmt) override;
    void visit(ShortVarDecl* stmt) override;
    void visit(IncDecStmt* stmt) override;
    void visit(IfStmt* stmt) override;
    void visit(ForStmt* stmt) override;
    void visit(ReturnStmt* stmt) override;
    void visit(VarDecl* stmt) override;
    
    // Visitantes de declaraciones
    void visit(TypeDecl* decl) override;
    void visit(FuncDecl* decl) override;
    void visit(Block* block) override;
    void visit(ImportDecl* decl) override;
    void visit(Program* program) override;
};

#endif
