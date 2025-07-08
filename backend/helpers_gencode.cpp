#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
#include "exp.h"
#include "imp_value_visitor.h"
#include "visitor.h"
#include "environment.hh"

using namespace std;

class StringCollectorVisitor : public ImpValueVisitor {
public:
    std::unordered_map<std::string, std::string>& string_literals;
    int& string_counter;
    StringCollectorVisitor(std::unordered_map<std::string, std::string>& literals, int& counter)
        : string_literals(literals), string_counter(counter) {}
    ImpValue visit(StringExp* exp) override {
        if (string_literals.find(exp->value) == string_literals.end()) {
            string label = "string_" + to_string(string_counter++);
            string_literals[exp->value] = label;
        }
        return ImpValue();
    }
    ImpValue visit(BinaryExp* exp) override { if(exp->left) exp->left->accept(this); if(exp->right) exp->right->accept(this); return ImpValue(); }
    ImpValue visit(UnaryExp* exp) override { if(exp->exp) exp->exp->accept(this); return ImpValue(); }
    void visit(IfStmt* stmt) override { if(stmt->condition) stmt->condition->accept(this); if(stmt->thenBlock) stmt->thenBlock->accept(this); if (stmt->elseBlock) stmt->elseBlock->accept(this); }
    void visit(ForStmt* stmt) override { if (stmt->init) stmt->init->accept(this); if (stmt->condition) stmt->condition->accept(this); if (stmt->post) stmt->post->accept(this); if(stmt->body) stmt->body->accept(this); }
    void visit(Block* block) override { for (auto s : block->statements) if(s) s->accept(this); }
    void visit(Program* program) override { for (auto f : program->functions) if(f) f->accept(this); }
    void visit(FuncDecl* decl) override { if (decl->body) decl->body->accept(this); }
    void visit(VarDecl* stmt) override { for (auto v : stmt->values) if (v) v->accept(this); }
    void visit(ShortVarDecl* stmt) override { for (auto v : stmt->values) if (v) v->accept(this); }
    void visit(AssignStmt* stmt) override { if(stmt->lhs) stmt->lhs->accept(this); if(stmt->rhs) stmt->rhs->accept(this); }
    void visit(ExprStmt* stmt) override { if(stmt->expression) stmt->expression->accept(this); }
    void visit(ReturnStmt* stmt) override { if(stmt->expression) stmt->expression->accept(this); }
    ImpValue visit(FunctionCallExp* exp) override { for(auto arg : exp->args) if (arg) arg->accept(this); return ImpValue(); }
    ImpValue visit(NumberExp* exp) override { return ImpValue(); }
    ImpValue visit(BoolExp* exp) override { return ImpValue(); }
    ImpValue visit(IdentifierExp* exp) override { return ImpValue(); }
    ImpValue visit(FieldAccessExp* exp) override { return ImpValue(); }
    ImpValue visit(IndexExp* exp) override { return ImpValue(); }
    ImpValue visit(SliceExp* exp) override { return ImpValue(); }
    ImpValue visit(StructLiteralExp* exp) override { for(auto v : exp->values) if(v) v->accept(this); return ImpValue(); }
    void visit(IncDecStmt* stmt) override {}
    void visit(TypeDecl* decl) override {}
    void visit(ImportDecl* decl) override {}
};

class StructCollectorVisitor : public Visitor {
public:
    Environment& env;
    StructCollectorVisitor(Environment& env) : env(env) {}

    void visit(Program* program) override {
        for (auto type_decl : program->types) {
            type_decl->accept(this);
        }
    }

    void visit(TypeDecl* decl) override {
        StructInfo sinfo;
        sinfo.name = decl->name;
        int current_offset = 0;
        
        if (auto st = dynamic_cast<StructType*>(decl->structType)) {
            for (auto field_pair : st->fields) {
                string field_name = field_pair.first;
                Type* field_type_node = field_pair.second;
                
                FieldInfo finfo;
                if (auto bt = dynamic_cast<BasicType*>(field_type_node)) {
                    finfo.type_name = bt->typeName;
                    finfo.type = ImpValue::get_basic_type(bt->typeName);
                    finfo.size = 8;
                } else {
                     throw runtime_error("Structs anidados o tipos complejos no soportados.");
                }

                sinfo.fields[field_name] = finfo;
                sinfo.offsets[field_name] = current_offset;
                current_offset += finfo.size;
            }
        }
        sinfo.size = current_offset;
        env.add_struct(decl->name, sinfo);
    }
    
    void visit(BinaryExp* exp) override {} void visit(UnaryExp* exp) override {} void visit(NumberExp* exp) override {}
    void visit(StringExp* exp) override {} void visit(BoolExp* exp) override {} void visit(IdentifierExp* exp) override {}
    void visit(FieldAccessExp* exp) override {} void visit(IndexExp* exp) override {} void visit(SliceExp* exp) override {}
    void visit(FunctionCallExp* exp) override {} void visit(StructLiteralExp* exp) override {} void visit(BasicType* type) override {}
    void visit(StructType* type) override {} void visit(IdentifierType* type) override {} void visit(ExprStmt* stmt) override {}
    void visit(AssignStmt* stmt) override {} void visit(ShortVarDecl* stmt) override {} void visit(IncDecStmt* stmt) override {}
    void visit(IfStmt* stmt) override {} void visit(ForStmt* stmt) override {} void visit(ReturnStmt* stmt) override {}
    void visit(VarDecl* stmt) override {} void visit(FuncDecl* decl) override {} void visit(Block* block) override {}
    void visit(ImportDecl* decl) override {}
};