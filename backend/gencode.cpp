#include "gencode.h"
#include <iostream>
#include <algorithm>

using namespace std;

// =================== Implementación de OffsetCalculator ===================
OffsetCalculator::OffsetCalculator() : current_offset(0) {}

std::unordered_map<std::string, int> OffsetCalculator::calculateOffsets(Program* program) {
    env.clear();
    current_offset = 0;
    stack_offsets.clear();
    
    // Calcular offsets para variables recorriendo el AST
    program->accept(this);
    
    return stack_offsets;
}

// Recorrer el árbol para expresiones
ImpValue OffsetCalculator::visit(BinaryExp* exp) {
    exp->left->accept(this);
    exp->right->accept(this);
    return ImpValue();
}

ImpValue OffsetCalculator::visit(UnaryExp* exp) {
    exp->exp->accept(this);
    return ImpValue();
}

// Implementaciones vacías para literales y expresiones simples
ImpValue OffsetCalculator::visit(NumberExp* exp) { return ImpValue(); }
ImpValue OffsetCalculator::visit(StringExp* exp) { return ImpValue(); }
ImpValue OffsetCalculator::visit(BoolExp* exp) { return ImpValue(); }
ImpValue OffsetCalculator::visit(IdentifierExp* exp) { return ImpValue(); }
ImpValue OffsetCalculator::visit(FieldAccessExp* exp) { 
    exp->object->accept(this);
    return ImpValue(); 
}
ImpValue OffsetCalculator::visit(IndexExp* exp) { 
    exp->array->accept(this);
    exp->index->accept(this);
    return ImpValue();
}
ImpValue OffsetCalculator::visit(SliceExp* exp) {
    exp->array->accept(this);
    if (exp->start) exp->start->accept(this);
    if (exp->end) exp->end->accept(this);
    return ImpValue();
}
ImpValue OffsetCalculator::visit(StructLiteralExp* exp) {
    for (auto val : exp->values) {
        val->accept(this);
    }
    return ImpValue();
}

ImpValue OffsetCalculator::visit(FunctionCallExp* exp) {
    for (auto arg : exp->args) {
        arg->accept(this);
    }
    return ImpValue();
}

// Visitantes de sentencias para asignar variables
void OffsetCalculator::visit(ExprStmt* stmt) {
    stmt->expression->accept(this);
}

void OffsetCalculator::visit(AssignStmt* stmt) {
    stmt->lhs->accept(this);
    stmt->rhs->accept(this);
}

void OffsetCalculator::visit(ShortVarDecl* stmt) {
    // Para declaraciones cortas de variables, asignar espacio en pila para cada variable
    for (const string& var : stmt->identifiers) {
        current_offset -= 8; // Asignar 8 bytes para cada variable
        stack_offsets[var] = current_offset;
        env.add_var(var, ImpValue());
    }
    
    // Recorrer expresiones para posibles declaraciones de variables en ámbitos anidados
    for (auto expr : stmt->values) {
        expr->accept(this);
    }
}

void OffsetCalculator::visit(IncDecStmt* stmt) {
    // Nada que asignar aquí
}

void OffsetCalculator::visit(IfStmt* stmt) {
    stmt->condition->accept(this);
    stmt->thenBlock->accept(this);
    if (stmt->elseBlock) stmt->elseBlock->accept(this);
}

void OffsetCalculator::visit(ForStmt* stmt) {
    if (stmt->init) stmt->init->accept(this);
    if (stmt->condition) stmt->condition->accept(this);
    if (stmt->post) stmt->post->accept(this);
    stmt->body->accept(this);
}

void OffsetCalculator::visit(ReturnStmt* stmt) {
    if (stmt->expression) stmt->expression->accept(this);
}

void OffsetCalculator::visit(VarDecl* stmt) {
    // Asignar espacio en pila para cada variable
    for (const string& name : stmt->names) {
        current_offset -= 8; // Asignar 8 bytes para cada variable
        stack_offsets[name] = current_offset;
        env.add_var(name, ImpValue());
    }
    
    // Recorrer expresiones para posibles declaraciones de variables en ámbitos anidados
    for (auto expr : stmt->values) {
        expr->accept(this);
    }
}

void OffsetCalculator::visit(TypeDecl* decl) {
    // Las declaraciones de tipo struct no asignan variables de pila
}

void OffsetCalculator::visit(FuncDecl* decl) {
    if (decl->name == "main") {
        env.add_level();
        decl->body->accept(this);
        env.remove_level();
    }
}

void OffsetCalculator::visit(Block* block) {
    env.add_level();
    
    for (auto stmt : block->statements) {
        stmt->accept(this);
    }
    
    env.remove_level();
}

void OffsetCalculator::visit(ImportDecl* decl) {
    // Las declaraciones de importación no asignan variables de pila
}

