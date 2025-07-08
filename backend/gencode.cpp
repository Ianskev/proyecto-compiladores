#include "gencode.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include "helpers_gencode.cpp" // Incluimos los visitors auxiliares

using namespace std;

GoCodeGen::GoCodeGen(std::ostream& out) 
    : current_offset(0), label_counter(0), string_counter(0), output(out) {}

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
        size += s->identifiers.size() * 16;
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
    if (stack_size > 0) {
        if (stack_size % 16 != 0) stack_size = ((stack_size + 15) / 16) * 16;
        if (stack_size > 0) output << "  subq $" << stack_size << ", %rsp" << endl;
    }

    current_offset = 0;
    env.add_level();

    int param_offset = 16;
    for (auto param : decl->params) {
        string param_name = param.first;
        Type* param_type_node = param.second;
        
        if (auto id_type = dynamic_cast<IdentifierType*>(param_type_node)) {
            if (env.has_struct(id_type->name)) {
                StructInfo sinfo = env.get_struct(id_type->name);
                env.add_var(param_name, param_offset, NOTYPE, sinfo.name);
                param_offset += sinfo.size;
            }
        } else {
            env.add_var(param_name, param_offset, TINT);
            param_offset += 8;
        }
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
        
        output << "  movq %rax, " << total_offset << "(%rbp) # Asignar a " << obj_id->name << "." << field_access->field << endl;
    } else {
        stmt->rhs->accept(this);
        if (auto id = dynamic_cast<IdentifierExp*>(stmt->lhs)) {
            int offset = env.lookup(id->name).offset;
            output << "  movq %rax, " << offset << "(%rbp) # Almacenar en " << id->name << endl;
        }
    }
}

