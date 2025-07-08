package main

import "fmt"

type Rectangulo struct {
	ancho int,
	alto int
}

type Triangulo struct {
	base int,
	altura int
}

func main() {
	r := Rectangulo{5, 4}
	t := Triangulo{5, 4}

	resultado := r.ancho*r.alto + (t.base*t.altura)/2

	fmt.Println("Area del Rectangulo + Area del triangulo:", resultado)
}