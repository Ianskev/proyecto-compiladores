package main

import "fmt"

func mayor(a int, b int) int {
	if a > b {
		return a
	}
	return b
}

func main() {
	fmt.Println(mayor(4, 7))
}
