#include "gencode.h"
#include <iostream>
#include <algorithm>

using namespace std;

// =================== Nuevo: StringCollectorVisitor (Corregido) ===================
// Este visitante SÓLO recorre el AST para encontrar literales de cadena.
// No escribe ningún código ensamblador.
class StringCollectorVisitor : public ImpValueVisitor {
public:
    std::unordered_map<std::string, std::string>& string_literals;
    int& string_counter;

    StringCollectorVisitor(std::unordered_map<std::string, std::string>& literals, int& counter)
        : string_literals(literals), string_counter(counter) {}

    // El único método que hace algo es para StringExp
    ImpValue visit(StringExp* exp) override {
        if (string_literals.find(exp->value) == string_literals.end()) {
            string label = "string_" + to_string(string_counter++);
            string_literals[exp->value] = label;
        }
        return ImpValue();
    }

    // El resto de los métodos solo se encargan de continuar el recorrido
    ImpValue visit(BinaryExp* exp) override {
        exp->left->accept(this);
        exp->right->accept(this);
        return ImpValue();
    }
    ImpValue visit(UnaryExp* exp) override {
        exp->exp->accept(this);
        return ImpValue();
    }
    void visit(IfStmt* stmt) override {
        stmt->condition->accept(this);
        stmt->thenBlock->accept(this);
        if (stmt->elseBlock) stmt->elseBlock->accept(this);
    }
    void visit(ForStmt* stmt) override {
        if (stmt->init) stmt->init->accept(this);
        if (stmt->condition) stmt->condition->accept(this);
        if (stmt->post) stmt->post->accept(this);
        stmt->body->accept(this);
    }
    void visit(Block* block) override {
        for (auto s : block->statements) s->accept(this);
    }
    void visit(Program* program) override {
        for (auto f : program->functions) f->accept(this);
    }
    void visit(FuncDecl* decl) override {
        decl->body->accept(this);
    }
    void visit(VarDecl* stmt) override {
        for (auto v : stmt->values) {
            if (v) v->accept(this);
        }
    }
    void visit(ShortVarDecl* stmt) override {
        for (auto v : stmt->values) {
            if (v) v->accept(this);
        }
    }
    void visit(AssignStmt* stmt) override {
        stmt->lhs->accept(this);
        stmt->rhs->accept(this);
    }
    void visit(ExprStmt* stmt) override {
        stmt->expression->accept(this);
    }
    void visit(ReturnStmt* stmt) override {
        if(stmt->expression) stmt->expression->accept(this);
    }
    
    // --- CORRECCIÓN AQUÍ ---
    ImpValue visit(FunctionCallExp* exp) override {
        for(auto arg : exp->args) {
            if (arg) arg->accept(this);
        }
        return ImpValue(); // Devolver un ImpValue
    }

