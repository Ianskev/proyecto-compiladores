#include "gencode.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include "helpers_gencode.cpp" 

using namespace std;

GoCodeGen::GoCodeGen(std::ostream& out) 
    : current_offset(0), label_counter(0), string_counter(0), output(out) {
    this->needs_string_concat = false;
    this->needs_string_compare = false;
}

string GoCodeGen::new_label() { return "L" + to_string(label_counter++); }

void GoCodeGen::generateCode(Program* program) {
    try {
        env.clear();
        env.add_level();
        
        StructCollectorVisitor struct_collector(env);
        program->accept(&struct_collector);

        StringCollectorVisitor string_collector(this->string_literals, this->string_counter);
        program->accept(&string_collector);
        
        for (auto f : program->functions) {
            ImpVType retType = NOTYPE;
            if (f->returnType) {
                if (auto bt = dynamic_cast<BasicType*>(f->returnType)) {
                    retType = ImpValue::get_basic_type(bt->typeName);
                }
            }
            env.add_function(f->name, {calculate_block_size(f->body), retType});
        }
        
        current_offset = 0;
        label_counter = 0;

        generate_prologue();
        program->accept(this); 

        generate_runtime_helpers(); 
        generate_epilogue();

    } catch (const std::runtime_error& e) {
        cerr << "Error de generación de código: " << e.what() << endl;
    }
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
        while ((pos = val.find('\\', pos)) != std::string::npos) {
            val.replace(pos, 1, "\\\\");
            pos += 2;
        }
        output << kv.second << ": .string \"" << val << "\"" << endl;
    }
}

void GoCodeGen::generate_runtime_helpers() {
    if (!needs_string_concat && !needs_string_compare) {
        return; 
    }

    output << endl << "# --- Funciones de Ayuda para Runtime (Generadas Condicionalmente) ---" << endl;

    if (this->needs_string_concat) {
        output << "_concat_strings:" << endl;
        output << "  pushq %rbp" << endl;
        output << "  movq %rsp, %rbp" << endl;
        output << "  subq $16, %rsp # Espacio para guardar args" << endl;
        output << "  movq %rdi, -8(%rbp)" << endl;
        output << "  movq %rsi, -16(%rbp)" << endl;
        output << "  call strlen@PLT" << endl;
        output << "  pushq %rax" << endl;
        output << "  movq -16(%rbp), %rdi" << endl;
        output << "  call strlen@PLT" << endl;
        output << "  popq %rcx" << endl;
        output << "  addq %rcx, %rax" << endl;
        output << "  incq %rax" << endl;
        output << "  movq %rax, %rdi" << endl;
        output << "  call malloc@PLT" << endl;
        output << "  movq -8(%rbp), %rsi" << endl;
        output << "  movq %rax, %rdi" << endl;
        output << "  call strcpy@PLT" << endl;
        output << "  movq -16(%rbp), %rsi" << endl;
        output << "  movq %rax, %rdi" << endl;
        output << "  call strcat@PLT" << endl;
        output << "  leave" << endl;
        output << "  ret" << endl;
    }

    if (this->needs_string_compare) {
        output << "_compare_strings:" << endl;
        output << "  call strcmp@PLT" << endl;
        output << "  ret" << endl;
    }
    output << "# --- Fin de Funciones de Ayuda ---" << endl;
}

