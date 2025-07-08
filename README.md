## Introducción

Este compilador traduce un subconjunto del lenguaje Go a código ensamblador x86_64, permitiendo la ejecución de programas Go simples en arquitecturas compatibles. El sistema realiza análisis léxico, sintáctico y genera código ensamblador optimizado para su posterior ensamblaje y ejecución.

## Estructura del Proyecto

El compilador está organizado en los siguientes componentes principales:

```
c:\Users\Ian\Desktop\UTEC\CICLO 5\COMPILADORES\test\
├── backend/
│   ├── environment.hh       # Entorno de variables y símbolos
│   ├── exp.cpp/.h           # Representación de expresiones del AST
│   ├── gencode.cpp/.h       # Generador de código ensamblador
│   ├── imp_value.cpp/.h     # Valores e información de tipos
│   ├── imp_value_visitor.h  # Interfaz para visitantes con valores de retorno
│   ├── main.cpp             # Punto de entrada del compilador
│   ├── parser.cpp/.h        # Analizador sintáctico
│   ├── scanner.cpp/.h       # Analizador léxico
│   ├── token.cpp/.h         # Definiciones de tokens
│   ├── visitor.cpp/.h       # Sistema de visitantes para el AST
│   ├── outputs/             # Código ensamblador generado
│   └── tests/               # Programas Go de prueba
└── README.md                # Este archivo
```

## Fases del Compilador

### 1. Análisis Léxico (Scanner)
- Divide el código fuente en tokens (identificadores, palabras reservadas, operadores, etc.)
- Implementado en `scanner.cpp` y `scanner.h`
- Reconoce todos los elementos léxicos de Go soportados

### 2. Análisis Sintáctico (Parser)
- Construye un Árbol de Sintaxis Abstracta (AST) a partir de los tokens
- Implementado en `parser.cpp` y `parser.h`
- Valida la estructura gramatical del programa Go

### 3. Generación de Código
- Traduce el AST a instrucciones ensamblador x86_64
- Utiliza un sistema de dos pasadas:
  1. Primera pasada: Calcula offsets de variables en la pila
  2. Segunda pasada: Genera el código ensamblador real
- Gestiona registros, la pila, y llamadas a funciones
- Implementado en `gencode.cpp` y `gencode.h`

## Características Soportadas

### Tipos de Datos
- Enteros (`int`)
- Cadenas (`string`)
- Booleanos (`bool`)
- Estructuras básicas

### Expresiones
- Operadores aritméticos: `+`, `-`, `*`, `/`, `%`
- Operadores lógicos: `&&`, `||`, `!`
- Operadores de comparación: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Operadores unarios: `+`, `-`

### Sentencias
- Declaración de variables (`var` y `:=`)
- Asignación (`=`, `+=`, `-=`, etc.)
- Condicionales (`if`, `else`)
- Bucles (`for`)
- Funciones (`func`)
- Retorno (`return`)

### Biblioteca Estándar
- Soporte básico para `fmt.Println()`

## Uso del Compilador

### Compilación y Ejecución

1. **Compilar el compilador:**

```bash
g++ -o goc backend/*.cpp
```

2. **Compilar un programa Go:**

```bash
./goc ruta/al/programa.go
# Genera ruta/al/programa.s (código ensamblador)
```

3. **Ensamblar y enlazar:**

```bash
gcc -no-pie -o programa programa.s
```

4. **Ejecutar:**

```bash
./programa
```

### Opciones

- `-s`: Solo genera código ensamblador sin imprimir información adicional

## Ejemplos

### Ejemplo 1: Hello World con variables
```go
package main

import "fmt"

func main() {
    x := 10
    fmt.Println(x)
}
```

### Ejemplo 2: Operaciones aritméticas
```go
package main

import "fmt"

func main() {
    result := 1 + 2*3 - 4/2
    fmt.Println(result)
}
```

### Ejemplo 3: Condicionales
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

