#ifndef EXP_H
#define EXP_H
#include "imp_value.hh"
#include "imp_type.hh"
#include <string>
#include <unordered_map>
#include <list>
#include "visitor.h"
using namespace std;
enum BinaryOp { 
    PLUS_OP, MINUS_OP, MUL_OP, DIV_OP, LT_OP, LE_OP, EQ_OP, 
    // New operators for Go
    GT_OP, GE_OP, NE_OP, AND_OP, OR_OP, MOD_OP
};

class Body;
class ImpValueVisitor;
class Exp {
public:
    virtual int  accept(Visitor* visitor) = 0;
    virtual ImpValue accept(ImpValueVisitor* v) = 0;
    virtual ~Exp() = 0;
    static string binopToChar(BinaryOp op);
};



class BinaryExp : public Exp {
public:
    Exp *left, *right;
    BinaryOp op;
    BinaryExp(Exp* l, Exp* r, BinaryOp op);
    int accept(Visitor* visitor);
    ImpValue accept(ImpValueVisitor* v);
    ~BinaryExp();
};

class NumberExp : public Exp {
public:
    int value;
    NumberExp(int v);
    int accept(Visitor* visitor);
    ImpValue accept(ImpValueVisitor* v);
    ~NumberExp();
};

class BoolExp : public Exp {
public:
    int value;
    BoolExp(bool v);
    int accept(Visitor* visitor);
    ImpValue accept(ImpValueVisitor* v);
    ~BoolExp();
};

class IdentifierExp : public Exp {
public:
    std::string name;
    IdentifierExp(const std::string& n);
    int accept(Visitor* visitor);
    ImpValue accept(ImpValueVisitor* v);
    ~IdentifierExp();
};

// New class for string literals
class StringExp : public Exp {
public:
    string value;
    StringExp(const string& v);
    int accept(Visitor* visitor);
    ImpValue accept(ImpValueVisitor* v);
    ~StringExp();
};


class Stm {
public:
    virtual int accept(Visitor* visitor) = 0;
    virtual ~Stm() = 0;
    virtual void accept(ImpValueVisitor* v) = 0;
};


class AssignStatement : public Stm {
public:
    std::string id;
    Exp* rhs;
    AssignStatement(std::string id, Exp* e);
    int accept(Visitor* visitor);
    void accept(ImpValueVisitor* v);
    ~AssignStatement();
};

class PrintStatement : public Stm {
public:
    Exp* e;
    PrintStatement(Exp* e);
    int accept(Visitor* visitor);
    void accept(ImpValueVisitor* v);
    ~PrintStatement();
};


class IfStatement : public Stm {
public:
    Exp* condition;
    Body* then;
    Body* els;
    IfStatement(Exp* condition, Body* then, Body* els);
    int accept(Visitor* visitor);
    void accept(ImpValueVisitor* v);
    ~IfStatement();
};
class WhileStatement : public Stm {
public:
    Exp* condition;
    Body* b;
    WhileStatement(Exp* condition, Body* b);
    int accept(Visitor* visitor);
    void accept(ImpValueVisitor* v);
    ~WhileStatement();
};

// New statement types for Go language
class StructDeclaration : public Stm {
public:
    string name;
    list<pair<string, string>> fields; // field name, type name
    StructDeclaration(const string& name, const list<pair<string, string>>& fields);
    int accept(Visitor* visitor);
    void accept(ImpValueVisitor* v);
    ~StructDeclaration();
};

class StructFieldAccess : public Exp {
public:
    Exp* structure;
    string field;
    StructFieldAccess(Exp* structure, const string& field);
    int accept(Visitor* visitor);
    ImpValue accept(ImpValueVisitor* v);
    ~StructFieldAccess();
};

// For import statements
class ImportDeclaration : public Stm {
public:
    string path;
    ImportDeclaration(const string& path);
    int accept(Visitor* visitor);
    void accept(ImpValueVisitor* v);
    ~ImportDeclaration();
};

// For package declarations
class PackageDeclaration : public Stm {
public:
    string name;
    PackageDeclaration(const string& name);
    int accept(Visitor* visitor);
    void accept(ImpValueVisitor* v);
    ~PackageDeclaration();
};

// Enhanced program class to include package and imports
class GoProgram {
public:
    PackageDeclaration* package;
    list<ImportDeclaration*> imports;
    Body* body;
    GoProgram(PackageDeclaration* package, const list<ImportDeclaration*>& imports, Body* body);
    ~GoProgram();
    int accept(Visitor* v);
    void accept(ImpValueVisitor* v);
};



#endif // EXP_H