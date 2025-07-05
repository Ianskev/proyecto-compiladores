package main

import "fmt"

type Rectangulo struct {
	ancho, alto int
}

type Triangulo struct {
	base, altura int
}

func areaRect(r Rectangulo) int {
	return r.ancho * r.alto
}

func areaTri(t Triangulo) int {
	return t.base * t.altura / 2
}

func comparaAreas(r Rectangulo, t Triangulo) {
	ar := areaRect(r)
	at := areaTri(t)

	if ar > at {
		fmt.Println("Area del rectangulo es", ar, "y es mayor que la del triangulo, que es", at)
	} else if ar < at {
		fmt.Println("Area del triangulo es", at, "y es mayor que la del rectangulo, que es", ar)
	} else {
		fmt.Println("Ambas areas son iguales:", ar)
	}
}

func main() {
	r := Rectangulo{5, 4}
	t := Triangulo{6, 3}
	comparaAreas(r, t)
}
