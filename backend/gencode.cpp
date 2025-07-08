#include "gencode.h"
#include <iostream>

using namespace std;

GoCodeGen::GoCodeGen() : current_offset(0), label_counter(0), string_counter(0) {}

string GoCodeGen::new_label() {
    return "L" + to_string(label_counter++);
}

void GoCodeGen::generate_prologue() {
    cout << ".data" << endl;
    cout << "print_fmt: .string \"%ld\\n\"" << endl;
    cout << ".text" << endl;
    cout << ".globl main" << endl;
    cout << "main:" << endl;
    cout << "  pushq %rbp" << endl;
    cout << "  movq %rsp, %rbp" << endl;
}

void GoCodeGen::generate_epilogue() {
    cout << "  movq $0, %rax" << endl;
    cout << "  leave" << endl;
    cout << "  ret" << endl;
    cout << ".section .note.GNU-stack,\"\",@progbits" << endl;
}

void GoCodeGen::generateCode(Program* program) {
    env.clear();
    current_offset = 0;
    label_counter = 0;
    string_counter = 0;
    string_literals.clear();
    stack_offsets.clear();
    
    generate_prologue();
    program->accept(this);
    generate_epilogue();
}

void GoCodeGen::visit(Program* program) {
    env.add_level();
    
    for (auto func : program->functions) {
        func->accept(this);
    }
    
    env.remove_level();
}

void GoCodeGen::visit(FuncDecl* decl) {
    if (decl->name == "main") {
        env.add_level();
        long old_offset = current_offset;
        
        decl->body->accept(this);
        
        current_offset = old_offset;
        env.remove_level();
    }
}

void GoCodeGen::visit(Block* block) {
    env.add_level();
    long old_offset = current_offset;
    
    // Process statements
    for (auto stmt : block->statements) {
        stmt->accept(this);
    }
    
    current_offset = old_offset;
    env.remove_level();
}

void GoCodeGen::visit(ExprStmt* stmt) {
    stmt->expression->accept(this);
}

void GoCodeGen::visit(ShortVarDecl* stmt) {
    // Allocate space for variables
    for (const string& var : stmt->identifiers) {
        current_offset -= 8; // 8 bytes for each variable (assuming 64-bit)
        stack_offsets[var] = current_offset;
        env.add_var(var, ImpValue());
    }
    
    // Evaluate expressions and assign
    auto varIt = stmt->identifiers.begin();
    for (auto expr : stmt->values) {
        if (varIt != stmt->identifiers.end()) {
            expr->accept(this);
            cout << "  movq %rax, " << stack_offsets[*varIt] << "(%rbp)" << endl;
            ++varIt;
        }
    }
}

void GoCodeGen::visit(AssignStmt* stmt) {
    stmt->rhs->accept(this);
    
    // Assuming lhs is an identifier
    IdentifierExp* id = dynamic_cast<IdentifierExp*>(stmt->lhs);
    if (id) {
        cout << "  movq %rax, " << stack_offsets[id->name] << "(%rbp)" << endl;
    }
}

void GoCodeGen::visit(IncDecStmt* stmt) {
    if (stack_offsets.find(stmt->variable) != stack_offsets.end()) {
        cout << "  movq " << stack_offsets[stmt->variable] << "(%rbp), %rax" << endl;
        if (stmt->isIncrement) {
            cout << "  incq %rax" << endl;
        } else {
            cout << "  decq %rax" << endl;
        }
        cout << "  movq %rax, " << stack_offsets[stmt->variable] << "(%rbp)" << endl;
    }
}

void GoCodeGen::visit(IfStmt* stmt) {
    string else_label = new_label();
    string end_label = new_label();
    
    // Evaluar condicioon
    stmt->condition->accept(this);
    cout << "  cmpq $0, %rax" << endl;
    cout << "  je " << else_label << endl;
    
    stmt->thenBlock->accept(this);
    cout << "  jmp " << end_label << endl;
    
    cout << else_label << ":" << endl;
    if (stmt->elseBlock) {
        stmt->elseBlock->accept(this);
    }
    
    cout << end_label << ":" << endl;
}

void GoCodeGen::visit(ForStmt* stmt) {
    string loop_start = new_label();
    string loop_end = new_label();
    
    // Inicio de stm
    if (stmt->init) {
        stmt->init->accept(this);
    }
    
    cout << loop_start << ":" << endl;
    
    // Condicion
    if (stmt->condition) {
        stmt->condition->accept(this);
        cout << "  cmpq $0, %rax" << endl;
        cout << "  je " << loop_end << endl;
    }
    
    // Curpo
    stmt->body->accept(this);
    
    // Post
    if (stmt->post) {
        stmt->post->accept(this);
    }
    
    cout << "  jmp " << loop_start << endl;
    cout << loop_end << ":" << endl;
}

void GoCodeGen::visit(ReturnStmt* stmt) {
    if (stmt->expression) {
        stmt->expression->accept(this);
    }
}

