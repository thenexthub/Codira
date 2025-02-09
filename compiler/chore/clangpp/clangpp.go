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
	"fmt"
	"os"

	"github.com/goplus/llgo/xtool/clang/preprocessor"
)

func usage() {
	fmt.Fprintf(os.Stderr, "Usage: clangpp source.c\n")
}

func main() {
	if len(os.Args) < 2 {
		usage()
		return
	}
	infile := os.Args[1]
	outfile := infile + ".i"
	if err := preprocessor.Do(infile, outfile, nil); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}
