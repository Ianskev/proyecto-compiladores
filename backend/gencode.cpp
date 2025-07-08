#include "gencode.h"
#include <iostream>
#include <algorithm>

using namespace std;

// --- StringCollectorVisitor ---
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
    ImpValue visit(BinaryExp* exp) override { exp->left->accept(this); exp->right->accept(this); return ImpValue(); }
    ImpValue visit(UnaryExp* exp) override { exp->exp->accept(this); return ImpValue(); }
    void visit(IfStmt* stmt) override { stmt->condition->accept(this); stmt->thenBlock->accept(this); if (stmt->elseBlock) stmt->elseBlock->accept(this); }
    void visit(ForStmt* stmt) override { if (stmt->init) stmt->init->accept(this); if (stmt->condition) stmt->condition->accept(this); if (stmt->post) stmt->post->accept(this); stmt->body->accept(this); }
    void visit(Block* block) override { for (auto s : block->statements) s->accept(this); }
    void visit(Program* program) override { for (auto f : program->functions) f->accept(this); }
    void visit(FuncDecl* decl) override { decl->body->accept(this); }
    void visit(VarDecl* stmt) override { for (auto v : stmt->values) if (v) v->accept(this); }
    void visit(ShortVarDecl* stmt) override { for (auto v : stmt->values) if (v) v->accept(this); }
    void visit(AssignStmt* stmt) override { stmt->lhs->accept(this); stmt->rhs->accept(this); }
    void visit(ExprStmt* stmt) override { stmt->expression->accept(this); }
    void visit(ReturnStmt* stmt) override { if(stmt->expression) stmt->expression->accept(this); }
    ImpValue visit(FunctionCallExp* exp) override { for(auto arg : exp->args) if (arg) arg->accept(this); return ImpValue(); }
    ImpValue visit(NumberExp* exp) override { return ImpValue(); }
    ImpValue visit(BoolExp* exp) override { return ImpValue(); }
    ImpValue visit(IdentifierExp* exp) override { return ImpValue(); }
    ImpValue visit(FieldAccessExp* exp) override { return ImpValue(); }
    ImpValue visit(IndexExp* exp) override { return ImpValue(); }
    ImpValue visit(SliceExp* exp) override { return ImpValue(); }
    ImpValue visit(StructLiteralExp* exp) override { return ImpValue(); }
    void visit(IncDecStmt* stmt) override {}
    void visit(TypeDecl* decl) override {}
    void visit(ImportDecl* decl) override {}
};

// =================== Implementación de GoCodeGen (con funciones) ===================

GoCodeGen::GoCodeGen(std::ostream& out) 
    : current_offset(0), label_counter(0), string_counter(0), output(out) {
    env.add_level();
}

string GoCodeGen::new_label() { return "L" + to_string(label_counter++); }

void GoCodeGen::generateCode(Program* program) {
    // PASO 1: Recolectar información (strings y definiciones de funciones)
    StringCollectorVisitor string_collector(this->string_literals, this->string_counter);
    program->accept(&string_collector);
    
    for (auto f : program->functions) {
        env.add_function(f->name, {calculate_block_size(f->body)});
    }
    
    // PASO 2: Generar el código
    current_offset = 0;
    label_counter = 0;
    env.clear();
    env.add_level();
    for (auto f : program->functions) {
        env.add_function(f->name, {calculate_block_size(f->body)});
    }

    generate_prologue();
    program->accept(this); // Genera el código para todas las funciones, incluyendo main
    generate_epilogue();
}

void GoCodeGen::generate_prologue() {
    output << ".data" << endl;
    output << "print_fmt: .string \"%ld\\n\"" << endl;
    output << "print_str_fmt: .string \"%s\\n\"" << endl;
    output << "print_bool_true: .string \"true\\n\"" << endl;
    output << "print_bool_false: .string \"false\\n\"" << endl;
    generate_string_literals();
    output << ".text" << endl;
}

void GoCodeGen::generate_epilogue() {
    output << ".section .note.GNU-stack,\"\",@progbits" << endl;
}

