package main

import (
	"fmt"
	"strconv"
)

func main() {
	bc := NewBlockchain()

	defer bc.db.Close()
	cli := CLI{bc}
	cli.Run()
}