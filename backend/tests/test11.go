package main

import "fmt"

func main() {
	age := 20
	registered := true
	if age >= 18 && registered {
		fmt.Println("acceso permitido")
	} else {
		fmt.Println("acceso denegado")
	}
}