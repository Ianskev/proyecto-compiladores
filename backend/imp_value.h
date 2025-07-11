#ifndef IMP_VALUE_H
#define IMP_VALUE_H

#include <string>

using namespace std;

enum ImpVType { NOTYPE, TINT, TBOOL, TSTRING };

class ImpValue {
public:
    ImpValue();
    ImpVType type;
    int int_value;
    bool bool_value;
    string string_value;
    string struct_name; // Para saber si es un struct
    
    ImpValue(ImpVType t);
    ImpValue(int v);
    ImpValue(bool v);
    ImpValue(string v);
    
    void set_default_value(ImpVType tt);
    static ImpVType get_basic_type(string s);
    string to_string();
};

#endif