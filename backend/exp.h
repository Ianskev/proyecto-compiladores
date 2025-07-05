#ifndef EXP_H
#define EXP_H

#include <string>
#include <list>
#include <vector>

using namespace std;

// Forward declarations
class Visitor;

//=== ENUMS ===
enum BinaryOp { 
    PLUS_OP, MINUS_OP, MUL_OP, DIV_OP, MOD_OP,
    LT_OP, LE_OP, GT_OP, GE_OP, EQ_OP, NE_OP,
    AND_OP, OR_OP
};

enum UnaryOp { 
    UPLUS_OP, UMINUS_OP, NOT_OP 
};

enum AssignOp {
    ASSIGN_OP, PLUS_ASSIGN_OP, MINUS_ASSIGN_OP, 
    MUL_ASSIGN_OP, DIV_ASSIGN_OP, MOD_ASSIGN_OP
};

//=== EXPRESIONES ===
class Exp {
public:
    virtual ~Exp() = 0;
    virtual void accept(Visitor* visitor) = 0;
    static string binopToString(BinaryOp op);
    static string unopToString(UnaryOp op);
};

class BinaryExp : public Exp {
public:
    Exp* left;
    Exp* right;
    BinaryOp op;
    BinaryExp(Exp* l, Exp* r, BinaryOp op);
    void accept(Visitor* visitor) override;
    ~BinaryExp();
};

class UnaryExp : public Exp {
public:
    Exp* exp;
    UnaryOp op;
    UnaryExp(Exp* e, UnaryOp op);
    void accept(Visitor* visitor) override;
    ~UnaryExp();
};

class NumberExp : public Exp {
public:
    int value;
    NumberExp(int v);
    void accept(Visitor* visitor) override;
    ~NumberExp();
};

class StringExp : public Exp {
public:
    string value;
    StringExp(const string& v);
    void accept(Visitor* visitor) override;
    ~StringExp();
};

class BoolExp : public Exp {
public:
    bool value;
    BoolExp(bool v);
    void accept(Visitor* visitor) override;
    ~BoolExp();
};

class IdentifierExp : public Exp {
public:
    string name;
    IdentifierExp(const string& n);
    void accept(Visitor* visitor) override;
    ~IdentifierExp();
};

class FieldAccessExp : public Exp {
public:
    Exp* object;
    string field;
    FieldAccessExp(Exp* obj, const string& f);
    void accept(Visitor* visitor) override;
    ~FieldAccessExp();
};

class IndexExp : public Exp {
public:
    Exp* array;
    Exp* index;
    IndexExp(Exp* arr, Exp* idx);
    void accept(Visitor* visitor) override;
    ~IndexExp();
};

class FunctionCallExp : public Exp {
public:
    string funcName;
    list<Exp*> args;
    FunctionCallExp(const string& name, list<Exp*> arguments);
    void accept(Visitor* visitor) override;
    ~FunctionCallExp();
};

class StructLiteralExp : public Exp {
public:
    string typeName;
    list<Exp*> values; // Para inicialización posicional
    StructLiteralExp(const string& type, list<Exp*> vals);
    void accept(Visitor* visitor) override;
    ~StructLiteralExp();
};

class SliceExp : public Exp {
public:
    Exp* array;
    Exp* start;  // puede ser nullptr
    Exp* end;    // puede ser nullptr
    SliceExp(Exp* arr, Exp* startIdx, Exp* endIdx);
    void accept(Visitor* visitor) override;
    ~SliceExp();
};

//=== TIPOS ===
class Type {
public:
    virtual ~Type() = 0;
    virtual void accept(Visitor* visitor) = 0;
    virtual string toString() = 0;
};

class BasicType : public Type {
public:
    string typeName; // "int", "string"
    BasicType(const string& name);
    void accept(Visitor* visitor) override;
    string toString() override;
    ~BasicType();
};

class StructType : public Type {
public:
    list<pair<string, Type*>> fields; // (nombre, tipo)
    StructType(list<pair<string, Type*>> fieldList);
    void accept(Visitor* visitor) override;
    string toString() override;
    ~StructType();
};

