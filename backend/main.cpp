#include <iostream>
#include <fstream>
#include <string>
#include "scanner.h"
// #include "parser.h"  // Comentado hasta que implementemos el parser
// #include "visitor.h" // Comentado hasta que implementemos el parser

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

    // Fase 1: Scanner
    Scanner scanner(input.c_str());
    test_scanner(&scanner);
    cout << "Scanner exitoso" << endl << endl;

    // TODO: Implementar Parser para Go
    // GoParser parser(&scanner); 
    // Program* program = parser.parse();
    
    // TODO: Implementar PrintVisitor para Go
    // PrintVisitor printVisitor;
    // printVisitor.print(program);
    
    cout << "Compilador terminado (solo scanner implementado por ahora)" << endl;
    
    return 0;
}
