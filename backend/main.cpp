#include <iostream>
#include <fstream>
#include <string>
#include "scanner.h"
#include "parser.h"
#include "visitor.h"
#include "imp_interpreter.hh"
#include "gencode.hh"
#include "imp_type.hh"

using namespace std;

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        cout << "Numero incorrecto de argumentos. Uso: " << argv[0] << " <archivo_de_entrada>" << endl;
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

    Scanner scanner(input.c_str());

    string input_copy = input;
    Scanner scanner_test(input_copy.c_str());
    test_scanner(&scanner_test);
    cout << "Scanner exitoso" << endl;
    cout << endl;
    
    cout << "Iniciando parsing:" << endl;
    Parser parser(&scanner); 
    try {
        // Detect if this is a Go program by checking for "package" keyword at start
        bool isGoProgram = false;
        Scanner detector(input.c_str());
        Token* first = detector.nextToken();
        if (first->type == Token::PACKAGE) {
            isGoProgram = true;
        }
        delete first;
        
        PrintVisitor printVisitor;
        ImpCODE codeGenerator;
        
        if (isGoProgram) {
            cout << "Detected Go program, parsing with Go grammar" << endl;
            GoProgram* program = parser.parseGoProgram();
            cout << "Parsing exitoso" << endl << endl;
            
            cout << "Iniciando Visitor:" << endl;
            cout << endl;
            cout << "IMPRIMIR:" << endl;
            printVisitor.imprimirGo(program);
            cout << endl;
            
            cout << endl << "Generating code:" << endl;
            codeGenerator.interpretGo(program);
            cout << "End of code generation" << endl;
            delete program;
        } else {
            Program* program = parser.parseProgram();
            cout << "Parsing exitoso" << endl << endl;
            
            cout << "Iniciando Visitor:" << endl;
            cout << endl;
            cout << "IMPRIMIR:" << endl;
            printVisitor.imprimir(program);
            cout << endl;
            
            cout << endl << "Generating code:" << endl;
            codeGenerator.interpret(program);
            cout << "End of code generation" << endl;
            delete program;
        }
    } catch (const exception& e) {
        cout << "Error durante la ejecución: " << e.what() << endl;
        return 1;
    }

    return 0;
}