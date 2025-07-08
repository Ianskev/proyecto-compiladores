# 🖥️ Compilador de Go a Ensamblador x86_64

<div align="center">

![Go](https://img.shields.io/badge/go-%2300ADD8.svg?style=for-the-badge&logo=go&logoColor=white)
![Assembly](https://img.shields.io/badge/Assembly-x86_64-red?style=for-the-badge)
![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Python](https://img.shields.io/badge/python-3670A0?style=for-the-badge&logo=python&logoColor=ffdd54)
![Streamlit](https://img.shields.io/badge/Streamlit-FF4B4B?style=for-the-badge&logo=streamlit&logoColor=white)

*Un compilador educativo que traduce un subconjunto del lenguaje Go a código ensamblador x86_64*

</div>

---

## 📋 Índice

- [Introducción](#introducción)
- [Instalación y Configuración](#instalación-y-configuración)
- [Ejecución del Proyecto](#ejecución-del-proyecto)
- [Estructura del Proyecto](#estructura-del-proyecto)
- [Fases del Compilador](#fases-del-compilador)
- [Características Soportadas](#características-soportadas)
- [Ejemplos de Uso](#ejemplos-de-uso)
- [Limitaciones](#limitaciones)
- [Desarrolladores](#desarrolladores)

---

## 🎯 Introducción

Este compilador traduce un subconjunto del lenguaje Go a código ensamblador x86_64, permitiendo la ejecución de programas Go simples en arquitecturas compatibles. El sistema realiza análisis léxico, sintáctico y genera código ensamblador optimizado para su posterior ensamblaje y ejecución.

## 🚀 Instalación y Configuración

### Prerrequisitos

- **C++ Compiler**: GCC o Clang
- **Python 3.7+**: Para el frontend web
- **Streamlit**: Para la interfaz web
- **Sistema operativo**: Linux, macOS o Windows con WSL

### Instalación

1. **Clonar el repositorio**:
```bash
git clone https://github.com/usuario/proyecto-compiladores.git
cd proyecto-compiladores
```

2. **Instalar dependencias de Python**:
```bash
pip install streamlit
```

3. **Compilar el backend**:
```bash
cd backend
g++ -o main *.cpp
cd ..
```

## 🎮 Ejecución del Proyecto

### Opción 1: Interfaz Web (Recomendada)

1. **Ejecutar la aplicación Streamlit**:
```bash
cd frontend
streamlit run frontend_app.py
```

2. **Acceder a la aplicación**:
   - **✅ Recomendado**: `http://127.0.0.1:8501`
   - **🆗 Alternativo**: `http://localhost:8501`
   - **❌ Evitar**: `http://0.0.0.0:8501` (puede causar problemas de red)

> **💡 Tip**: Si Streamlit muestra `http://0.0.0.0:8501` en la consola, simplemente cambia `0.0.0.0` por `127.0.0.1` en tu navegador.

### Opción 2: Línea de Comandos

1. **Compilar un programa Go**:
```bash
./backend/main archivo.go
```

2. **Ensamblar y enlazar**:
```bash
gcc -no-pie -o programa resultado/result.s
```

3. **Ejecutar**:
```bash
./programa
```

### Solución de Problemas Comunes

#### Problemas de Red
- **Síntoma**: La aplicación no carga o es lenta
- **Solución**: Usa `http://127.0.0.1:8501` en lugar de `http://0.0.0.0:8501`

#### Problemas de Permisos
- **Síntoma**: `[Errno 13] Permission denied`
- **Solución**: El sistema intentará solucionarlo automáticamente, o ejecuta `chmod +x backend/main`

### Opciones de Compilación

- **Solo generar ensamblador**: `./backend/main archivo.go -s`
- **Verbose**: `./backend/main archivo.go -v`

---

## 📁 Estructura del Proyecto

```
proyecto-compiladores/
├── 🎯 backend/                 # Motor del compilador
│   ├── environment.hh          # Entorno de variables y símbolos
│   ├── exp.cpp/.h              # Representación de expresiones del AST
│   ├── gencode.cpp/.h          # Generador de código ensamblador
│   ├── imp_value.cpp/.h        # Valores e información de tipos
│   ├── imp_value_visitor.h     # Interfaz para visitantes con valores
│   ├── main.cpp                # Punto de entrada del compilador
│   ├── parser.cpp/.h           # Analizador sintáctico
│   ├── scanner.cpp/.h          # Analizador léxico
│   ├── token.cpp/.h            # Definiciones de tokens
│   ├── visitor.cpp/.h          # Sistema de visitantes para el AST
│   ├── outputs/                # Código ensamblador generado
│   └── tests/                  # Programas Go de prueba
├── 🌐 frontend/                # Interfaz web
│   ├── frontend_app.py         # Aplicación principal Streamlit
│   └── executer.py             # Ejecutor del compilador
├── 📄 resultado/               # Archivos de salida
└── 📚 README.md               # Este archivo
```

---

## ⚙️ Fases del Compilador

### 🔍 1. Análisis Léxico (Scanner)
- **Función**: Divide el código fuente en tokens
- **Implementación**: `scanner.cpp` y `scanner.h`
- **Reconoce**: Identificadores, palabras reservadas, operadores, literales

### 🌳 2. Análisis Sintáctico (Parser)
- **Función**: Construye un Árbol de Sintaxis Abstracta (AST)
- **Implementación**: `parser.cpp` y `parser.h`
- **Valida**: Estructura gramatical del programa Go

### 🔧 3. Generación de Código
- **Función**: Traduce el AST a instrucciones ensamblador x86_64
- **Proceso**: Sistema de dos pasadas
  1. **Primera pasada**: Calcula offsets de variables en la pila
  2. **Segunda pasada**: Genera el código ensamblador real
- **Gestiona**: Registros, la pila, y llamadas a funciones
- **Implementación**: `gencode.cpp` y `gencode.h`

---

## ✨ Características Soportadas

### 📊 Tipos de Datos
- 🔢 **Enteros** (`int`)
- 🔤 **Cadenas** (`string`)
- ✅ **Booleanos** (`bool`)
- 🏗️ **Estructuras** básicas

### 🧮 Expresiones
- ➕ **Aritméticos**: `+`, `-`, `*`, `/`, `%`
- 🔗 **Lógicos**: `&&`, `||`, `!`
- 🔍 **Comparación**: `==`, `!=`, `<`, `>`, `<=`, `>=`
- 1️⃣ **Unarios**: `+`, `-`

### 📝 Sentencias
- 📦 **Declaración de variables** (`var` y `:=`)
- 📝 **Asignación** (`=`, `+=`, `-=`, etc.)
- 🔀 **Condicionales** (`if`, `else`)
- 🔄 **Bucles** (`for`)
- 🔧 **Funciones** (`func`)
- 🔙 **Retorno** (`return`)

### 📚 Biblioteca Estándar
- 🖨️ **fmt.Println()** básico

---

## 💡 Ejemplos de Uso

### 🌟 Ejemplo 1: Hello World
```go
package main

import "fmt"

func main() {
    fmt.Println("Hello, World!")
}
```

### 🧮 Ejemplo 2: Operaciones Aritméticas
```go
package main

import "fmt"

func main() {
    x := 10
    y := 5
    result := x + y * 2
    fmt.Println(result)
}
```

### 🔀 Ejemplo 3: Condicionales
```go
package main

import "fmt"

func main() {
    x := -3
    if x >= 0 {
        fmt.Println("no negativo")
    } else {
        fmt.Println("negativo")
    }
}
```

### 🔄 Ejemplo 4: Bucles
```go
package main

import "fmt"

func main() {
    for i := 0; i < 5; i++ {
        fmt.Println(i)
    }
}
```

---

## 🚧 Limitaciones

- ❌ No arrays multidimensionales
- ❌ Soporte limitado para punteros
- ❌ Sin verificación de tipos completa
- ❌ Sin recolección de basura
- ❌ Biblioteca estándar limitada
- ❌ Sin goroutines ni canales
- ❌ Interfaces incompletas

---

## 👨‍💻 Desarrolladores

<div align="center">

**Proyecto desarrollado como parte del curso de Compiladores**  
**Universidad de Ingeniería y Tecnología (UTEC)**

*Para fines educativos y de investigación*

</div>

---

<div align="center">

Made with ❤️ by UTEC Students

</div>