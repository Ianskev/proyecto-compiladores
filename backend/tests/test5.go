package main

import "fmt"

func main() {
    flag := true || (false && !false)
    fmt.Println(flag)
}