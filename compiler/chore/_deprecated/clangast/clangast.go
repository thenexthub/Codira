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
	"encoding/json"
	"flag"
	"fmt"
	"os"

	"github.com/goplus/llgo/_xtool/clang/parser"
)

var (
	dump = flag.Bool("dump", false, "dump AST")
)

func usage() {
	fmt.Fprintf(os.Stderr, "Usage: clangast [-dump] source.i\n")
	flag.PrintDefaults()
}

func main() {
	flag.Usage = usage
	flag.Parse()
	if flag.NArg() < 1 {
		usage()
		return
	}
	var file = flag.Arg(0)
	var err error
	if *dump {
		doc, _, e := parser.DumpAST(file, nil)
		if e == nil {
			os.Stdout.Write(doc)
			return
		}
		err = e
	} else {
		doc, _, e := parser.ParseFile(file, 0)
		if e == nil {
			enc := json.NewEncoder(os.Stdout)
			enc.SetIndent("", "  ")
			check(enc.Encode(doc))
			return
		}
		err = e
	}
	fmt.Fprintln(os.Stderr, err)
	os.Exit(1)
}

func check(err error) {
	if err != nil {
		panic(err)
	}
}
