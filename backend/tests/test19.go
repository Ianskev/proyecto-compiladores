package main

import "fmt"

type Cubo struct {
	lado int
}

type PrismaRectangular struct {
	ancho, largo, alto int
}

func main() {
	c := Cubo{lado: 3}
	p := PrismaRectangular{ancho: 2, largo: 3, alto: 4}

	volumenCubo := c.lado * c.lado * c.lado
	volumenPrisma := p.ancho * p.largo * p.alto
	totalVolumen := volumenCubo + volumenPrisma

	fmt.Println("Volumen del Cubo + Volumen del Prisma Rectangular:", totalVolumen)
}