int GoCodeGen::calculate_stmt_size(Stmt* stmt) {
    int size = 0;
    if (auto s = dynamic_cast<VarDecl*>(stmt)) {
        if (auto id_type = dynamic_cast<IdentifierType*>(s->type)) {
            if (env.has_struct(id_type->name)) {
                size += env.get_struct(id_type->name).size * s->names.size();
            }
        } else {
             size += s->names.size() * 8;
        }
    } else if (auto s = dynamic_cast<ShortVarDecl*>(stmt)) {
        // El código anterior era: size += s->identifiers.size() * 8;
        // lo que asumía incorrectamente que toda variable declarada con := ocupa 8 bytes.
        // El nuevo código inspecciona el tipo de la expresión para determinar el tamaño correcto.
        auto value_it = s->values.begin();
        for (const auto& id_name : s->identifiers) {
            if (value_it != s->values.end()) {
                Exp* value_exp = *value_it;
                if (auto struct_lit = dynamic_cast<StructLiteralExp*>(value_exp)) {
                    // Es un struct. Obtenemos su tamaño del entorno.
                    if (env.has_struct(struct_lit->typeName)) {
                        size += env.get_struct(struct_lit->typeName).size;
                    } else {
                        // Fallback por si el struct no se encuentra (no debería pasar)
                        size += 8;
                    }
                } else {
                    // No es un struct (es int, string, bool, etc.), asumimos 8 bytes.
                    size += 8;
                }
                value_it++;
            } else {
                // Fallback en caso de que no coincidan identificadores y valores
                size += 8;
            }
        }
        // --- FIN DE LA CORRECCIÓN ---
    } else if (auto s = dynamic_cast<IfStmt*>(stmt)) {
        if(s->thenBlock) size += calculate_block_size(s->thenBlock);
        if (s->elseBlock) size += calculate_block_size(s->elseBlock);
    } else if (auto s = dynamic_cast<ForStmt*>(stmt)) {
        if (s->init) size += calculate_stmt_size(s->init);
        if (s->body) size += calculate_block_size(s->body);
    }
    return size;
}

int GoCodeGen::calculate_block_size(Block* block) {
    int size = 0;
    if (block) {
        for(auto s : block->statements) size += calculate_stmt_size(s);
    }
    return size;
}

int GoCodeGen::calculate_stack_size(Program* p) { return 0; }

void GoCodeGen::visit(Program* program) {
    for (auto func : program->functions) {
        func->accept(this);
    }
}

void GoCodeGen::visit(FuncDecl* decl) {
    current_epilogue_label = new_label();

    output << ".globl " << decl->name << endl;
    output << decl->name << ":" << endl;
    output << "  pushq %rbp" << endl;
    output << "  movq %rsp, %rbp" << endl;

    int stack_size = env.get_function(decl->name).stack_size;
    // El 'pushq %rbp' ya desalinea la pila por 8 bytes.
    // Necesitamos que el tamaño total restado a %rsp sea de la forma 16*N - 8
    // para que la pila vuelva a estar alineada a 16 bytes.
    // La forma más simple es redondear el tamaño necesario al múltiplo de 16 más cercano.
    if (stack_size > 0) {
        stack_size = (stack_size + 15) & -16; // Redondea hacia arriba al múltiplo de 16
        output << "  subq $" << stack_size << ", %rsp" << endl;
    }

    current_offset = 0;
    env.add_level();

    int param_offset = 16;
    for (auto param : decl->params) {
        string param_name = param.first;
        Type* param_type_node = param.second;
        
        ImpVType param_type_enum = NOTYPE;
        string struct_name = "";

        if (auto bt = dynamic_cast<BasicType*>(param_type_node)) {
            param_type_enum = ImpValue::get_basic_type(bt->typeName);
        } else if (auto id_type = dynamic_cast<IdentifierType*>(param_type_node)) {
            struct_name = id_type->name;
        }
        
        env.add_var(param_name, param_offset, param_type_enum, struct_name);
        param_offset += 8;
    }

    if (decl->body) {
        decl->body->accept(this);
    }

    output << current_epilogue_label << ":" << endl;
    if (decl->name == "main") {
        output << "  movq $0, %rax" << endl;
    }
    output << "  leave" << endl;
    output << "  ret" << endl;
    
    env.remove_level();
}

void GoCodeGen::visit(Block* block) {
    env.add_level();
    if(block) {
        for (auto stmt : block->statements) {
            stmt->accept(this);
        }
    }
    env.remove_level();
}

void GoCodeGen::visit(ExprStmt* stmt) { stmt->expression->accept(this); }

