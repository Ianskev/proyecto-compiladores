package main

import "fmt"

func main() {
    s := "hola"
    for i := 0; i < len(s); i++ {
        fmt.Println(string(s[i]))
    }
}