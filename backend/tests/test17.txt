package main

import "fmt"

type Persona struct {
	nombre string
	edad   int
}

func Saludar(p Persona) string {
	return "Hola mi nombre es " + p.nombre
}
func main() {
	p1 := Persona{"Ian", 19}
	fmt.Println(Saludar(p1))
}