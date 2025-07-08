import os
import subprocess
import tempfile
import sys
import stat
from pathlib import Path

def ensure_dir_exists(directory):
    """Asegura que el directorio exista, creándolo si es necesario."""
    if not os.path.exists(directory):
        os.makedirs(directory)
        print(f"📁 Directorio creado: {directory}")

def is_running_in_docker():
    """Detecta si el código está ejecutándose dentro de Docker."""
    return os.path.exists('/.dockerenv')

def make_executable(filepath):
    """Hace que un archivo sea ejecutable."""
    try:
        current_permissions = os.stat(filepath).st_mode
        os.chmod(filepath, current_permissions | stat.S_IEXEC)
        return True
    except Exception as e:
        print(f"Error al hacer ejecutable {filepath}: {e}")
        return False

def compile_go_code(go_code, output_dir="resultado", output_name="result.s"):
    """
    Compila el código Go proporcionado y guarda el resultado como archivo .s
    
    Args:
        go_code (str): Código fuente en Go
        output_dir (str): Directorio donde guardar el resultado
        output_name (str): Nombre del archivo de salida
        
    Returns:
        tuple: (éxito (bool), mensaje (str), ruta_del_archivo (str))
    """
    # Asegurar que el directorio de salida exista
    ensure_dir_exists(output_dir)
    output_path = os.path.join(output_dir, output_name)
    
    # Crear un archivo temporal para el código Go
    with tempfile.NamedTemporaryFile(suffix='.go', delete=False, mode='w') as temp_file:
        temp_file.write(go_code)
        temp_file_path = temp_file.name
    
    try:
        # Compilar el código Go en modo ensamblador-only
        if os.name == 'nt' and not is_running_in_docker():  # Windows (no Docker)
            exec_path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "backend", "main.exe")
            if not os.path.exists(exec_path):
                # Intentar compilar el compilador si no existe
                backend_dir = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "backend")
                os.chdir(backend_dir)
                subprocess.run(["g++", "-o", "main.exe"] + [f for f in os.listdir() if f.endswith('.cpp')], 
                              check=True)
                os.chdir('../frontend')
                exec_path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "backend", "main.exe")
            
            result = subprocess.run([exec_path, temp_file_path, "-s"], 
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE, 
                                   text=True)
        else:  # Linux/Mac o Docker
            exec_path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "backend", "main")
            
            # Verificar si el archivo existe en Docker
            if is_running_in_docker():
                exec_path = "/app/backend/main"
            
            if not os.path.exists(exec_path):
                # Intentar compilar el compilador si no existe
                backend_dir = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "backend")
                if is_running_in_docker():
                    backend_dir = "/app/backend"
                
                os.chdir(backend_dir)
                subprocess.run(["g++", "-o", "main"] + [f for f in os.listdir() if f.endswith('.cpp')], 
                              check=True)
                
                # Hacer el archivo ejecutable
                if not make_executable(os.path.join(backend_dir, "main")):
                    return False, "Error: No se pudo hacer ejecutable el compilador", None
                
                if not is_running_in_docker():
                    os.chdir('../frontend')
                
                exec_path = os.path.join(backend_dir, "main")
            
            # Verificar y establecer permisos de ejecución
            if not make_executable(exec_path):
                return False, "Error: No se pudo establecer permisos de ejecución", None
                
            result = subprocess.run([exec_path, temp_file_path, "-s"], 
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE, 
                                   text=True)
        
        # Guardar la salida en el archivo de resultado
        with open(output_path, 'w') as f:
            f.write(result.stdout)
        
        if result.returncode == 0:
            return True, f"Compilación exitosa. Resultado guardado en: {output_path}", output_path
        else:
            error_msg = result.stderr if result.stderr else "Error desconocido durante la compilación"
            return False, f"Error en la compilación: {error_msg}", None
            
    except Exception as e:
        return False, f"Error: {str(e)}", None
    finally:
        # Eliminar el archivo temporal
        try:
            os.unlink(temp_file_path)
        except:
            pass

def compile_assembly_to_executable(assembly_file, executable_name="programa"):
    """
    Compila el archivo ensamblador .s a un ejecutable usando GCC
    
    Args:
        assembly_file (str): Ruta al archivo ensamblador
        executable_name (str): Nombre del ejecutable a generar
    
    Returns:
        tuple: (éxito (bool), mensaje (str), ruta_del_ejecutable (str))
    """
    try:
        output_dir = os.path.dirname(assembly_file)
        executable_path = os.path.join(output_dir, executable_name)
        
        if os.name == 'nt' and not is_running_in_docker():  # Windows (no Docker)
            executable_path += '.exe'
        
        # Compilar el ensamblador a un ejecutable usando GCC
        result = subprocess.run(
            ["gcc", "-no-pie", "-o", executable_path, assembly_file],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        if result.returncode == 0:
            # Hacer el archivo ejecutable
            if not make_executable(executable_path):
                return False, "Error: No se pudo hacer ejecutable el programa", None
            
            return True, f"Compilación exitosa. Ejecutable creado en: {executable_path}", executable_path
        else:
            error_msg = result.stderr if result.stderr else "Error desconocido durante la compilación"
            return False, f"Error en la compilación del ejecutable: {error_msg}", None
    
    except Exception as e:
        return False, f"Error al compilar a ejecutable: {str(e)}", None

def run_executable(executable_path):
    """
    Ejecuta el programa compilado y captura su salida
    
    Args:
        executable_path (str): Ruta al archivo ejecutable
    
    Returns:
        tuple: (éxito (bool), salida (str), error (str))
    """
    try:
        result = subprocess.run(
            [executable_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        return (
            result.returncode == 0,
            result.stdout,
            result.stderr if result.returncode != 0 else ""
        )
    
    except Exception as e:
        return False, "", f"Error al ejecutar el programa: {str(e)}"

def load_test_examples():
    """Carga los ejemplos de prueba desde la carpeta tests."""
    test_dir = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "backend", "tests")
    examples = {}
    
    if not os.path.exists(test_dir):
        return examples
    
    for file in os.listdir(test_dir):
        if file.endswith('.go'):
            try:
                with open(os.path.join(test_dir, file), 'r') as f:
                    content = f.read()
                    examples[file] = content
            except:
                pass
    
    return examples

if __name__ == "__main__":
    # Modo de ejecución directa para pruebas
    if len(sys.argv) > 1:
        # Si se proporciona un archivo como argumento
        with open(sys.argv[1], 'r') as f:
            code = f.read()
    else:
        # Código de ejemplo
        code = """
package main

import "fmt"

func main() {
    fmt.Println("Hello, World!")
}
"""
    
    success, message, asm_path = compile_go_code(code)
    print(message)
    
    if success and asm_path:
        success, message, exe_path = compile_assembly_to_executable(asm_path)
        print(message)
        
        if success and exe_path:
            success, output, error = run_executable(exe_path)
            if success:
                print("Ejecución exitosa:")
                print(output)
            else:
                print(f"Error en la ejecución: {error}")
