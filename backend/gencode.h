#ifndef GENCODE_H
#define GENCODE_H

#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
#include "exp.h"
#include "imp_value_visitor.h"
#include "environment.hh" // Incluimos nuestro nuevo environment

using namespace std;
class StringCollectorVisitor;
class GoCodeGen : public ImpValueVisitor {
private:
    Environment env; // Usamos nuestro nuevo Environment
    int current_offset;
    std::unordered_map<std::string, std::string> string_literals;
    int label_counter;
    int string_counter;
    std::ostream& output;
    string current_epilogue_label; // Para manejar los 'return'

    string new_label();
    void generate_prologue();
    void generate_epilogue();
    void generate_string_literals();
    int calculate_stack_size(Program* p); // Para calcular el espacio total del stack

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

    // Métodos para el cálculo de tamaño de stack (helpers)
    int calculate_block_size(Block* block);
    int calculate_stmt_size(Stmt* stmt);
};

#endif