ImpValue GoCodeGen::visit(BinaryExp* exp) {
    exp->left->accept(this);
    cout << "  pushq %rax" << endl; // Guardamos rax en el stack
    
    exp->right->accept(this);
    cout << "  movq %rax, %rbx" << endl; // Movemos el derecho a rbx
    cout << "  popq %rax" << endl; // Restauramos rax
    
    switch (exp->op) {
        case PLUS_OP:
            cout << "  addq %rbx, %rax" << endl;
            break;
        case MINUS_OP:
            cout << "  subq %rbx, %rax" << endl;
            break;
        case MUL_OP:
            cout << "  imulq %rbx, %rax" << endl;
            break;
        case DIV_OP:
            cout << "  cqto" << endl;
            cout << "  idivq %rbx" << endl;
            break;
        case LT_OP:
            cout << "  cmpq %rbx, %rax" << endl;
            cout << "  setl %al" << endl;
            cout << "  movzbq %al, %rax" << endl;
            break;
        case LE_OP:
            cout << "  cmpq %rbx, %rax" << endl;
            cout << "  setle %al" << endl;
            cout << "  movzbq %al, %rax" << endl;
            break;
        case GT_OP:
            cout << "  cmpq %rbx, %rax" << endl;
            cout << "  setg %al" << endl;
            cout << "  movzbq %al, %rax" << endl;
            break;
        case GE_OP:
            cout << "  cmpq %rbx, %rax" << endl;
            cout << "  setge %al" << endl;
            cout << "  movzbq %al, %rax" << endl;
            break;
        case EQ_OP:
            cout << "  cmpq %rbx, %rax" << endl;
            cout << "  sete %al" << endl;
            cout << "  movzbq %al, %rax" << endl;
            break;
        case NE_OP:
            cout << "  cmpq %rbx, %rax" << endl;
            cout << "  setne %al" << endl;
            cout << "  movzbq %al, %rax" << endl;
            break;
        case AND_OP:
            cout << "  andq %rbx, %rax" << endl;
            break;
        case OR_OP:
            cout << "  orq %rbx, %rax" << endl;
            break;
        default:
            cout << "  # Unsupported binary operator" << endl;
            break;
    }
    
    return ImpValue();
}

ImpValue GoCodeGen::visit(UnaryExp* exp) {
    exp->exp->accept(this);
    
    switch (exp->op) {
        case UPLUS_OP:
            break;
        case UMINUS_OP:
            cout << "  negq %rax" << endl;
            break;
        case NOT_OP:
            cout << "  notq %rax" << endl;
            break;
        default:
            cout << "  # Unsupported unary operator" << endl;
            break;
    }
    
    return ImpValue();
}

ImpValue GoCodeGen::visit(NumberExp* exp) {
    cout << "  movq $" << exp->value << ", %rax" << endl;
    return ImpValue(exp->value);
}

ImpValue GoCodeGen::visit(StringExp* exp) {
    cout << "  movq $0, %rax  # String: " << exp->value << endl;
    return ImpValue(exp->value);
}

ImpValue GoCodeGen::visit(BoolExp* exp) {
    cout << "  movq $" << (exp->value ? 1 : 0) << ", %rax" << endl;
    return ImpValue(exp->value);
}

ImpValue GoCodeGen::visit(IdentifierExp* exp) {
    if (stack_offsets.find(exp->name) != stack_offsets.end()) {
        cout << "  movq " << stack_offsets[exp->name] << "(%rbp), %rax" << endl;
    } else {
        cout << "  # Undefined variable: " << exp->name << endl;
        cout << "  movq $0, %rax" << endl;
    }
    return ImpValue();
}

ImpValue GoCodeGen::visit(FunctionCallExp* exp) {
    if (exp->funcName == "fmt.Println") {
        if (!exp->args.empty()) {
            auto arg = exp->args.front();
            arg->accept(this);
            
            // For simplicity, assume integer values for now
            cout << "  leaq print_fmt(%rip), %rdi" << endl;
            cout << "  movq %rax, %rsi" << endl;
            cout << "  xorq %rax, %rax" << endl; // Limpiamos RAX por funcion variadica
            cout << "  call printf" << endl;
        }
    }
    
    return ImpValue();
}

ImpValue GoCodeGen::visit(FieldAccessExp* exp) { return ImpValue(); }
ImpValue GoCodeGen::visit(IndexExp* exp) { return ImpValue(); }
ImpValue GoCodeGen::visit(SliceExp* exp) { return ImpValue(); }
ImpValue GoCodeGen::visit(StructLiteralExp* exp) { return ImpValue(); }

void GoCodeGen::visit(VarDecl* stmt) {
    // Declaraciones de variables
    auto nameIt = stmt->names.begin();
    auto valIt = stmt->values.begin();
    
    for (const string& name : stmt->names) {
        // Guardamos espacio en el stack
        current_offset -= 8; // 8 bytes por cada variable
        stack_offsets[name] = current_offset;
        env.add_var(name, ImpValue());
        
        // Inicializamos con valor
        if (valIt != stmt->values.end()) {
            (*valIt)->accept(this);
            cout << "  movq %rax, " << stack_offsets[name] << "(%rbp)" << endl;
            ++valIt;
        } else {
            // Inicializamos sin valor si es que no se da
            cout << "  movq $0, %rax" << endl;
            cout << "  movq %rax, " << stack_offsets[name] << "(%rbp)" << endl;
        }
    }
}

void GoCodeGen::visit(TypeDecl* decl) {
    // Type declarations (not implemented)
}

void GoCodeGen::visit(ImportDecl* decl) {
    // Import declarations (not implemented)
}