void GoCodeGen::visit(AssignStmt* stmt) {
    if (auto field_access = dynamic_cast<FieldAccessExp*>(stmt->lhs)) {
        stmt->rhs->accept(this);
        
        IdentifierExp* obj_id = dynamic_cast<IdentifierExp*>(field_access->object);
        if (!obj_id) throw runtime_error("Asignación solo a campos de variables de struct simples.");
        
        VarInfo var_info = env.lookup(obj_id->name);
        StructInfo sinfo = env.get_struct(var_info.struct_name);
        int field_offset = sinfo.offsets.at(field_access->field);
        int total_offset = var_info.offset + field_offset;
        
        output << "  movq %rax, " << total_offset << "(%rbp)" << endl;
    } else {
        if (auto id = dynamic_cast<IdentifierExp*>(stmt->lhs)) {
            VarInfo info = env.lookup(id->name);
            
            if (stmt->op == PLUS_ASSIGN_OP && info.type == TSTRING) {
                this->needs_string_concat = true; 
                stmt->lhs->accept(this);
                output << "  pushq %rax" << endl;
                stmt->rhs->accept(this);
                output << "  movq %rax, %rsi" << endl;
                output << "  popq %rdi" << endl;
                output << "  call _concat_strings" << endl;
                output << "  movq %rax, " << info.offset << "(%rbp)" << endl;
            } else {
                stmt->rhs->accept(this);
                output << "  movq %rax, " << info.offset << "(%rbp)" << endl;
            }
        } else {
             throw runtime_error("LHS de asignación debe ser una variable o campo de struct.");
        }
    }
}

void GoCodeGen::visit(ShortVarDecl* stmt) {
    if (stmt->values.empty() || stmt->identifiers.empty() || stmt->identifiers.size() != stmt->values.size()) {
        throw runtime_error("Declaración corta (:=) inválida.");
    }
    
    auto var_it = stmt->identifiers.begin();
    auto val_it = stmt->values.begin();
    
    while (var_it != stmt->identifiers.end()) {
        ImpValue val_info = (*val_it)->accept(this);
        
        if (val_info.type == TSTRING) {
            current_offset -= 8;
            env.add_var(*var_it, current_offset, TSTRING);
            output << "  movq %rax, " << current_offset << "(%rbp)" << endl;
        } 
        else if (!val_info.struct_name.empty()) {
            StructInfo sinfo = env.get_struct(val_info.struct_name);
            current_offset -= sinfo.size;
            env.add_var(*var_it, current_offset, NOTYPE, sinfo.name);
            
            vector<pair<string, int>> sorted_fields;
            for(auto const& field_map_entry : sinfo.offsets) sorted_fields.push_back(field_map_entry);
            sort(sorted_fields.begin(), sorted_fields.end(), [](const pair<string,int>& a, const pair<string,int>& b) { return a.second < b.second; });

            StructLiteralExp* struct_lit = dynamic_cast<StructLiteralExp*>(*val_it);
            auto value_node_it = struct_lit->values.begin();
            for(const auto& field : sorted_fields) {
                if (value_node_it != struct_lit->values.end()) {
                    (*value_node_it)->accept(this);
                    int total_offset = current_offset + field.second;
                    output << "  movq %rax, " << total_offset << "(%rbp)" << endl;
                    ++value_node_it;
                }
            }
        } 
        else {
            current_offset -= 8;
            env.add_var(*var_it, current_offset, val_info.type);
            output << "  movq %rax, " << current_offset << "(%rbp)" << endl;
        }
        
        ++var_it;
        ++val_it;
    }
}

void GoCodeGen::visit(VarDecl* stmt) {
    auto nameIt = stmt->names.begin();
    auto valueIt = stmt->values.begin();
    bool hasInitializers = !stmt->values.empty();
    
    int var_size = 8;
    ImpVType var_type_enum = NOTYPE;
    string struct_name = "";

    if (auto id_type = dynamic_cast<IdentifierType*>(stmt->type)) {
        struct_name = id_type->name;
        var_size = env.get_struct(struct_name).size;
    } else if (auto basic = dynamic_cast<BasicType*>(stmt->type)) {
        var_type_enum = ImpValue::get_basic_type(basic->typeName);
    }

    while (nameIt != stmt->names.end()) {
        current_offset -= var_size;
        env.add_var(*nameIt, current_offset, var_type_enum, struct_name);
        
        if (hasInitializers && valueIt != stmt->values.end()) {
            (*valueIt)->accept(this);
            output << "  movq %rax, " << current_offset << "(%rbp)" << endl;
            ++valueIt;
        } else {
            for (int i = 0; i < var_size; i += 8) {
                output << "  movq $0, " << current_offset + i << "(%rbp)" << endl;
            }
        }
        
        ++nameIt;
    }
}