void OffsetCalculator::visit(Program* program) {
    env.add_level();
    
    // Procesar funciones (especialmente main)
    for (auto func : program->functions) {
        func->accept(this);
    }
    
    env.remove_level();
}

// =================== Implementación de GoCodeGen ===================
GoCodeGen::GoCodeGen(std::ostream& out) 
    : current_offset(0), label_counter(0), string_counter(0), output(out) {}

string GoCodeGen::new_label() {
    return "L" + to_string(label_counter++);
}

void GoCodeGen::generateCode(Program* program) {
    // Primera pasada: Calcular offsets de pila
    OffsetCalculator calculator;
    stack_offsets = calculator.calculateOffsets(program);
    
    // Obtener el offset directo del calculador para asegurar consistencia
    current_offset = calculator.getCurrentOffset();
    
    // Segunda pasada: Generar código
    generate_prologue();
    program->accept(this);
    generate_epilogue();
}

void GoCodeGen::generate_prologue() {
    output << ".data" << endl;
    output << "print_fmt: .string \"%ld\\n\"" << endl;
    
    // Agregar literales de cadena si existen
    generate_string_literals();
    
    output << ".text" << endl;
    output << ".globl main" << endl;
    output << "main:" << endl;
    output << "  pushq %rbp" << endl;
    output << "  movq %rsp, %rbp" << endl;
    
    // Calcular tamaño de pila (será positivo porque current_offset es negativo)
    int stack_size = -current_offset;
    
    // Asegurar alineación de 16 bytes (requerido por ABI)
    if (stack_size % 16 != 0) {
        stack_size = ((stack_size + 15) / 16) * 16;
    }
    
    // Emitir siempre el ajuste de pila cuando tenemos variables locales
    if (stack_size > 0) {
        output << "  subq $" << stack_size << ", %rsp  # Reservar espacio para " << (-current_offset/8) << " variables" << endl;
    }
}

void GoCodeGen::generate_epilogue() {
    output << "  movq $0, %rax" << endl;
    output << "  leave" << endl;
    output << "  ret" << endl;
    output << ".section .note.GNU-stack,\"\",@progbits" << endl;
}

void GoCodeGen::generate_string_literals() {
    // Generar los literales de cadena utilizados en el programa
    for (const auto& kv : string_literals) {
        output << kv.second << ": .string \"" << kv.first << "\"" << endl;
    }
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
        decl->body->accept(this);
        env.remove_level();
    }
    // Nota: Para un compilador completo, también generaríamos código para otras funciones
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
    // Evaluar primero el lado derecho de la asignación
    stmt->rhs->accept(this);
    
    // Luego almacenar el resultado en la ubicación del lado izquierdo
    if (auto id = dynamic_cast<IdentifierExp*>(stmt->lhs)) {
        if (stack_offsets.find(id->name) != stack_offsets.end()) {
            int offset = stack_offsets[id->name];
            output << "  movq %rax, " << offset << "(%rbp) # Almacenar en " << id->name << endl;
        } else {
            output << "  # Advertencia: Variable no definida " << id->name << endl;
        }
    } else if (auto field = dynamic_cast<FieldAccessExp*>(stmt->lhs)) {
        // Manejar acceso a campos de struct
        output << "  # Almacenar en campo de struct no implementado completamente" << endl;
    } else if (auto index = dynamic_cast<IndexExp*>(stmt->lhs)) {
        // Manejar indexación de arreglos
        output << "  # Almacenar en índice de arreglo no implementado completamente" << endl;
    }
}

void GoCodeGen::visit(ShortVarDecl* stmt) {
    auto varIt = stmt->identifiers.begin();
    auto valIt = stmt->values.begin();
    
    while (varIt != stmt->identifiers.end() && valIt != stmt->values.end()) {
        // Evaluar la expresión de inicialización
        (*valIt)->accept(this);
        
        // Almacenar el resultado en la ubicación de la variable en la pila
        int offset = stack_offsets[*varIt];
        output << "  movq %rax, " << offset << "(%rbp) # Inicializar " << *varIt << endl;
        
        ++varIt;
        ++valIt;
    }
    
    // Inicializar por defecto cualquier variable restante
    while (varIt != stmt->identifiers.end()) {
        int offset = stack_offsets[*varIt];
        output << "  movq $0, " << offset << "(%rbp) # Inicialización por defecto " << *varIt << endl;
        ++varIt;
    }
}

void GoCodeGen::visit(IncDecStmt* stmt) {
    if (stack_offsets.find(stmt->variable) != stack_offsets.end()) {
        int offset = stack_offsets[stmt->variable];
        
        // Cargar el valor actual
        output << "  movq " << offset << "(%rbp), %rax" << endl;
        
        // Incrementar o decrementar
        if (stmt->isIncrement) {
            output << "  incq %rax" << endl;
        } else {
            output << "  decq %rax" << endl;
        }
        
        // Almacenar el valor actualizado
        output << "  movq %rax, " << offset << "(%rbp)" << endl;
    }
}

