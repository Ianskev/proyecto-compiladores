#include <iostream>
#include "exp.h"
#include "visitor.h"

using namespace std;

// Implementación de destructores abstractos
Exp::~Exp() {}
Type::~Type() {}
Stmt::~Stmt() {}

// Métodos estáticos de utilidad
string Exp::binopToString(BinaryOp op) {
    switch(op) {
        case PLUS_OP: return "+";
        case MINUS_OP: return "-";
        case MUL_OP: return "*";
        case DIV_OP: return "/";
        case MOD_OP: return "%";
        case LT_OP: return "<";
        case LE_OP: return "<=";
        case GT_OP: return ">";
        case GE_OP: return ">=";
        case EQ_OP: return "==";
        case NE_OP: return "!=";
        case AND_OP: return "&&";
        case OR_OP: return "||";
        default: return "?";
    }
}

string Exp::unopToString(UnaryOp op) {
    switch(op) {
        case UPLUS_OP: return "+";
        case UMINUS_OP: return "-";
        case NOT_OP: return "!";
        default: return "?";
    }
}

//=== EXPRESIONES ===
BinaryExp::BinaryExp(Exp* l, Exp* r, BinaryOp operation) : left(l), right(r), op(operation) {}
BinaryExp::~BinaryExp() { delete left; delete right; }
void BinaryExp::accept(Visitor* visitor) { visitor->visit(this); }

UnaryExp::UnaryExp(Exp* e, UnaryOp operation) : exp(e), op(operation) {}
UnaryExp::~UnaryExp() { delete exp; }
void UnaryExp::accept(Visitor* visitor) { visitor->visit(this); }

NumberExp::NumberExp(int v) : value(v) {}
NumberExp::~NumberExp() {}
void NumberExp::accept(Visitor* visitor) { visitor->visit(this); }

StringExp::StringExp(const string& v) : value(v) {}
StringExp::~StringExp() {}
void StringExp::accept(Visitor* visitor) { visitor->visit(this); }

BoolExp::BoolExp(bool v) : value(v) {}
BoolExp::~BoolExp() {}
void BoolExp::accept(Visitor* visitor) { visitor->visit(this); }

IdentifierExp::IdentifierExp(const string& n) : name(n) {}
IdentifierExp::~IdentifierExp() {}
void IdentifierExp::accept(Visitor* visitor) { visitor->visit(this); }

FieldAccessExp::FieldAccessExp(Exp* obj, const string& f) : object(obj), field(f) {}
FieldAccessExp::~FieldAccessExp() { delete object; }
void FieldAccessExp::accept(Visitor* visitor) { visitor->visit(this); }

IndexExp::IndexExp(Exp* arr, Exp* idx) : array(arr), index(idx) {}
IndexExp::~IndexExp() { delete array; delete index; }
void IndexExp::accept(Visitor* visitor) { visitor->visit(this); }

FunctionCallExp::FunctionCallExp(const string& name, list<Exp*> arguments) 
    : funcName(name), args(arguments) {}
FunctionCallExp::~FunctionCallExp() {
    for (auto arg : args) delete arg;
}
void FunctionCallExp::accept(Visitor* visitor) { visitor->visit(this); }

StructLiteralExp::StructLiteralExp(const string& type, list<Exp*> vals) 
    : typeName(type), values(vals) {}
StructLiteralExp::~StructLiteralExp() {
    for (auto val : values) delete val;
}
void StructLiteralExp::accept(Visitor* visitor) { visitor->visit(this); }

//=== TIPOS ===
BasicType::BasicType(const string& name) : typeName(name) {}
BasicType::~BasicType() {}
void BasicType::accept(Visitor* visitor) { visitor->visit(this); }
string BasicType::toString() { return typeName; }

StructType::StructType(list<pair<string, Type*>> fieldList) : fields(fieldList) {}
StructType::~StructType() {
    for (auto& field : fields) delete field.second;
}
void StructType::accept(Visitor* visitor) { visitor->visit(this); }
string StructType::toString() { return "struct"; }

IdentifierType::IdentifierType(const string& n) : name(n) {}
IdentifierType::~IdentifierType() {}
void IdentifierType::accept(Visitor* visitor) { visitor->visit(this); }
string IdentifierType::toString() { return name; }

//=== SENTENCIAS ===
ExprStmt::ExprStmt(Exp* exp) : expression(exp) {}
ExprStmt::~ExprStmt() { delete expression; }
void ExprStmt::accept(Visitor* visitor) { visitor->visit(this); }

