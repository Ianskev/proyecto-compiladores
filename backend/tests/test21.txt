package main

import "fmt"

func main() {
    s1 := "Hola"
    s2 := "Mundo"
    saludo := s1 + ", " + s2 + "!"
    fmt.Println(saludo)                      // → Hola, Mundo!
    fmt.Println(saludo == "Hola, Mundo!")    // → true
}
