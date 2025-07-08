package main

import "fmt"

type Persona struct {
    nombre string
    edad   int
}
func main() {
    p1 := Persona{"Ian", 19}
    fmt.Println("Me llamo Ian y tengo: ", p1.edad, " años")
}