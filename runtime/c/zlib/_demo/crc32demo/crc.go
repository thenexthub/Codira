package main

import (
	"fmt"

	"language/llgo/c/zlib"
)

func main() {
	fmt.Printf("%08x\n", zlib.Crc32ZString(0, "Hello world"))
}
