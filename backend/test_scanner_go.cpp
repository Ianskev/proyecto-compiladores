#include <iostream>
#include <fstream>
#include <string>
#include "scanner.h"

using namespace std;

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        cout << "Uso: " << argv[0] << " <archivo_go>" << endl;
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

    cout << "=== TESTING GO SCANNER ===" << endl;
    cout << "Archivo: " << argv[1] << endl;
    cout << "Contenido:" << endl;
    cout << input << endl;
    cout << "=========================" << endl << endl;

    Scanner scanner(input.c_str());
    test_scanner(&scanner);

    return 0;
}