void GoCodeGen::visit(IfStmt* stmt) {
    string else_label = new_label();
    string end_label = new_label();
    
    // Evaluar condición
    stmt->condition->accept(this);
    output << "  cmpq $0, %rax" << endl;
    output << "  je " << else_label << endl;
    
    // Generar código para el bloque then
    stmt->thenBlock->accept(this);
    output << "  jmp " << end_label << endl;
    
    // Generar código para el bloque else (si está presente)
    output << else_label << ":" << endl;
    if (stmt->elseBlock) {
        stmt->elseBlock->accept(this);
    }
    
    output << end_label << ":" << endl;
}

void GoCodeGen::visit(ForStmt* stmt) {
    string start_label = new_label();
    string end_label = new_label();
    
    // Inicializar (si está presente)
    if (stmt->init) {
        stmt->init->accept(this);
    }
    
    // Condición del bucle
    output << start_label << ":" << endl;
    if (stmt->condition) {
        stmt->condition->accept(this);
        output << "  cmpq $0, %rax" << endl;
        output << "  je " << end_label << endl;
    }
    
    // Cuerpo del bucle
    stmt->body->accept(this);
    
    // Sentencia de post-iteración (si está presente)
    if (stmt->post) {
        stmt->post->accept(this);
    }
    
    // Volver al inicio
    output << "  jmp " << start_label << endl;
    output << end_label << ":" << endl;
}

void GoCodeGen::visit(ReturnStmt* stmt) {
    if (stmt->expression) {
        stmt->expression->accept(this);
    }
}

void GoCodeGen::visit(VarDecl* stmt) {
    auto nameIt = stmt->names.begin();
    auto valIt = stmt->values.begin();
    
    while (nameIt != stmt->names.end() && valIt != stmt->values.end()) {
        // Evaluar la expresión de inicialización
        (*valIt)->accept(this);
        
        // Almacenar el resultado en la ubicación de la variable en la pila
        int offset = stack_offsets[*nameIt];
        output << "  movq %rax, " << offset << "(%rbp) # Inicializar " << *nameIt << endl;
        
        ++nameIt;
        ++valIt;
    }
    
    // Inicializar por defecto cualquier variable restante
    while (nameIt != stmt->names.end()) {
        int offset = stack_offsets[*nameIt];
        output << "  movq $0, " << offset << "(%rbp) # Inicialización por defecto " << *nameIt << endl;
        ++nameIt;
    }
}

void GoCodeGen::visit(TypeDecl* decl) {
    // No generamos código para declaraciones de tipo en esta versión simplificada
}

void GoCodeGen::visit(ImportDecl* decl) {
    // No generamos código para declaraciones de importación en esta versión simplificada
}

// Implementación de visitantes de expresiones
ImpValue GoCodeGen::visit(BinaryExp* exp) {
    // Manejar diferentes operadores binarios
    switch (exp->op) {
        case PLUS_OP:
        case MINUS_OP:
        case MUL_OP:
        case DIV_OP:
        case MOD_OP:
            // Operaciones aritméticas
            exp->left->accept(this);
            output << "  pushq %rax" << endl;
            exp->right->accept(this);
            output << "  movq %rax, %rbx" << endl;
            output << "  popq %rax" << endl;
            
            switch (exp->op) {
                case PLUS_OP:
                    output << "  addq %rbx, %rax" << endl;
                    break;
                case MINUS_OP:
                    output << "  subq %rbx, %rax" << endl;
                    break;
                case MUL_OP:
                    output << "  imulq %rbx, %rax" << endl;
                    break;
                case DIV_OP:
                    output << "  cqto" << endl;  // Extender signo de RAX a RDX:RAX
                    output << "  idivq %rbx" << endl;
                    break;
                case MOD_OP:
                    output << "  cqto" << endl;
                    output << "  idivq %rbx" << endl;
                    output << "  movq %rdx, %rax" << endl;  // El residuo está en RDX
                    break;
                default:
                    break;
            }
            break;
            
        case LT_OP:
        case LE_OP:
        case GT_OP:
        case GE_OP:
        case EQ_OP:
        case NE_OP:
            // Operaciones de comparación
            exp->left->accept(this);
            output << "  pushq %rax" << endl;
            exp->right->accept(this);
            output << "  movq %rax, %rbx" << endl;
            output << "  popq %rax" << endl;
            output << "  cmpq %rbx, %rax" << endl;
            
            switch (exp->op) {
                case LT_OP:
                    output << "  setl %al" << endl;
                    break;
                case LE_OP:
                    output << "  setle %al" << endl;
                    break;
                case GT_OP:
                    output << "  setg %al" << endl;
                    break;
                case GE_OP:
                    output << "  setge %al" << endl;
                    break;
                case EQ_OP:
                    output << "  sete %al" << endl;
                    break;
                case NE_OP:
                    output << "  setne %al" << endl;
                    break;
                default:
                    break;
            }
            output << "  movzbq %al, %rax" << endl;
            break;
            
        case AND_OP:
        case OR_OP:
            // Operaciones lógicas (evaluación de cortocircuito)
            if (exp->op == AND_OP) {
                string false_label = new_label();
                string end_label = new_label();
                
                exp->left->accept(this);
                output << "  cmpq $0, %rax" << endl;
                output << "  je " << false_label << endl;
                
                exp->right->accept(this);
                output << "  cmpq $0, %rax" << endl;
                output << "  je " << false_label << endl;
                
                output << "  movq $1, %rax" << endl;
                output << "  jmp " << end_label << endl;
                
                output << false_label << ":" << endl;
                output << "  movq $0, %rax" << endl;
                
                output << end_label << ":" << endl;
            } else {  // OR_OP
                string true_label = new_label();
                string end_label = new_label();
                
                exp->left->accept(this);
                output << "  cmpq $0, %rax" << endl;
                output << "  jne " << true_label << endl;
                
                exp->right->accept(this);
                output << "  cmpq $0, %rax" << endl;
                output << "  je " << end_label << endl;
                
                output << true_label << ":" << endl;
                output << "  movq $1, %rax" << endl;
                
                output << end_label << ":" << endl;
            }
            break;
            
        default:
            output << "  # Operador binario no soportado" << endl;
            break;
    }
    
    return ImpValue();
}