void GoCodeGen::visit(IncDecStmt* stmt) {
    VarInfo info = env.lookup(stmt->variable);
    output << "  movq " << info.offset << "(%rbp), %rax" << endl;
    if (stmt->isIncrement) output << "  incq %rax" << endl;
    else output << "  decq %rax" << endl;
    output << "  movq %rax, " << info.offset << "(%rbp)" << endl;
}

void GoCodeGen::visit(IfStmt* stmt) {
    string else_label = new_label();
    string end_label = stmt->elseBlock ? new_label() : else_label;
    
    stmt->condition->accept(this);
    output << "  cmpq $0, %rax" << endl;
    output << "  je " << else_label << endl;
    
    if (stmt->thenBlock) stmt->thenBlock->accept(this);
    if (stmt->elseBlock) output << "  jmp " << end_label << endl;
    output << else_label << ":" << endl;
    if (stmt->elseBlock) stmt->elseBlock->accept(this);
    if (stmt->elseBlock) output << end_label << ":" << endl;
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
    if (stmt->body) stmt->body->accept(this);
    if (stmt->post) stmt->post->accept(this);
    output << "  jmp " << start_label << endl;
    output << end_label << ":" << endl;
    env.remove_level();
}

void GoCodeGen::visit(ReturnStmt* stmt) {
    if (stmt->expression) {
        stmt->expression->accept(this);
    }
    output << "  jmp " << current_epilogue_label << endl;
}

void GoCodeGen::visit(TypeDecl* decl) {}
void GoCodeGen::visit(ImportDecl* decl) {}

ImpValue GoCodeGen::visit(BinaryExp* exp) {
    ImpValue v_left = exp->left->accept(this);
    output << "  pushq %rax" << endl;
    ImpValue v_right = exp->right->accept(this);
    output << "  movq %rax, %rbx" << endl;
    output << "  popq %rax" << endl;

    if (v_left.type == TSTRING && v_right.type == TSTRING) {
        output << "  movq %rax, %rdi" << endl;
        output << "  movq %rbx, %rsi" << endl;

        switch(exp->op) {
            case PLUS_OP:
                this->needs_string_concat = true; 
                output << "  call _concat_strings" << endl;
                return ImpValue(TSTRING);
            case EQ_OP:
                this->needs_string_compare = true; 
                output << "  call _compare_strings" << endl;
                output << "  cmpq $0, %rax" << endl;
                output << "  sete %al" << endl;
                output << "  movzbq %al, %rax" << endl;
                return ImpValue(TBOOL);
            case NE_OP:
                this->needs_string_compare = true; 
                output << "  call _compare_strings" << endl;
                output << "  cmpq $0, %rax" << endl;
                output << "  setne %al" << endl;
                output << "  movzbq %al, %rax" << endl;
                return ImpValue(TBOOL);
            default:
                throw runtime_error("Operador binario no soportado para strings.");
        }
    }

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
            string true_label = new_label(), end_label = new_label();
            output << "  cmpq $0, %rax" << endl; output << "  jne " << true_label << endl;
            output << "  cmpq $0, %rbx" << endl; output << "  jne " << true_label << endl;
            output << "  movq $0, %rax" << endl; output << "  jmp " << end_label << endl;
            output << true_label << ":" << endl; output << "  movq $1, %rax" << endl;
            output << end_label << ":" << endl; break;
        }
    }
    return ImpValue(TBOOL);
