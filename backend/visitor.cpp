#include <iostream>
#include "exp.h"
#include "visitor.h"

using namespace std;

//=== PrintVisitor Implementation ===
PrintVisitor::PrintVisitor() : indentLevel(0) {}

void PrintVisitor::printIndent() {
    for (int i = 0; i < indentLevel; i++) {
        cout << "  ";
    }
}

void PrintVisitor::increaseIndent() {
    indentLevel++;
}

void PrintVisitor::decreaseIndent() {
    if (indentLevel > 0) indentLevel--;
}

void PrintVisitor::print(Program* program) {
    program->accept(this);
}

// Expresiones
void PrintVisitor::visit(BinaryExp* exp) {
    exp->left->accept(this);
    cout << " " << Exp::binopToString(exp->op) << " ";
    exp->right->accept(this);
}

void PrintVisitor::visit(UnaryExp* exp) {
    cout << Exp::unopToString(exp->op);
    exp->exp->accept(this);
}

void PrintVisitor::visit(NumberExp* exp) {
    cout << exp->value;
}

void PrintVisitor::visit(StringExp* exp) {
    cout << "\"" << exp->value << "\"";
}

void PrintVisitor::visit(BoolExp* exp) {
    cout << (exp->value ? "true" : "false");
}

void PrintVisitor::visit(IdentifierExp* exp) {
    cout << exp->name;
}

void PrintVisitor::visit(FieldAccessExp* exp) {
    exp->object->accept(this);
    cout << "." << exp->field;
}

void PrintVisitor::visit(IndexExp* exp) {
    exp->array->accept(this);
    cout << "[";
    exp->index->accept(this);
    cout << "]";
}

void PrintVisitor::visit(FunctionCallExp* exp) {
    cout << exp->funcName << "(";
    bool first = true;
    for (auto arg : exp->args) {
        if (!first) cout << ", ";
        arg->accept(this);
        first = false;
    }
    cout << ")";
}

void PrintVisitor::visit(StructLiteralExp* exp) {
    cout << exp->typeName << "{";
    bool first = true;
    for (auto val : exp->values) {
        if (!first) cout << ", ";
        val->accept(this);
        first = false;
    }
    cout << "}";
}

// Tipos
void PrintVisitor::visit(BasicType* type) {
    cout << type->typeName;
}

void PrintVisitor::visit(StructType* type) {
    cout << "struct {" << endl;
    increaseIndent();
    for (auto& field : type->fields) {
        printIndent();
        cout << field.first << " ";
        field.second->accept(this);
        cout << endl;
    }
    decreaseIndent();
    printIndent();
    cout << "}";
}

void PrintVisitor::visit(IdentifierType* type) {
    cout << type->name;
}

// Sentencias
void PrintVisitor::visit(ExprStmt* stmt) {
    stmt->expression->accept(this);
}

void PrintVisitor::visit(AssignStmt* stmt) {
    stmt->lhs->accept(this);
    switch (stmt->op) {
        case ASSIGN_OP: cout << " = "; break;
        case PLUS_ASSIGN_OP: cout << " += "; break;
        case MINUS_ASSIGN_OP: cout << " -= "; break;
        case MUL_ASSIGN_OP: cout << " *= "; break;
        case DIV_ASSIGN_OP: cout << " /= "; break;
        case MOD_ASSIGN_OP: cout << " %= "; break;
    }
    stmt->rhs->accept(this);
}

void PrintVisitor::visit(ShortVarDecl* stmt) {
    bool first = true;
    for (auto& var : stmt->vars) {
        if (!first) cout << ", ";
        cout << var;
        first = false;
    }
    cout << " := ";
    first = true;
    for (auto val : stmt->values) {
        if (!first) cout << ", ";
        val->accept(this);
        first = false;
    }
}

void PrintVisitor::visit(IncDecStmt* stmt) {
    cout << stmt->var;
    if (stmt->isIncrement) cout << "++";
    else cout << "--";
}

void PrintVisitor::visit(IfStmt* stmt) {
    cout << "if ";
    stmt->condition->accept(this);
    cout << " ";
    stmt->thenBlock->accept(this);
    if (stmt->elseBlock) {
        cout << " else ";
        stmt->elseBlock->accept(this);
    }
}

void PrintVisitor::visit(ForStmt* stmt) {
    cout << "for ";
    if (stmt->init || stmt->condition || stmt->post) {
        if (stmt->init) stmt->init->accept(this);
        cout << "; ";
        if (stmt->condition) stmt->condition->accept(this);
        cout << "; ";
        if (stmt->post) stmt->post->accept(this);
    } else if (stmt->condition) {
        stmt->condition->accept(this);
    }
    cout << " ";
    stmt->body->accept(this);
}

void PrintVisitor::visit(ReturnStmt* stmt) {
    cout << "return";
    if (stmt->expression) {
        cout << " ";
        stmt->expression->accept(this);
    }
}

void PrintVisitor::visit(VarDecl* stmt) {
    cout << "var ";
    bool first = true;
    for (auto& name : stmt->names) {
        if (!first) cout << ", ";
        cout << name;
        first = false;
    }
    cout << " ";
    stmt->type->accept(this);
    if (!stmt->values.empty()) {
        cout << " = ";
        first = true;
        for (auto val : stmt->values) {
            if (!first) cout << ", ";
            val->accept(this);
            first = false;
        }
    }
}

// Declaraciones
void PrintVisitor::visit(TypeDecl* decl) {
    cout << "type " << decl->name << " ";
    decl->structType->accept(this);
}

void PrintVisitor::visit(FuncDecl* decl) {
    cout << "func " << decl->name << "(";
    bool first = true;
    for (auto& param : decl->params) {
        if (!first) cout << ", ";
        cout << param.first << " ";
        param.second->accept(this);
        first = false;
    }
    cout << ")";
    if (decl->returnType) {
        cout << " ";
        decl->returnType->accept(this);
    }
    cout << " ";
    decl->body->accept(this);
}

void PrintVisitor::visit(Block* block) {
    cout << "{" << endl;
    increaseIndent();
    for (auto stmt : block->statements) {
        printIndent();
        stmt->accept(this);
        cout << endl;
    }
    decreaseIndent();
    printIndent();
    cout << "}";
}

void PrintVisitor::visit(ImportDecl* decl) {
    cout << "import " << decl->path;
}

void PrintVisitor::visit(Program* program) {
    cout << "package " << program->packageName << endl << endl;
    
    for (auto imp : program->imports) {
        imp->accept(this);
        cout << endl;
    }
    if (!program->imports.empty()) cout << endl;
    
    for (auto var : program->globalVars) {
        var->accept(this);
        cout << endl;
    }
    if (!program->globalVars.empty()) cout << endl;
    
    for (auto type : program->types) {
        type->accept(this);
        cout << endl << endl;
    }
    
    for (auto func : program->functions) {
        func->accept(this);
        cout << endl << endl;
    }
}
