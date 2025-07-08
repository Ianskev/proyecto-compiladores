#ifndef GO_ENVIRONMENT_H
#define GO_ENVIRONMENT_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "imp_value.h"

using namespace std;

// Información simple de una variable
struct VarInfo {
    int offset;
    ImpVType type;
};

class Environment {
private:
    // Lista de "niveles" de scope. Cada nivel es un mapa de nombre de var a su info.
    vector<unordered_map<string, VarInfo>> levels;

    // Busca una variable desde el scope actual hacia afuera.
    int search_rib(const string& var) {
        int idx = levels.size() - 1;
        while (idx >= 0) {
            if (levels[idx].find(var) != levels[idx].end()) {
                return idx;
            }
            idx--;
        }
        return -1;
    }

public:
    Environment() {}

    void clear() {
        levels.clear();
    }

    // Abre un nuevo scope (ej. al entrar a un bloque)
    void add_level() {
        levels.push_back({});
    }

    // Cierra el scope actual
    bool remove_level() {
        if (!levels.empty()) {
            levels.pop_back();
            return true;
        }
        return false;
    }

    // Agrega una variable al scope actual
    void add_var(const string& var, int offset, ImpVType type) {
        if (levels.empty()) {
            cout << "Environment sin niveles: no se pueden agregar variables" << endl;
            exit(1);
        }
        levels.back()[var] = {offset, type};
    }
    
    // Verifica si una variable existe en algún scope
    bool check(const string& x) {
        return search_rib(x) >= 0;
    }

    // Obtiene la información de una variable
    VarInfo lookup(const string& x) {
        int idx = search_rib(x);
        if (idx < 0) {
            cout << "Variable no declarada: " << x << endl;
            exit(1);
        }
        return levels[idx][x];
    }
};

#endif // GO_ENVIRONMENT_H