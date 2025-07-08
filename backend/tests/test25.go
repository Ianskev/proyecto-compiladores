package main

import "fmt"

func main() {
	s := "hola"
	fmt.Println("Length:", len(s))

	for i := 0; i < len(s); i++ {
		fmt.Println(i)
	}
}