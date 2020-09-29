package main

import (
	"fmt"
	"time"
)

func main() {
	for {
		fmt.Println("I am alive")
		time.Sleep(30 * time.Second)
	}
}
