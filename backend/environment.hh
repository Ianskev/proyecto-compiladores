#ifndef GO_ENVIRONMENT_H
#define GO_ENVIRONMENT_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include "imp_value.h"

using namespace std;

struct VarInfo {
    int offset;
    ImpVType type;
    string struct_name;
};

struct FieldInfo {
    string type_name;
    ImpVType type;
    int size;
};

struct StructInfo {
    string name;
    std::unordered_map<string, FieldInfo> fields; 
    std::unordered_map<string, int> offsets;      
    int size = 0;
};

struct FuncInfo {
    int stack_size; 
    ImpVType return_type;
};

class Environment {
private:
    vector<unordered_map<string, VarInfo>> var_levels;
    unordered_map<string, FuncInfo> functions;
    unordered_map<string, StructInfo> structs;

    int search_rib(const string& var) {
        int idx = var_levels.size() - 1;
        while (idx >= 0) {
            if (var_levels[idx].find(var) != var_levels[idx].end()) {
                return idx;
            }
            idx--;
        }
        return -1;
    }

public:
    Environment() {}

    void clear() {
        var_levels.clear();
        functions.clear();
        structs.clear();
    }

    void add_level() {
        var_levels.push_back({});
    }

    bool remove_level() {
        if (!var_levels.empty()) {
            var_levels.pop_back();
            return true;
        }
        return false;
    }

    void add_var(const string& var, int offset, ImpVType type, const string& struct_name = "") {
        if (var_levels.empty()) {
            cout << "Environment sin niveles: no se pueden agregar variables" << endl;
            exit(1);
        }
        var_levels.back()[var] = {offset, type, struct_name};
    }
    
    bool check(const string& x) {
        return search_rib(x) >= 0;
    }

    VarInfo lookup(const string& x) {
        int idx = search_rib(x);
        if (idx < 0) {
            cerr << "Error en tiempo de compilación: Variable no declarada: " << x << endl;
            exit(1);
        }
        return var_levels[idx][x];
    }

    void add_function(const string& name, const FuncInfo& info) {
        functions[name] = info;
    }

    bool has_function(const string& name) {
        return functions.find(name) != functions.end();
    }

    FuncInfo get_function(const string& name) {
        if (!has_function(name)) {
            cerr << "Error en tiempo de compilación: Función no declarada: " << name << endl;
            exit(1);
        }
        return functions[name];
    }
    
    void add_struct(const string& name, const StructInfo& info) {
        if (has_struct(name)) {
             cerr << "Error: Redefinición del struct '" << name << "'" << endl;
             exit(1);
        }
        structs[name] = info;
    }

    bool has_struct(const string& name) {
        return structs.find(name) != structs.end();
    }

    StructInfo get_struct(const string& name) {
        if (!has_struct(name)) {
            cerr << "Error: Uso de tipo struct no definido '" << name << "'" << endl;
            exit(1);
        }
        return structs[name];
    }
};

#endif