#!/usr/bin/env python3
import subprocess
import os
import platform
from pathlib import Path

system = platform.system()
print(f"Sistema operativo detectado: {system}")

# Carpeta de tests
tests_dir = "tests"

# Carpeta de salida para archivos .s
outputs_dir = "outputs"

# Archivos fuente del compilador
source_files = [
    "main.cpp", "scanner.cpp", "token.cpp", 
    "parser.cpp", "exp.cpp", "visitor.cpp", 
    "gencode.cpp", "imp_value.cpp"
]

if system == "Windows":
    executable = "main.exe"
else:
    executable = "main"

# Compilar el compilador de Go
print("Compilando el compilador de Go...")
compile_cmd = ["g++", "-o", executable] + source_files + ["-std=c++11"]
result = subprocess.run(compile_cmd)

if result.returncode != 0:
    print("Error de compilación del compilador.")
    exit(1)

print("Compilación exitosa del compilador.\n")

# Crear carpeta de salida si no existe
if not os.path.exists(outputs_dir):
    os.makedirs(outputs_dir)
    print(f"Carpeta {outputs_dir} creada.")

# Verificar que existe la carpeta de tests
if not os.path.exists(tests_dir):
    print(f"La carpeta {tests_dir} no existe.")
    exit(1)

# Obtener todos los archivos .go en la carpeta tests
test_files = []
for file in os.listdir(tests_dir):
    if file.endswith('.go'):
        test_files.append(file)

# Ordenar los archivos por número
test_files.sort(key=lambda x: int(''.join(filter(str.isdigit, x))))

print(f"Encontrados {len(test_files)} archivos de test:")
for test_file in test_files:
    print(f"  - {test_file}")

print("\n" + "="*50)
print("Generando archivos .s para cada test...")
print("="*50)

successful_generations = 0
failed_generations = 0

for test_file in test_files:
    input_file = os.path.join(tests_dir, test_file)
    output_file = os.path.join(outputs_dir, test_file.replace('.go', '.s'))
    
    print(f"\n🔨 Procesando {test_file}...")
    
    try:
        # Ejecutar el compilador en modo assembly-only y redirigir a archivo .s
        with open(output_file, 'w') as out_file:
            if system == "Windows":
                result = subprocess.run([f".\\{executable}", input_file, "-s"], 
                                      stdout=out_file, stderr=subprocess.PIPE, text=True)
            else:
                result = subprocess.run([f"./{executable}", input_file, "-s"], 
                                      stdout=out_file, stderr=subprocess.PIPE, text=True)
        
        if result.returncode == 0:
            print(f"✅ {output_file} generado exitosamente")
            successful_generations += 1
        else:
            print(f"❌ Error generando {output_file}")
            if result.stderr:
                print(f"   Error: {result.stderr}")
            failed_generations += 1
            # Eliminar archivo de salida si hubo error
            if os.path.exists(output_file):
                os.remove(output_file)
    
    except Exception as e:
        print(f"❌ Excepción procesando {test_file}: {e}")
        failed_generations += 1
        # Eliminar archivo de salida si hubo error
        if os.path.exists(output_file):
            os.remove(output_file)

print("\n" + "="*50)
print("RESUMEN DE GENERACIÓN")
print("="*50)
print(f"Total de tests procesados: {len(test_files)}")
print(f"Archivos .s generados exitosamente: {successful_generations}")
print(f"Fallos en generación: {failed_generations}")
print(f"Porcentaje de éxito: {(successful_generations/len(test_files)*100):.1f}%")

if successful_generations > 0:
    print(f"\n✅ Archivos .s generados:")
    for test_file in test_files:
        output_file = os.path.join(outputs_dir, test_file.replace('.go', '.s'))
        if os.path.exists(output_file):
            print(f"   - {output_file}")

print("\n🎉 Proceso completado!")
