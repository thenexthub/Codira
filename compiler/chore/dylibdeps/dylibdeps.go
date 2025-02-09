/*
 * Copyright (c) NeXTHub Corporation. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 */

package main

import (
	"debug/macho"
	"fmt"
	"os"
)

func main() {
	if len(os.Args) != 2 {
		fmt.Fprintln(os.Stderr, "Usage: dylibdeps exefile")
		return
	}
	exe := os.Args[1]

	file, err := macho.Open(exe)
	check(err)

	defer file.Close()
	for _, load := range file.Loads {
		if dylib, ok := load.(*macho.Dylib); ok {
			fmt.Println(dylib.Name)
		}
	}
}

func check(err error) {
	if err != nil {
		panic(err)
	}
}