class IdentifierType : public Type {
public:
    string name; // Para tipos definidos por el usuario
    IdentifierType(const string& n);
    void accept(Visitor* visitor) override;
    string toString() override;
    ~IdentifierType();
};

//=== SENTENCIAS ===
class Stmt {
public:
    virtual ~Stmt() = 0;
    virtual void accept(Visitor* visitor) = 0;
};

class ExprStmt : public Stmt {
public:
    Exp* expression;
    ExprStmt(Exp* exp);
    void accept(Visitor* visitor) override;
    ~ExprStmt();
};

class AssignStmt : public Stmt {
public:
    Exp* lhs;
    Exp* rhs;
    AssignOp op;
    AssignStmt(Exp* left, Exp* right, AssignOp operation);
    void accept(Visitor* visitor) override;
    ~AssignStmt();
};

class ShortVarDecl : public Stmt {
public:
    list<string> vars;
    list<Exp*> values;
    ShortVarDecl(list<string> variables, list<Exp*> vals);
    void accept(Visitor* visitor) override;
    ~ShortVarDecl();
};

class IncDecStmt : public Stmt {
public:
    string var;
    bool isIncrement; // true para ++, false para --
    IncDecStmt(const string& variable, bool inc);
    void accept(Visitor* visitor) override;
    ~IncDecStmt();
};

class IfStmt : public Stmt {
public:
    Exp* condition;
    class Block* thenBlock;
    class Block* elseBlock; // puede ser nullptr
    IfStmt(Exp* cond, Block* thenB, Block* elseB = nullptr);
    void accept(Visitor* visitor) override;
    ~IfStmt();
};

class ForStmt : public Stmt {
public:
    Stmt* init;        // puede ser nullptr
    Exp* condition;    // puede ser nullptr
    Stmt* post;        // puede ser nullptr
    class Block* body;
    ForStmt(Stmt* initStmt, Exp* cond, Stmt* postStmt, Block* bodyBlock);
    void accept(Visitor* visitor) override;
    ~ForStmt();
};

class ReturnStmt : public Stmt {
public:
    Exp* expression; // puede ser nullptr
    ReturnStmt(Exp* exp = nullptr);
    void accept(Visitor* visitor) override;
    ~ReturnStmt();
};

//=== DECLARACIONES ===
class VarDecl : public Stmt {
public:
    list<string> names;
    Type* type;
    list<Exp*> values; // puede estar vacía
    VarDecl(list<string> varNames, Type* varType, list<Exp*> initValues = list<Exp*>());
    void accept(Visitor* visitor) override;
    ~VarDecl();
};

class TypeDecl {
public:
    string name;
    StructType* structType;
    TypeDecl(const string& typeName, StructType* type);
    void accept(Visitor* visitor);
    ~TypeDecl();
};

class FuncDecl {
public:
    string name;
    list<pair<string, Type*>> params; // (nombre, tipo)
    Type* returnType; // puede ser nullptr para void
    class Block* body;
    FuncDecl(const string& funcName, list<pair<string, Type*>> parameters, 
             Type* retType, Block* bodyBlock);
    void accept(Visitor* visitor);
    ~FuncDecl();
};

//=== BLOQUES Y ESTRUCTURAS ===
class Block {
public:
    list<Stmt*> statements;
    Block(list<Stmt*> stmts);
    void accept(Visitor* visitor);
    ~Block();
};

class ImportDecl {
public:
    string path;
    ImportDecl(const string& importPath);
    void accept(Visitor* visitor);
    ~ImportDecl();
};

class Program {
public:
    string packageName;
    list<ImportDecl*> imports;
    list<VarDecl*> globalVars;
    list<TypeDecl*> types;
    list<FuncDecl*> functions;
    
    Program(const string& pkg, list<ImportDecl*> imps, 
            list<VarDecl*> vars, list<TypeDecl*> typeDecls, 
            list<FuncDecl*> funcs);
    void accept(Visitor* visitor);
    ~Program();
};

#endif // EXP_H