    // Métodos vacíos para nodos que no contienen expresiones
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


// =================== Implementación de GoCodeGen (Corregido) ===================

GoCodeGen::GoCodeGen(std::ostream& out) 
    : current_offset(0), label_counter(0), string_counter(0), output(out) {
    env.add_level(); // Nivel global
}

string GoCodeGen::new_label() {
    return "L" + to_string(label_counter++);
}

void GoCodeGen::generateCode(Program* program) {
    // --- PASO 1: Recolectar información (sin generar código) ---
    // Usamos nuestro nuevo visitante para recolectar strings.
    StringCollectorVisitor string_collector(this->string_literals, this->string_counter);
    program->accept(&string_collector);
    
    // Calculamos el tamaño del stack.
    int stack_size = calculate_stack_size(program);
    
    // --- PASO 2: Generar el código en el orden correcto ---
    current_offset = 0;
    label_counter = 0;
    env.clear();
    env.add_level();

    generate_prologue(stack_size);
    program->accept(this); // AHORA SÍ, la única llamada que genera código.
    generate_epilogue();
}

void GoCodeGen::generate_prologue(int stack_size) {
    output << ".data" << endl;
    output << "print_fmt: .string \"%ld\\n\"" << endl;
    output << "print_str_fmt: .string \"%s\\n\"" << endl;
    output << "print_bool_true: .string \"true\\n\"" << endl;
    output << "print_bool_false: .string \"false\\n\"" << endl;

    generate_string_literals();
    
    output << ".text" << endl;
    output << ".globl main" << endl;
    output << "main:" << endl;
    output << "  pushq %rbp" << endl;
    output << "  movq %rsp, %rbp" << endl;
    
    if (stack_size > 0) {
        if (stack_size % 16 != 0) {
            stack_size = ((stack_size + 15) / 16) * 16;
        }
        output << "  subq $" << stack_size << ", %rsp" << endl;
    }
}

void GoCodeGen::generate_epilogue() {
    output << "  movq $0, %rax" << endl;
    output << "  leave" << endl;
    output << "  ret" << endl;
    output << ".section .note.GNU-stack,\"\",@progbits" << endl;
}

void GoCodeGen::generate_string_literals() {
    for (const auto& kv : string_literals) {
        // Escapar caracteres especiales como \n si es necesario.
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
    if (auto s = dynamic_cast<VarDecl*>(stmt)) {
        size += s->names.size() * 8;
    } else if (auto s = dynamic_cast<ShortVarDecl*>(stmt)) {
        size += s->identifiers.size() * 8;
    } else if (auto s = dynamic_cast<IfStmt*>(stmt)) {
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
    for(auto s : block->statements) {
        size += calculate_stmt_size(s);
    }
    return size;
}

int GoCodeGen::calculate_stack_size(Program* p) {
    int total_size = 0;
    for (auto f : p->functions) {
        if (f->name == "main") {
            total_size += calculate_block_size(f->body);
        }
    }
    return total_size;
}

void GoCodeGen::visit(Program* program) {
    for (auto func : program->functions) {
        if (func->name == "main") {
            func->accept(this);
        }
    }
}

void GoCodeGen::visit(FuncDecl* decl) {
    if (decl->name == "main") {
        decl->body->accept(this);
    }
}

void GoCodeGen::visit(Block* block) {
    env.add_level();
    for (auto stmt : block->statements) {
        stmt->accept(this);
    }
    env.remove_level();
}

void GoCodeGen::visit(ExprStmt* stmt) {
    stmt->expression->accept(this);
}

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
        // Determinar el tipo de la expresion
        ImpValue val;
        if(valIt != stmt->values.end()) {
            val = (*valIt)->accept(this); // Esto ya pone el valor en %rax
        }
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
    string else_label = new_label();
    string end_label = new_label();
    
    stmt->condition->accept(this);
    output << "  cmpq $0, %rax" << endl;
    output << "  je " << else_label << endl;
    
    stmt->thenBlock->accept(this);
    output << "  jmp " << end_label << endl;
    
    output << else_label << ":" << endl;
    if (stmt->elseBlock) {
        stmt->elseBlock->accept(this);
    }
    
    output << end_label << ":" << endl;
}

void GoCodeGen::visit(ForStmt* stmt) {
    string start_label = new_label();
    string end_label = new_label();
    
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

void GoCodeGen::visit(ReturnStmt* stmt) {
    if (stmt->expression) {
        stmt->expression->accept(this);
    }
}

void GoCodeGen::visit(VarDecl* stmt) {
    auto nameIt = stmt->names.begin();
    auto valIt = stmt->values.begin();
    
    while (nameIt != stmt->names.end()) {
        current_offset -= 8;
        ImpValue val;
        if(valIt != stmt->values.end()) {
             val = (*valIt)->accept(this);
        }
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

// --- Visitantes de expresiones ---

ImpValue GoCodeGen::visit(BinaryExp* exp) {
    ImpValue left_val = exp->left->accept(this);
    output << "  pushq %rax" << endl;
    ImpValue right_val = exp->right->accept(this);
    output << "  movq %rax, %rbx" << endl;
    output << "  popq %rax" << endl;

    string set_instruction;
    switch (exp->op) {
        // --- Operaciones aritméticas: devuelven TINT ---
        case PLUS_OP: output << "  addq %rbx, %rax" << endl; return ImpValue(TINT);
        case MINUS_OP: output << "  subq %rbx, %rax" << endl; return ImpValue(TINT);
        case MUL_OP: output << "  imulq %rbx, %rax" << endl; return ImpValue(TINT);
        case DIV_OP: output << "  cqto" << endl; output << "  idivq %rbx" << endl; return ImpValue(TINT);
        case MOD_OP: output << "  cqto" << endl; output << "  idivq %rbx" << endl; output << "  movq %rdx, %rax" << endl; return ImpValue(TINT);
        
        // --- Operaciones de comparación: devuelven TBOOL ---
        case LT_OP: set_instruction = "setl"; goto compare;
        case LE_OP: set_instruction = "setle"; goto compare;
        case GT_OP: set_instruction = "setg"; goto compare;
        case GE_OP: set_instruction = "setge"; goto compare;
        case EQ_OP: set_instruction = "sete"; goto compare;
        case NE_OP: set_instruction = "setne"; goto compare;

        // --- Operaciones lógicas: devuelven TBOOL ---
        case AND_OP: {
            string false_label = new_label();
            string end_label = new_label();
            output << "  cmpq $0, %rax" << endl;
            output << "  je " << false_label << endl;
            output << "  cmpq $0, %rbx" << endl;
            output << "  je " << false_label << endl;
            output << "  movq $1, %rax" << endl;
            output << "  jmp " << end_label << endl;
            output << false_label << ":" << endl;
            output << "  movq $0, %rax" << endl;
            output << end_label << ":" << endl;
            break;
        }
        case OR_OP: {
            string true_label = new_label();
            string end_label = new_label();
            output << "  cmpq $0, %rax" << endl;
            output << "  jne " << true_label << endl;
            output << "  cmpq $0, %rbx" << endl;
            output << "  je " << end_label << endl; // Si el segundo es 0, el resultado es 0
            output << true_label << ":" << endl;
            output << "  movq $1, %rax" << endl;
            output << end_label << ":" << endl; // Etiqueta para cuando el segundo es 0
            break;
        }
    }
    return ImpValue(true); // Devuelve TBOOL por defecto para AND/OR

compare:
    output << "  cmpq %rbx, %rax" << endl;
    output << "  " << set_instruction << " %al" << endl;
    output << "  movzbq %al, %rax" << endl;
    return ImpValue(true); // Devuelve TBOOL
}

ImpValue GoCodeGen::visit(UnaryExp* exp) {
    exp->exp->accept(this);
    switch (exp->op) {
        case UMINUS_OP: output << "  negq %rax" << endl; break;
        case NOT_OP:
            output << "  cmpq $0, %rax" << endl;
            output << "  sete %al" << endl;
            output << "  movzbq %al, %rax" << endl;
            break;
        case UPLUS_OP: break;
    }
    return ImpValue();
}

ImpValue GoCodeGen::visit(NumberExp* exp) {
    output << "  movq $" << exp->value << ", %rax" << endl;
    return ImpValue(exp->value);
}

ImpValue GoCodeGen::visit(StringExp* exp) {
    string label = string_literals[exp->value];
    output << "  leaq " << label << "(%rip), %rax" << endl;
    return ImpValue(exp->value);
}

ImpValue GoCodeGen::visit(BoolExp* exp) {
    output << "  movq $" << (exp->value ? 1 : 0) << ", %rax" << endl;
    return ImpValue(exp->value);
}

ImpValue GoCodeGen::visit(IdentifierExp* exp) {
    if (env.check(exp->name)) {
        VarInfo info = env.lookup(exp->name);
        output << "  movq " << info.offset << "(%rbp), %rax  # Cargar " << exp->name << endl;
        ImpValue val;
        val.type = info.type;
        return val;
    } else {
        output << "  # Advertencia: Variable no definida: " << exp->name << endl;
        output << "  movq $0, %rax" << endl;
    }
    return ImpValue();
}

ImpValue GoCodeGen::visit(FunctionCallExp* exp) {
    if (exp->funcName == "fmt.Println") {
        for (auto arg : exp->args) {
            if (!arg) continue;

            ImpValue val = arg->accept(this);
            
            string fmt_reg = "%rdi";
            if (val.type == TINT) {
                output << "  leaq print_fmt(%rip), " << fmt_reg << endl;
                output << "  movq %rax, %rsi" << endl;
            } else if (val.type == TSTRING) {
                output << "  leaq print_str_fmt(%rip), " << fmt_reg << endl;
                output << "  movq %rax, %rsi" << endl;
            } else if (val.type == TBOOL) {
                // Debemos usar el valor en %rax que es en tiempo de ejecución.
                string true_label = new_label();
                string end_label = new_label();
                output << "  cmpq $0, %rax" << endl;
                output << "  jne " << true_label << endl;
                // Si es false (0)
                output << "  leaq print_bool_false(%rip), %rdi" << endl;
                output << "  jmp " << end_label << endl;
                // Si es true (1)
                output << true_label << ":" << endl;
                output << "  leaq print_bool_true(%rip), %rdi" << endl;
                output << end_label << ":" << endl;
                // No necesitamos %rsi para imprimir "true" o "false"
            } else {
                 output << "  leaq print_fmt(%rip), " << fmt_reg << endl;
                 output << "  movq %rax, %rsi" << endl;
            }

            output << "  xorq %rax, %rax" << endl;
            output << "  call printf@PLT" << endl;
        }
    }
    return ImpValue();
}

// Implementaciones vacías para lo no soportado
ImpValue GoCodeGen::visit(FieldAccessExp* exp) { return ImpValue(); }
ImpValue GoCodeGen::visit(IndexExp* exp) { return ImpValue(); }
ImpValue GoCodeGen::visit(SliceExp* exp) { return ImpValue(); }
ImpValue GoCodeGen::visit(StructLiteralExp* exp) { return ImpValue(); }