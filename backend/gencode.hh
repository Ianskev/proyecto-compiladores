#ifndef GENCODE
#define GENCODE

#include <unordered_map>
#include <vector>
#include <string>

#include "exp.h"
#include "imp_value_visitor.hh"
#include "environment.hh"

using namespace std;

// Structure to represent a struct type
struct StructType {
    string name;
    vector<pair<string, string>> fields;  // field name, type
    unordered_map<string, int> fieldOffsets;  // field name -> offset in the struct
    int size;  // total size in bytes
};

class ImpCODE : public ImpValueVisitor {
private:
  Environment<ImpValue> env;
  unordered_map<string, StructType> structTypes;  // map of struct name -> struct info
  

public:
  int current_offset;
  std::unordered_map<std::string, int> stack_offsets;
  int etiquetas=0;
  int string_count=0;  // Counter for string literals
  vector<string> string_literals;  // Store string literals
  
  void interpret(Program*);
  void interpretGo(GoProgram*);
  void visit(Program*);
  void visit(GoProgram*);
  void visit(Body*);
  void visit(VarDecList*);
  void visit(VarDec*);
  void visit(StatementList*);
  void visit(AssignStatement*);
  void visit(PrintStatement*);
  void visit(IfStatement*);
  void visit(WhileStatement*);
  void visit(ImportDeclaration*);
  void visit(PackageDeclaration*);
  void visit(StructDeclaration*);
  
  ImpValue visit(BinaryExp* e);
  ImpValue visit(NumberExp* e);
  ImpValue visit(BoolExp* e);
  ImpValue visit(IdentifierExp* e);
  ImpValue visit(StringExp* e);
  ImpValue visit(StructFieldAccess* e);
  
  // Helper methods for struct handling
  int calculateStructSize(const vector<pair<string, string>>& fields);
  void calculateFieldOffsets(StructType& structType);
};

#endif

