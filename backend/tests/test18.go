package main

import "fmt"

type Triangulo struct {
    base int
    altura int
}

func main() {
    t := Triangulo{5, 4}
    fmt.Println(t.base)
}