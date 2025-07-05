package main

import "fmt"

func main() {
    s := "go"
    rep := ""
    for i := 0; i < 3; i++ {
        rep += s
    }
    fmt.Println(rep)
}