compare:
    output << "  cmpq %rbx, %rax" << endl;
    output << "  " << set_instruction << " %al" << endl;
    output << "  movzbq %al, %rax" << endl;
    return ImpValue(TBOOL);
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
    VarInfo info = env.lookup(exp->name);
    if (info.struct_name.empty()) {
        output << "  movq " << info.offset << "(%rbp), %rax" << endl;
    }
    ImpValue val(info.type);
    val.struct_name = info.struct_name;
    return val; 
}

ImpValue GoCodeGen::visit(FunctionCallExp* exp) {
    if (exp->funcName == "len") {
        if (exp->args.size() != 1) throw runtime_error("'len' espera 1 argumento.");
        ImpValue arg_val = exp->args.front()->accept(this);
        if (arg_val.type != TSTRING) throw runtime_error("'len' solo soporta strings.");
        output << "  movq %rax, %rdi" << endl;
        output << "  call strlen@PLT" << endl;
        return ImpValue(TINT);
    }
    if (exp->funcName == "fmt.Println") {
        for (auto arg : exp->args) {
            ImpValue val = arg->accept(this);
            if (val.type == TSTRING) {
                output << "  movq %rax, %rsi" << endl;
                output << "  leaq print_str_fmt(%rip), %rdi" << endl;
            } else if(val.type == TBOOL){
                string false_label = new_label();
                string end_label = new_label();
                output << "  cmpq $0, %rax" << endl;
                output << "  je " << false_label << endl;
                output << "  leaq print_bool_true(%rip), %rdi" << endl;
                output << "  jmp " << end_label << endl;
                output << false_label << ":" << endl;
                output << "  leaq print_bool_false(%rip), %rdi" << endl;
                output << end_label << ":" << endl;
                // Para este caso, %rsi no se usa, ya que la cadena de formato no tiene %s
            } 
            else { // TINT
                output << "  movq %rax, %rsi" << endl;
                output << "  leaq print_fmt(%rip), %rdi" << endl;
            }
            output << "  movl $0, %eax" << endl; // Para funciones variádicas, setear %eax a 0
            output << "  call printf@PLT" << endl;
        }
        return ImpValue();
    } else {
        if (!env.has_function(exp->funcName)) {
            throw runtime_error("Llamada a función no definida '" + exp->funcName + "'");
        }
        
        for (auto it = exp->args.rbegin(); it != exp->args.rend(); ++it) {
            (*it)->accept(this);
            output << "  pushq %rax" << endl;
        }
        
        output << "  call " << exp->funcName << endl;
        
        if (!exp->args.empty()) {
            output << "  addq $" << exp->args.size() * 8 << ", %rsp" << endl;
        }
        
        return ImpValue(env.get_function(exp->funcName).return_type);
    }
}

ImpValue GoCodeGen::visit(FieldAccessExp* exp) {
    IdentifierExp* obj_id = dynamic_cast<IdentifierExp*>(exp->object);
    if (!obj_id) throw runtime_error("Acceso a campos solo en variables.");
    
    VarInfo var_info = env.lookup(obj_id->name);
    if (var_info.struct_name.empty()) throw runtime_error("Variable no es un struct.");
    
    StructInfo sinfo = env.get_struct(var_info.struct_name);
    if (sinfo.offsets.find(exp->field) == sinfo.offsets.end()) {
        throw runtime_error("Struct no tiene campo '" + exp->field + "'.");
    }
    int field_offset = sinfo.offsets.at(exp->field);
    
    int total_offset = var_info.offset + field_offset;
    output << "  movq " << total_offset << "(%rbp), %rax" << endl;

    return ImpValue(sinfo.fields.at(exp->field).type);
}

ImpValue GoCodeGen::visit(StructLiteralExp* exp) {
    if (!env.has_struct(exp->typeName)) {
        throw runtime_error("Uso de tipo struct no definido en un literal.");
    }
    ImpValue val;
    val.type = NOTYPE;
    val.struct_name = exp->typeName;
    return val;
}

ImpValue GoCodeGen::visit(IndexExp* exp) { return ImpValue(); }
ImpValue GoCodeGen::visit(SliceExp* exp) { return ImpValue(); }