ImpValue GoCodeGen::visit(UnaryExp* exp) {
    exp->exp->accept(this);
    
    switch (exp->op) {
        case UPLUS_OP:
            // El más unario no hace nada
            break;
        case UMINUS_OP:
            output << "  negq %rax" << endl;
            break;
        case NOT_OP:
            output << "  cmpq $0, %rax" << endl;
            output << "  sete %al" << endl;
            output << "  movzbq %al, %rax" << endl;
            break;
        default:
            output << "  # Operador unario no soportado" << endl;
            break;
    }
    
    return ImpValue();
}

ImpValue GoCodeGen::visit(NumberExp* exp) {
    output << "  movq $" << exp->value << ", %rax" << endl;
    return ImpValue(exp->value);
}

ImpValue GoCodeGen::visit(StringExp* exp) {
    // Crear una etiqueta para esta cadena si no existe
    if (string_literals.find(exp->value) == string_literals.end()) {
        string label = ".LC" + to_string(string_counter++);
        string_literals[exp->value] = label;
    }
    
    string label = string_literals[exp->value];
    output << "  movq $0, %rax  # Cadena: " << exp->value << endl;
    return ImpValue(exp->value);
}

ImpValue GoCodeGen::visit(BoolExp* exp) {
    output << "  movq $" << (exp->value ? 1 : 0) << ", %rax" << endl;
    return ImpValue(exp->value);
}

ImpValue GoCodeGen::visit(IdentifierExp* exp) {
    if (stack_offsets.find(exp->name) != stack_offsets.end()) {
        int offset = stack_offsets[exp->name];
        output << "  movq " << offset << "(%rbp), %rax  # Cargar " << exp->name << endl;
    } else {
        output << "  # Advertencia: Variable no definida: " << exp->name << endl;
        output << "  movq $0, %rax" << endl;
    }
    
    return ImpValue();
}

ImpValue GoCodeGen::visit(FieldAccessExp* exp) {
    output << "  # FieldAccessExp no implementado completamente" << endl;
    return ImpValue();
}

ImpValue GoCodeGen::visit(IndexExp* exp) {
    output << "  # IndexExp no implementado completamente" << endl;
    return ImpValue();
}

ImpValue GoCodeGen::visit(SliceExp* exp) {
    output << "  # SliceExp no implementado completamente" << endl;
    return ImpValue();
}

ImpValue GoCodeGen::visit(FunctionCallExp* exp) {
    // Manejo especial para fmt.Println
    if (exp->funcName == "fmt.Println") {
        if (!exp->args.empty()) {
            auto arg = exp->args.front();
            arg->accept(this);
            
            output << "  leaq print_fmt(%rip), %rdi" << endl;
            output << "  movq %rax, %rsi" << endl;
            output << "  xorq %rax, %rax" << endl;  // Limpiar RAX para función variadica
            output << "  call printf" << endl;
        }
    }
    
    return ImpValue();
}

ImpValue GoCodeGen::visit(StructLiteralExp* exp) {
    output << "  # StructLiteralExp no implementado completamente" << endl;
    return ImpValue();
}
