#include <iostream>
#include <fstream>
#include <string>
#include "scanner.h"
#include "parser.h"
#include "visitor.h"
#include "gencode.h"

using namespace std;

int main(int argc, const char* argv[]) {
    if (argc < 2 || argc > 3) {
        cout << "Uso: " << argv[0] << " <archivo_go> [-s para solo ensamblador]" << endl;
        exit(1);
    }

    bool assembly_only = (argc == 3 && string(argv[2]) == "-s");

    ifstream infile(argv[1]);
    if (!infile.is_open()) {
        cout << "No se pudo abrir el archivo: " << argv[1] << endl;
        exit(1);
    }

    string input;
    string line;
    while (getline(infile, line)) {
        input += line + '\n';
    }
    infile.close();

    if (!assembly_only) {
        cout << "=== COMPILADOR GO ===" << endl;
        cout << "Archivo: " << argv[1] << endl;
        cout << "Contenido:" << endl;
        cout << input << endl;
        cout << "===================" << endl << endl;

        // Fase 1: Scanner (for testing only)
        Scanner test_scanner(input.c_str());
        cout << "=== TOKENS ===" << endl;
        Token* token;
        while ((token = test_scanner.nextToken()) && token->type != Token::END) {
            cout << "Token: " << *token << endl;
        }
        cout << "Scanner exitoso" << endl << endl;
    }

    // Fase 2: Parser
    if (!assembly_only) {
        cout << "=== PARSING ===" << endl;
    }
    Scanner scanner(input.c_str());
    GoParser parser(&scanner);
    Program* program = parser.parse();
    
    if (program) {
        if (!assembly_only) {
            cout << "Parser exitoso" << endl << endl;
            
            // Fase 3: Print AST
            cout << "=== AST ===" << endl;
            PrintVisitor printVisitor;
            printVisitor.print(program);
            cout << endl;
            
            // Fase 4: Generar código ensamblador
            cout << "=== CODIGO ENSAMBLADOR ===" << endl;
        }
        
        GoCodeGen codeGen;
        codeGen.generateCode(program);
        
        // Clean up
        delete program;
    } else {
        if (!assembly_only) {
            cout << "Error en el parser" << endl;
        }
    }
    
    if (!assembly_only) {
        cout << "Compilador terminado" << endl;
    }
    
    return 0;
}
