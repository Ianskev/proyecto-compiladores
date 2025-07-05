package main

import "fmt"

func repetir(s string, veces int) string {
    r := ""
    for i := 0; i < veces; i++ {
        r += s
    }
    return r
}

func main() {
    fmt.Println(repetir("hola", 4))
}