### Ejemplo 4: Bucles
```go
package main

import "fmt"

func main() {
    n := 0
    for n < 3 {
        fmt.Println(n)
        n++
    }
}
```

## Implementación de la Generación de Código

El generador de código sigue estos pasos para crear código ensamblador eficiente:

1. **Recolección de información preliminar**:
   - Recorre el AST para recopilar literales de cadena y definiciones de funciones
   - Calcula el tamaño necesario para el stack de cada función

2. **Generación de prólogo y epílogo**:
   - Establece correctamente el marco de pila para cada función
   - Garantiza la alineación de 16 bytes requerida por la ABI de x86_64

3. **Gestión de variables**:
   - Asigna espacio en la pila para cada variable local
   - Mantiene un registro de los offsets de cada variable

4. **Optimizaciones**:
   - Usa registros de manera eficiente para operaciones aritméticas
   - Implementa evaluación de cortocircuito para operaciones lógicas

## Limitaciones Actuales

- No se admiten arrays multidimensionales
- Soporte limitado para punteros y referencias
- No hay verificación de tipos completa
- No se implementa recolección de basura
- Soporte limitado para la biblioteca estándar
- No se admiten goroutines ni canales
- No se implementa interfaz completa para estructuras

## Desarrolladores

Este compilador fue desarrollado como proyecto del curso de Compiladores en UTEC.

## Licencia

Este proyecto es para fines educativos y de investigación.

---

## Próximos Pasos

- Implementar verificación de tipos más robusta
- Ampliar el soporte para la biblioteca estándar
- Optimización de código generado
- Soporte para más características avanzadas de Go

## Ejemplos de Código Generado

### Ejemplo 1: Variables Enteras

#### Código Go de Entrada
```go
package main

import "fmt"

func main() {
    x := 10
    fmt.Println(x)
}
```

#### Código Ensamblador Generado
```assembly
.data
print_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"
print_bool_true: .string "true\n"
print_bool_false: .string "false\n"
.text
.globl main
main:
  pushq %rbp
  movq %rsp, %rbp
  subq $16, %rsp  # Reservar espacio para 1 variable
  movq $10, %rax
  movq %rax, -8(%rbp)  # Inicializar x
  movq -8(%rbp), %rax  # Cargar x
  leaq print_fmt(%rip), %rdi
  movq %rax, %rsi
  xorq %rax, %rax
  call printf@PLT
  movq $0, %rax
  leave
  ret
.section .note.GNU-stack,"",@progbits
```

### Ejemplo 2: Variables de Cadena

#### Código Go de Entrada
```go
package main

import "fmt"

func main() {
    a, b, c := "foo", "bar", "baz"
    fmt.Println(a, b, c)
}
```

#### Código Ensamblador Generado
```assembly
.data
print_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"
print_bool_true: .string "true\n"
print_bool_false: .string "false\n"
string_2: .string "baz"
string_1: .string "bar"
string_0: .string "foo"
.text
.globl main
main:
  pushq %rbp
  movq %rsp, %rbp
  subq $32, %rsp  # Reservar espacio para 3 variables
  leaq string_0(%rip), %rax
  movq %rax, -8(%rbp)  # Inicializar a
  leaq string_1(%rip), %rax
  movq %rax, -16(%rbp)  # Inicializar b
  leaq string_2(%rip), %rax
  movq %rax, -24(%rbp)  # Inicializar c
  movq -8(%rbp), %rax  # Cargar a
  leaq print_str_fmt(%rip), %rdi
  movq %rax, %rsi
  xorq %rax, %rax
  call printf@PLT
  movq -16(%rbp), %rax  # Cargar b
  leaq print_str_fmt(%rip), %rdi
  movq %rax, %rsi
  xorq %rax, %rax
  call printf@PLT
  movq -24(%rbp), %rax  # Cargar c
  leaq print_str_fmt(%rip), %rdi
  movq %rax, %rsi
  xorq %rax, %rax
  call printf@PLT
  movq $0, %rax
  leave
  ret
.section .note.GNU-stack,"",@progbits
```