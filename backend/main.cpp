#include <iostream>
#include <fstream>
#include <string>
#include "scanner.h"
#include "parser.h"
#include "visitor.h"

using namespace std;

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        cout << "Numero incorrecto de argumentos. Uso: " << argv[0] << " <archivo_go>" << endl;
        exit(1);
    }

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

    // Fase 2: Parser
    cout << "=== PARSING ===" << endl;
    test_scanner.reset();
    GoParser parser(&test_scanner);
    Program* program = parser.parse();
    
    if (program) {
        cout << "Parser exitoso" << endl << endl;
        
        // Fase 3: Print AST
        cout << "=== AST ===" << endl;
        PrintVisitor printVisitor;
        printVisitor.print(program);
        
        // Clean up
        delete program;
    } else {
        cout << "Error en el parser" << endl;
    }
    
    cout << "Compilador terminado" << endl;
    
    return 0;
}
