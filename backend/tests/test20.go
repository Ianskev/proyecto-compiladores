package main

import "fmt"

type Rectangulo struct {
	ancho int
	alto int
}

type Triangulo struct {
	base int
	altura int
}

func main() {
	r:= Rectangulo{5, 4}
	t:= Triangulo{6, 3}

	if r.ancho > t.altura {
		fmt.Println("El ancho de el rectangulo es mayor que la altura del triangulo")
	} else if r.alto < t.altura {
		fmt.Println("La altura del rectangulo es menor que la altura del triangulo")
	} else {
		fmt.Println("viva compiladores")
	}
}