void GoCodeGen::generate_string_literals() {
    for (const auto& kv : string_literals) {
        string val = kv.first;
        size_t pos = 0;
        while ((pos = val.find("\\", pos)) != std::string::npos) {
            val.replace(pos, 1, "\\\\");
            pos += 2;
        }
        output << kv.second << ": .string \"" << val << "\"" << endl;
    }
}

// --- Calculadores de tamaño de stack (sin cambios) ---
int GoCodeGen::calculate_stmt_size(Stmt* stmt) {
    int size = 0;
    if (auto s = dynamic_cast<VarDecl*>(stmt)) size += s->names.size() * 8;
    else if (auto s = dynamic_cast<ShortVarDecl*>(stmt)) size += s->identifiers.size() * 8;
    else if (auto s = dynamic_cast<IfStmt*>(stmt)) {
        size += calculate_block_size(s->thenBlock);
        if (s->elseBlock) size += calculate_block_size(s->elseBlock);
    } else if (auto s = dynamic_cast<ForStmt*>(stmt)) {
        if (s->init) size += calculate_stmt_size(s->init);
        size += calculate_block_size(s->body);
    }
    return size;
}

int GoCodeGen::calculate_block_size(Block* block) {
    int size = 0;
    for(auto s : block->statements) size += calculate_stmt_size(s);
    return size;
}

int GoCodeGen::calculate_stack_size(Program* p) { return 0; } // Ya no es necesario a nivel de programa

// --- Visitantes de AST ---

void GoCodeGen::visit(Program* program) {
    // Generar código para todas las funciones
    for (auto func : program->functions) {
        func->accept(this);
    }
}

void GoCodeGen::visit(FuncDecl* decl) {
    // --- LÓGICA DE GENERACIÓN DE FUNCIÓN ---
    output << ".globl " << decl->name << endl;
    output << decl->name << ":" << endl;
    output << "  pushq %rbp" << endl;
    output << "  movq %rsp, %rbp" << endl;

    int stack_size = env.get_function(decl->name).stack_size;
    if (stack_size > 0) {
        if (stack_size % 16 != 0) stack_size = ((stack_size + 15) / 16) * 16;
        output << "  subq $" << stack_size << ", %rsp" << endl;
    }

    current_offset = 0; // Reiniciar offset para cada función
    env.add_level();

    decl->body->accept(this);

    // Epílogo de la función
    if (decl->name == "main") {
        output << "  movq $0, %rax" << endl;
    }
    output << "  leave" << endl;
    output << "  ret" << endl;
    
    env.remove_level();
}

void GoCodeGen::visit(Block* block) {
    env.add_level();
    for (auto stmt : block->statements) {
        stmt->accept(this);
    }
    env.remove_level();
}

