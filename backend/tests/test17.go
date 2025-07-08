package main

import "fmt"

type Persona struct {
	nombre string
	edad   int
}

func main() {
	p1 := Persona{"Ian", 19}

	fmt.Println("me llamo " + p1.nombre + " mi nombre tiene", len(p1.nombre), "letras")
}