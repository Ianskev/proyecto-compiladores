package main

import "fmt"

func main() {
	userInput := "GoLang"
	if userInput == "golang" {
		fmt.Println("iguales (sensible a mayúsculas)")
	} else if userInput == "GoLang" {
		fmt.Println("iguales exactas")
	} else {
		fmt.Println("diferentes")
	}
}