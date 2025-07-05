#!/usr/bin/env python3
import os
import subprocess
import sys
from pathlib import Path
import time

class CompilerTester:
    def __init__(self):
        self.backend_dir = Path("backend")
        self.tests_dir = self.backend_dir / "tests"
        self.executable = self.backend_dir / "parser_test.exe"
        self.successful_tests = []
        self.failed_tests = []
        
    def compile_backend(self):
        """Compile the backend Go compiler"""
        print("🔨 Compilando el compilador de Go...")
        try:
            os.chdir(self.backend_dir)
            result = subprocess.run([
                "g++", "-o", "parser_test.exe", 
                "main.cpp", "scanner.cpp", "token.cpp", 
                "parser.cpp", "exp.cpp", "visitor.cpp", 
                "-std=c++11"
            ], capture_output=True, text=True, timeout=30)
            
            if result.returncode == 0:
                print("✅ Compilador compilado exitosamente")
                return True
            else:
                print("❌ Error compilando el compilador:")
                print(result.stderr)
                return False
        except subprocess.TimeoutExpired:
            print("❌ Timeout compilando el compilador")
            return False
        except Exception as e:
            print(f"❌ Error: {e}")
            return False
        finally:
            os.chdir("..")
    
    def test_single_file(self, test_file):
        """Test a single Go file with the compiler"""
        try:
            result = subprocess.run([
                str(self.executable), str(test_file)
            ], capture_output=True, text=True, timeout=10)
            
            # Considera exitoso si no hay error de parsing
            success = "Parser exitoso" in result.stdout and "Error en el parser" not in result.stdout
            
            return {
                'success': success,
                'stdout': result.stdout,
                'stderr': result.stderr,
                'returncode': result.returncode
            }
        except subprocess.TimeoutExpired:
            return {
                'success': False,
                'stdout': '',
                'stderr': 'Timeout',
                'returncode': -1
            }
        except Exception as e:
            return {
                'success': False,
                'stdout': '',
                'stderr': str(e),
                'returncode': -1
            }
    
    def run_tests(self):
        """Run all tests and collect results"""
        if not self.compile_backend():
            print("❌ No se pudo compilar el compilador. Abortando tests.")
            return
        
        if not self.executable.exists():
            print(f"❌ Ejecutable no encontrado: {self.executable}")
            return
        
        # Obtener todos los archivos .go en el directorio de tests
        test_files = list(self.tests_dir.glob("test*.go"))
        test_files.sort()
        
        if not test_files:
            print("❌ No se encontraron archivos de test")
            return
        
        print(f"\n🧪 Ejecutando {len(test_files)} tests...\n")
        
        for test_file in test_files:
            print(f"🔍 Probando {test_file.name}...", end=" ")
            
            result = self.test_single_file(test_file)
            
            if result['success']:
                print("✅ EXITOSO")
                self.successful_tests.append(test_file.name)
            else:
                print("❌ FALLO")
                self.failed_tests.append({
                    'name': test_file.name,
                    'error': result.get('stderr', 'Error desconocido'),
                    'output': result.get('stdout', '')
                })
    
    def show_results(self):
        """Display test results summary"""
        total = len(self.successful_tests) + len(self.failed_tests)
        
        print(f"\n📊 RESUMEN DE RESULTADOS")
        print("=" * 50)
        print(f"Total de tests: {total}")
        print(f"Exitosos: {len(self.successful_tests)}")
        print(f"Fallidos: {len(self.failed_tests)}")
        if total > 0:
            print(f"Porcentaje de éxito: {len(self.successful_tests)/total*100:.1f}%")
        else:
            print("No se pudieron ejecutar tests")
        
        if self.successful_tests:
            print(f"\n✅ TESTS EXITOSOS ({len(self.successful_tests)}):")
            for test in sorted(self.successful_tests):
                print(f"   - {test}")
        
        if self.failed_tests:
            print(f"\n❌ TESTS FALLIDOS ({len(self.failed_tests)}):")
            for test in sorted(self.failed_tests, key=lambda x: x['name']):
                print(f"   - {test['name']}")
        
        # Mostrar errores detallados de algunos tests fallidos
        if self.failed_tests:
            print(f"\n🔍 DETALLES DE ERRORES (primeros 5):")
            for test in sorted(self.failed_tests, key=lambda x: x['name'])[:5]:
                print(f"\n--- {test['name']} ---")
                if test['error']:
                    print(f"Error: {test['error'][:200]}...")
                if test['output']:
                    # Buscar líneas relevantes en la salida
                    lines = test['output'].split('\n')
                    for line in lines:
                        if 'error' in line.lower() or 'parse error' in line.lower():
                            print(f"Output: {line}")
                            break
    
    def show_test_details(self, test_name):
        """Show detailed output for a specific test"""
        test_file = self.tests_dir / test_name
        if not test_file.exists():
            print(f"❌ Test file {test_name} not found")
            return
        
        print(f"\n🔍 DETALLE DE {test_name}")
        print("=" * 50)
        
        # Mostrar contenido del archivo
        print("📄 CONTENIDO DEL ARCHIVO:")
        try:
            with open(test_file, 'r', encoding='utf-8') as f:
                content = f.read()
                print(content)
        except Exception as e:
            print(f"Error leyendo archivo: {e}")
        
        print("\n🧪 RESULTADO DE COMPILACION:")
        result = self.test_single_file(test_file)
        
        if result['success']:
            print("✅ EXITOSO")
        else:
            print("❌ FALLO")
        
        if result['stdout']:
            print("\nSALIDA:")
            print(result['stdout'])
        
        if result['stderr']:
            print("\nERRORES:")
            print(result['stderr'])

def main():
    tester = CompilerTester()
    
    if len(sys.argv) > 1:
        # Modo detallado para un test específico
        test_name = sys.argv[1]
        if not test_name.endswith('.go'):
            test_name += '.go'
        tester.show_test_details(test_name)
    else:
        # Modo normal - ejecutar todos los tests
        tester.run_tests()
        tester.show_results()

if __name__ == "__main__":
    main()
