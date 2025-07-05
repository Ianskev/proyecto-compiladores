package main

import "fmt"

func califica(score int) string {
	if score >= 90 {
		return "A"
	} else if score >= 80 {
		return "B"
	} else if score >= 70 {
		return "C"
	}
	return "D"
}

func main() {
	fmt.Println(califica(95))
	fmt.Println(califica(78))
}
