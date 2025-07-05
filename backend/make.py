import os
import subprocess

source_files = [
    "**.cpp"
]

executable = "main"

test_dir = "tests"

print("🔧 Compilando...")
compile_cmd = ["g++", "-std=c++17", "-Wall"] + source_files + ["-o", executable]
result = subprocess.run(compile_cmd)

if result.returncode != 0:
    print("❌ Error de compilación.")
    exit(1)

print("✅ Compilación exitosa.\n")

test_files = []
for root, _, files in os.walk(test_dir):
    for file in files:
        if file.endswith(".txt"):
            test_files.append(os.path.join(root, file))

for test_file in test_files:
    print(f"🧪 Ejecutando prueba: {test_file}")
    test_dirname = os.path.dirname(test_file)
    test_name = os.path.splitext(os.path.basename(test_file))[0]
    output_path = os.path.join(test_dirname, f"{test_name}.s")

    with open(test_file, "r") as infile, open(output_path, "w") as outfile:
        result = subprocess.run(
        ["./" + executable, test_file],
        stdout=outfile
    )
        if result.returncode != 0:
            print(f"❌ Error al procesar {test_file}")

print("\n✅ Todos los tests fueron ejecutados y sus salidas se guardaron en el mismo directorio.")