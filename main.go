package main

import (
	"fmt"
)

func main() {
	fmt.Print("DEMS Blockchain Started")
	bc := NewBlockchain()

	defer bc.db.Close()
	cli := CLI{bc}
	cli.Run()
}