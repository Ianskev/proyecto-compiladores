package main

import "fmt"

type Cubo struct {
	lado int
}

type PrismaRectangular struct {
	ancho int
	largo int
	alto int
}

func main() {
	c := Cubo{lado: 3}
	p := PrismaRectangular{ancho: 2, largo: 3, alto: 4}

	fmt.Println(p.alto)
}