// --- Visitantes de Expresiones y Sentencias (el resto del código es casi igual) ---
void GoCodeGen::visit(ExprStmt* stmt) { stmt->expression->accept(this); }
void GoCodeGen::visit(AssignStmt* stmt) {
    stmt->rhs->accept(this);
    if (auto id = dynamic_cast<IdentifierExp*>(stmt->lhs)) {
        if (env.check(id->name)) {
            int offset = env.lookup(id->name).offset;
            output << "  movq %rax, " << offset << "(%rbp) # Almacenar en " << id->name << endl;
        }
    }
}
void GoCodeGen::visit(ShortVarDecl* stmt) {
    auto varIt = stmt->identifiers.begin();
    auto valIt = stmt->values.begin();
    while (varIt != stmt->identifiers.end()) {
        current_offset -= 8;
        ImpValue val;
        if(valIt != stmt->values.end()) val = (*valIt)->accept(this);
        env.add_var(*varIt, current_offset, val.type);
        if (valIt != stmt->values.end()) {
            output << "  movq %rax, " << current_offset << "(%rbp) # Inicializar " << *varIt << endl;
            ++valIt;
        } else {
            output << "  movq $0, " << current_offset << "(%rbp) # Inicializar por defecto " << *varIt << endl;
        }
        ++varIt;
    }
}
void GoCodeGen::visit(IncDecStmt* stmt) {
    if (env.check(stmt->variable)) {
        int offset = env.lookup(stmt->variable).offset;
        output << "  movq " << offset << "(%rbp), %rax" << endl;
        if (stmt->isIncrement) output << "  incq %rax" << endl;
        else output << "  decq %rax" << endl;
        output << "  movq %rax, " << offset << "(%rbp)" << endl;
    }
}
void GoCodeGen::visit(IfStmt* stmt) {
    string else_label = new_label(), end_label = new_label();
    stmt->condition->accept(this);
    output << "  cmpq $0, %rax" << endl;
    output << "  je " << else_label << endl;
    stmt->thenBlock->accept(this);
    output << "  jmp " << end_label << endl;
    output << else_label << ":" << endl;
    if (stmt->elseBlock) stmt->elseBlock->accept(this);
    output << end_label << ":" << endl;
}
void GoCodeGen::visit(ForStmt* stmt) {
    string start_label = new_label(), end_label = new_label();
    env.add_level();
    if (stmt->init) stmt->init->accept(this);
    output << start_label << ":" << endl;
    if (stmt->condition) {
        stmt->condition->accept(this);
        output << "  cmpq $0, %rax" << endl;
        output << "  je " << end_label << endl;
    }
    stmt->body->accept(this);
    if (stmt->post) stmt->post->accept(this);
    output << "  jmp " << start_label << endl;
    output << end_label << ":" << endl;
    env.remove_level();
}
void GoCodeGen::visit(ReturnStmt* stmt) { if (stmt->expression) stmt->expression->accept(this); }
void GoCodeGen::visit(VarDecl* stmt) {
    auto nameIt = stmt->names.begin();
    auto valIt = stmt->values.begin();
    while (nameIt != stmt->names.end()) {
        current_offset -= 8;
        ImpValue val;
        if(valIt != stmt->values.end()) val = (*valIt)->accept(this);
        env.add_var(*nameIt, current_offset, val.type);
        if (valIt != stmt->values.end()) {
            output << "  movq %rax, " << current_offset << "(%rbp) # Inicializar " << *nameIt << endl;
            ++valIt;
        } else {
            output << "  movq $0, " << current_offset << "(%rbp) # Inicialización por defecto " << *nameIt << endl;
        }
        ++nameIt;
    }
}
void GoCodeGen::visit(TypeDecl* decl) {}
void GoCodeGen::visit(ImportDecl* decl) {}
ImpValue GoCodeGen::visit(BinaryExp* exp) {
    exp->left->accept(this);
    output << "  pushq %rax" << endl;
    exp->right->accept(this);
    output << "  movq %rax, %rbx" << endl;
    output << "  popq %rax" << endl;
    string set_instruction;
    switch (exp->op) {
        case PLUS_OP: output << "  addq %rbx, %rax" << endl; return ImpValue(TINT);
        case MINUS_OP: output << "  subq %rbx, %rax" << endl; return ImpValue(TINT);
        case MUL_OP: output << "  imulq %rbx, %rax" << endl; return ImpValue(TINT);
        case DIV_OP: output << "  cqto" << endl; output << "  idivq %rbx" << endl; return ImpValue(TINT);
        case MOD_OP: output << "  cqto" << endl; output << "  idivq %rbx" << endl; output << "  movq %rdx, %rax" << endl; return ImpValue(TINT);
        case LT_OP: set_instruction = "setl"; goto compare;
        case LE_OP: set_instruction = "setle"; goto compare;
        case GT_OP: set_instruction = "setg"; goto compare;
        case GE_OP: set_instruction = "setge"; goto compare;
        case EQ_OP: set_instruction = "sete"; goto compare;
        case NE_OP: set_instruction = "setne"; goto compare;
        case AND_OP: {
            string f_l = new_label(), e_l = new_label();
            output << "  cmpq $0, %rax" << endl; output << "  je " << f_l << endl;
            output << "  cmpq $0, %rbx" << endl; output << "  je " << f_l << endl;
            output << "  movq $1, %rax" << endl; output << "  jmp " << e_l << endl;
            output << f_l << ":" << endl; output << "  movq $0, %rax" << endl;
            output << e_l << ":" << endl; break;
        }
        case OR_OP: {
            string t_l = new_label(), e_l = new_label();
            output << "  cmpq $0, %rax" << endl; output << "  jne " << t_l << endl;
            output << "  cmpq $0, %rbx" << endl; output << "  je " << e_l << endl;
            output << t_l << ":" << endl; output << "  movq $1, %rax" << endl;
            output << e_l << ":" << endl; break;
        }
    }
    return ImpValue(true);
compare:
    output << "  cmpq %rbx, %rax" << endl; output << "  " << set_instruction << " %al" << endl;
    output << "  movzbq %al, %rax" << endl; return ImpValue(true);
}
ImpValue GoCodeGen::visit(UnaryExp* exp) {
    exp->exp->accept(this);
    switch (exp->op) {
        case UMINUS_OP: output << "  negq %rax" << endl; break;
        case NOT_OP: output << "  cmpq $0, %rax" << endl; output << "  sete %al" << endl; output << "  movzbq %al, %rax" << endl; break;
        case UPLUS_OP: break;
    }
    return ImpValue();
}
ImpValue GoCodeGen::visit(NumberExp* exp) { output << "  movq $" << exp->value << ", %rax" << endl; return ImpValue(exp->value); }
ImpValue GoCodeGen::visit(StringExp* exp) { string label = string_literals[exp->value]; output << "  leaq " << label << "(%rip), %rax" << endl; return ImpValue(exp->value); }
ImpValue GoCodeGen::visit(BoolExp* exp) { output << "  movq $" << (exp->value ? 1 : 0) << ", %rax" << endl; return ImpValue(exp->value); }
ImpValue GoCodeGen::visit(IdentifierExp* exp) {
    if (env.check(exp->name)) {
        VarInfo info = env.lookup(exp->name);
        output << "  movq " << info.offset << "(%rbp), %rax  # Cargar " << exp->name << endl;
        ImpValue val; val.type = info.type; return val;
    }
    output << "  # Advertencia: Variable no definida: " << exp->name << endl; output << "  movq $0, %rax" << endl;
    return ImpValue();
}
ImpValue GoCodeGen::visit(FunctionCallExp* exp) {
    if (exp->funcName == "fmt.Println") {
        for (auto arg : exp->args) {
            if (!arg) continue;
            ImpValue val = arg->accept(this);
            string fmt_reg = "%rdi";
            if (val.type == TINT) { output << "  leaq print_fmt(%rip), " << fmt_reg << endl; output << "  movq %rax, %rsi" << endl; }
            else if (val.type == TSTRING) { output << "  leaq print_str_fmt(%rip), " << fmt_reg << endl; output << "  movq %rax, %rsi" << endl; }
            else if (val.type == TBOOL) {
                string t_l = new_label(), e_l = new_label();
                output << "  cmpq $0, %rax" << endl; output << "  jne " << t_l << endl;
                output << "  leaq print_bool_false(%rip), %rdi" << endl; output << "  jmp " << e_l << endl;
                output << t_l << ":" << endl; output << "  leaq print_bool_true(%rip), %rdi" << endl;
                output << e_l << ":" << endl;
            } else { output << "  leaq print_fmt(%rip), " << fmt_reg << endl; output << "  movq %rax, %rsi" << endl; }
            output << "  xorq %rax, %rax" << endl;
            output << "  call printf@PLT" << endl;
        }
    } else {
        // --- NUEVA LÓGICA PARA LLAMADAS A FUNCIONES DEFINIDAS POR EL USUARIO ---
        if (env.has_function(exp->funcName)) {
            output << "  call " << exp->funcName << endl;
        } else {
            output << "  # Advertencia: Llamada a función no definida " << exp->funcName << endl;
        }
    }
    return ImpValue();
}
ImpValue GoCodeGen::visit(FieldAccessExp* exp) { return ImpValue(); }
ImpValue GoCodeGen::visit(IndexExp* exp) { return ImpValue(); }
ImpValue GoCodeGen::visit(SliceExp* exp) { return ImpValue(); }
ImpValue GoCodeGen::visit(StructLiteralExp* exp) { return ImpValue(); }