#ifndef GO_ENVIRONMENT_H
#define GO_ENVIRONMENT_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include "imp_value.h"

using namespace std;

// Información de una variable
struct VarInfo {
    int offset;
    ImpVType type;
};

// Información de una función
struct FuncInfo {
    int stack_size; 
    ImpVType return_type; // <-- AÑADIDO: Para saber qué tipo devuelve la función
};

class Environment {
private:
    vector<unordered_map<string, VarInfo>> var_levels;
    unordered_map<string, FuncInfo> functions;

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

    void add_var(const string& var, int offset, ImpVType type) {
        if (var_levels.empty()) {
            cout << "Environment sin niveles: no se pueden agregar variables" << endl;
            exit(1);
        }
        var_levels.back()[var] = {offset, type};
    }
    
    bool check(const string& x) {
        return search_rib(x) >= 0;
    }

    VarInfo lookup(const string& x) {
        int idx = search_rib(x);
        if (idx < 0) {
            cout << "Variable no declarada: " << x << endl;
            exit(1);
        }
        return var_levels[idx][x];
    }

    // --- NUEVAS FUNCIONES PARA GESTIÓN DE FUNCIONES ---
    void add_function(const string& name, const FuncInfo& info) {
        functions[name] = info;
    }

    bool has_function(const string& name) {
        return functions.find(name) != functions.end();
    }

    FuncInfo get_function(const string& name) {
        if (!has_function(name)) {
            cout << "Función no declarada: " << name << endl;
            exit(1);
        }
        return functions[name];
    }
};

#endif // GO_ENVIRONMENT_H