void GoCodeGen::visit(ShortVarDecl* stmt) {
    // Verificar que haya al menos un valor
    if (stmt->values.empty() || stmt->identifiers.empty()) {
        throw runtime_error("Declaración corta (:=) debe tener al menos un inicializador.");
    }
    
    // Verificar que la cantidad de identificadores coincida con la cantidad de valores
    if (stmt->identifiers.size() != stmt->values.size()) {
        throw runtime_error("Cantidad de identificadores no coincide con cantidad de valores en declaración corta.");
    }
    
    // Iteramos por cada par de identificador y valor
    auto var_it = stmt->identifiers.begin();
    auto val_it = stmt->values.begin();
    
    while (var_it != stmt->identifiers.end() && val_it != stmt->values.end()) {
        if (!(*val_it)) {
            throw runtime_error("Valor nulo en declaración corta (:=).");
        }
        
        // Procesar el valor actual
        ImpValue val_info = (*val_it)->accept(this);
        
        if (val_info.type == TSTRING) {
            current_offset -= 8;
            env.add_var(*var_it, current_offset, TSTRING);
            output << "  movq %rax, " << current_offset << "(%rbp) # Inicializar " << *var_it << endl;
        } 
        else if (!val_info.struct_name.empty()) {
            StructInfo sinfo = env.get_struct(val_info.struct_name);
            current_offset -= sinfo.size;
            env.add_var(*var_it, current_offset, NOTYPE, sinfo.name);
            
            output << "# Inicializando struct " << *var_it << " en offset " << current_offset << endl;
            
            vector<pair<string, int>> sorted_fields;
            for(auto const& field_map_entry : sinfo.offsets) sorted_fields.push_back(field_map_entry);
            sort(sorted_fields.begin(), sorted_fields.end(), [](const pair<string,int>& a, const pair<string,int>& b) { return a.second < b.second; });

            StructLiteralExp* struct_lit = dynamic_cast<StructLiteralExp*>(*val_it);
            auto value_node_it = struct_lit->values.begin();
            for(const auto& field : sorted_fields) {
                if (value_node_it != struct_lit->values.end()) {
                    (*value_node_it)->accept(this);
                    int total_offset = current_offset + field.second;
                    output << "  movq %rax, " << total_offset << "(%rbp) # " << *var_it << "." << field.first << endl;
                    ++value_node_it;
                }
            }
        } 
        else {
            current_offset -= 8;
            env.add_var(*var_it, current_offset, val_info.type);
            output << "  movq %rax, " << current_offset << "(%rbp) # Inicializar " << *var_it << endl;
        }
        
        // Avanzar a la siguiente variable y valor
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
        if (env.has_struct(struct_name)) {
            var_size = env.get_struct(struct_name).size;
        } else {
            throw runtime_error("Uso de tipo no definido: " + struct_name);
        }
    } else if (auto basic = dynamic_cast<BasicType*>(stmt->type)) {
        var_type_enum = ImpValue::get_basic_type(basic->typeName);
    }

    while (nameIt != stmt->names.end()) {
        current_offset -= var_size;
        env.add_var(*nameIt, current_offset, var_type_enum, struct_name);
        
        if (hasInitializers && valueIt != stmt->values.end()) {
            // Process the initializer
            ImpValue val_info = (*valueIt)->accept(this);
            output << "  movq %rax, " << current_offset << "(%rbp) # Inicializar " << *nameIt << endl;
            ++valueIt;
        } else {
            // Default initialization to 0
            for (int i = 0; i < var_size; i += 8) {
                output << "  movq $0, " << current_offset + i << "(%rbp)" << endl;
            }
            output << "  # Inicialización por defecto de " << *nameIt << endl;
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
    output << "  cmpq %rbx, %rax" << endl; output << "  " << set_instruction << " %al" << endl;
    output << "  movzbq %al, %rax" << endl; return ImpValue(TBOOL);
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
        output << "  movq " << info.offset << "(%rbp), %rax  # Cargar " << exp->name << endl;
    }
    // Si es un struct, no cargamos nada. El llamador (FieldAccess, Assign, etc.)
    // usará la información de 'info' para saber qué hacer.
    ImpValue val(info.type);
    val.struct_name = info.struct_name;
    return val; 
}

ImpValue GoCodeGen::visit(FunctionCallExp* exp) {
    if (exp->funcName == "fmt.Println") {
        for (auto arg : exp->args) {
            ImpValue val = arg->accept(this);
            if (val.type == TSTRING) {
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
            } 
            else {
                output << "  leaq print_fmt(%rip), %rdi" << endl;
            }
            output << "  movq %rax, %rsi" << endl;
            output << "  xorq %rax, %rax" << endl;
            output << "  call printf@PLT" << endl;
        }
        return ImpValue();
    } else {
        if (!env.has_function(exp->funcName)) {
            throw runtime_error("Llamada a función no definida '" + exp->funcName + "'");
        }
        
        int arg_space = 0;
        vector<int> arg_sizes;
        for(auto arg : exp->args){
            if(auto id_exp = dynamic_cast<IdentifierExp*>(arg)){
                if(env.check(id_exp->name) && !env.lookup(id_exp->name).struct_name.empty()){
                    int size = env.get_struct(env.lookup(id_exp->name).struct_name).size;
                    arg_space += size;
                    arg_sizes.push_back(size);
                    continue;
                }
            }
            arg_space += 8;
            arg_sizes.push_back(8);
        }

        if (arg_space > 0) output << "  subq $" << arg_space << ", %rsp" << endl;

        int current_arg_offset = 0;
        auto size_it = arg_sizes.begin();
        for (auto arg : exp->args) {
             if(auto id_exp = dynamic_cast<IdentifierExp*>(arg)){
                if(env.check(id_exp->name) && !env.lookup(id_exp->name).struct_name.empty()){
                    VarInfo var_info = env.lookup(id_exp->name);
                    for(int i=0; i < *size_it; i+=8){
                        output << "  movq " << var_info.offset + i << "(%rbp), %r10" << endl;
                        output << "  movq %r10, " << current_arg_offset + i << "(%rsp)" << endl;
                    }
                    current_arg_offset += *size_it;
                    ++size_it;
                    continue;
                }
            }
            arg->accept(this);
            output << "  movq %rax, " << current_arg_offset << "(%rsp)" << endl;
            current_arg_offset += *size_it;
            ++size_it;
        }
        
        output << "  call " << exp->funcName << endl;
        if (arg_space > 0) output << "  addq $" << arg_space << ", %rsp" << endl;
        
        return ImpValue(env.get_function(exp->funcName).return_type);
    }
}

ImpValue GoCodeGen::visit(FieldAccessExp* exp) {
    IdentifierExp* obj_id = dynamic_cast<IdentifierExp*>(exp->object);
    if (!obj_id) throw runtime_error("Acceso a campos solo en variables.");
    
    VarInfo var_info = env.lookup(obj_id->name);
    if (var_info.struct_name.empty()) throw runtime_error("Variable '" + obj_id->name + "' no es un struct.");
    
    StructInfo sinfo = env.get_struct(var_info.struct_name);
    if (sinfo.offsets.find(exp->field) == sinfo.offsets.end()) {
        throw runtime_error("Struct '" + sinfo.name + "' no tiene campo '" + exp->field + "'.");
    }
    int field_offset = sinfo.offsets.at(exp->field);
    
    int total_offset = var_info.offset + field_offset;
    output << "  movq " << total_offset << "(%rbp), %rax  # Acceder a " << obj_id->name << "." << exp->field << endl;

    return ImpValue(sinfo.fields.at(exp->field).type);
}

// <<< --- IMPLEMENTACIÓN DE visit(StructLiteralExp) --- >>>
ImpValue GoCodeGen::visit(StructLiteralExp* exp) {
    if (!env.has_struct(exp->typeName)) {
        throw runtime_error("Uso de tipo struct no definido '" + exp->typeName + "' en un literal.");
    }
    ImpValue val;
    val.type = NOTYPE;
    val.struct_name = exp->typeName;
    return val;
}

ImpValue GoCodeGen::visit(IndexExp* exp) { return ImpValue(); }
ImpValue GoCodeGen::visit(SliceExp* exp) { return ImpValue(); }