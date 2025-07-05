package main

import "fmt"

func main() {
	s1 := "Hola"
	fmt.Println(s1[0:3])

	for i := 0; i < len(s1[0:3]); i++ {
		fmt.Println(i)
	}
}