AssignStmt::AssignStmt(Exp* left, Exp* right, AssignOp operation) 
    : lhs(left), rhs(right), op(operation) {}
AssignStmt::~AssignStmt() { delete lhs; delete rhs; }
void AssignStmt::accept(Visitor* visitor) { visitor->visit(this); }

ShortVarDecl::ShortVarDecl(list<string> variables, list<Exp*> vals) 
    : vars(variables), values(vals) {}
ShortVarDecl::~ShortVarDecl() {
    for (auto val : values) delete val;
}
void ShortVarDecl::accept(Visitor* visitor) { visitor->visit(this); }

IncDecStmt::IncDecStmt(const string& variable, bool inc) : var(variable), isIncrement(inc) {}
IncDecStmt::~IncDecStmt() {}
void IncDecStmt::accept(Visitor* visitor) { visitor->visit(this); }

IfStmt::IfStmt(Exp* cond, Block* thenB, Block* elseB) 
    : condition(cond), thenBlock(thenB), elseBlock(elseB) {}
IfStmt::~IfStmt() { 
    delete condition; 
    delete thenBlock; 
    if (elseBlock) delete elseBlock; 
}
void IfStmt::accept(Visitor* visitor) { visitor->visit(this); }

ForStmt::ForStmt(Stmt* initStmt, Exp* cond, Stmt* postStmt, Block* bodyBlock)
    : init(initStmt), condition(cond), post(postStmt), body(bodyBlock) {}
ForStmt::~ForStmt() {
    if (init) delete init;
    if (condition) delete condition;
    if (post) delete post;
    delete body;
}
void ForStmt::accept(Visitor* visitor) { visitor->visit(this); }

ReturnStmt::ReturnStmt(Exp* exp) : expression(exp) {}
ReturnStmt::~ReturnStmt() { if (expression) delete expression; }
void ReturnStmt::accept(Visitor* visitor) { visitor->visit(this); }

//=== DECLARACIONES ===
VarDecl::VarDecl(list<string> varNames, Type* varType, list<Exp*> initValues)
    : names(varNames), type(varType), values(initValues) {}
VarDecl::~VarDecl() {
    delete type;
    for (auto val : values) delete val;
}
void VarDecl::accept(Visitor* visitor) { visitor->visit(this); }

TypeDecl::TypeDecl(const string& typeName, StructType* type) 
    : name(typeName), structType(type) {}
TypeDecl::~TypeDecl() { delete structType; }
void TypeDecl::accept(Visitor* visitor) { visitor->visit(this); }

FuncDecl::FuncDecl(const string& funcName, list<pair<string, Type*>> parameters, 
                   Type* retType, Block* bodyBlock)
    : name(funcName), params(parameters), returnType(retType), body(bodyBlock) {}
FuncDecl::~FuncDecl() {
    for (auto& param : params) delete param.second;
    if (returnType) delete returnType;
    delete body;
}
void FuncDecl::accept(Visitor* visitor) { visitor->visit(this); }

//=== BLOQUES Y ESTRUCTURAS ===
Block::Block(list<Stmt*> stmts) : statements(stmts) {}
Block::~Block() {
    for (auto stmt : statements) delete stmt;
}
void Block::accept(Visitor* visitor) { visitor->visit(this); }

ImportDecl::ImportDecl(const string& importPath) : path(importPath) {}
ImportDecl::~ImportDecl() {}
void ImportDecl::accept(Visitor* visitor) { visitor->visit(this); }

Program::Program(const string& pkg, list<ImportDecl*> imps, 
                 list<VarDecl*> vars, list<TypeDecl*> typeDecls, 
                 list<FuncDecl*> funcs)
    : packageName(pkg), imports(imps), globalVars(vars), types(typeDecls), functions(funcs) {}
Program::~Program() {
    for (auto imp : imports) delete imp;
    for (auto var : globalVars) delete var;
    for (auto type : types) delete type;
    for (auto func : functions) delete func;
}
void Program::accept(Visitor* visitor) { visitor->visit(this); }

SliceExp::SliceExp(Exp* arr, Exp* startIdx, Exp* endIdx) 
    : array(arr), start(startIdx), end(endIdx) {}
SliceExp::~SliceExp() {
    delete array;
    if (start) delete start;
    if (end) delete end;
}
void SliceExp::accept(Visitor* visitor) { visitor->visit(this); }
