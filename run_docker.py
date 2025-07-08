import os
import subprocess
import platform
import sys

def check_docker():
    """Verifica si Docker está instalado y funcionando"""
    try:
        subprocess.run(["docker", "--version"], check=True, stdout=subprocess.PIPE)
        return True
    except (subprocess.SubprocessError, FileNotFoundError):
        return False

def build_docker_image():
    """Construye la imagen Docker"""
    print("Construyendo imagen Docker (esto puede tardar unos minutos)...")
    result = subprocess.run(
        ["docker", "build", "-t", "go-compiler:latest", "."],
        check=False
    )
    return result.returncode == 0

def run_docker_container():
    """Ejecuta el contenedor Docker con la aplicación"""
    # Determinar el directorio actual
    current_dir = os.path.dirname(os.path.abspath(__file__))
    
    print("Iniciando contenedor Docker...")
    
    # Mapear el puerto 8501 (Streamlit) y montar el directorio actual
    cmd = [
        "docker", "run", "--rm", "-it", 
        "-p", "8501:8501", 
        "-v", f"{current_dir}:/app",
        "--name", "go-compiler-container",
        "go-compiler:latest"
    ]
    
    try:
        subprocess.run(cmd, check=True)
        return True
    except subprocess.SubprocessError as e:
        print(f"Error al ejecutar el contenedor: {e}")
        return False

if __name__ == "__main__":
    # Verificar si Docker está instalado
    if not check_docker():
        print("Error: Docker no está instalado o no está en funcionamiento.")
        print("Por favor, instala Docker Desktop y asegúrate de que esté en ejecución.")
        sys.exit(1)
    
    # Construir la imagen Docker si no existe o si se solicita reconstruir
    rebuild = "--rebuild" in sys.argv
    
    try:
        subprocess.run(["docker", "image", "inspect", "go-compiler:latest"], 
                      check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if rebuild:
            print("Reconstruyendo imagen Docker...")
            if not build_docker_image():
                print("Error al construir la imagen Docker.")
                sys.exit(1)
        else:
            print("Imagen Docker ya existe. Usando imagen existente.")
    except subprocess.CalledProcessError:
        if not build_docker_image():
            print("Error al construir la imagen Docker.")
            sys.exit(1)
    
    # Ejecutar el contenedor Docker
    print("\nIniciando aplicación en Docker...")
    print("Una vez iniciado, accede a: http://localhost:8501")
    
    if not run_docker_container():
        print("Error al ejecutar el contenedor Docker.")
        sys.exit(1)
