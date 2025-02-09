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
	"log"
	"os"

	"github.com/goplus/llgo/xtool/env/llvm"
	nmtool "github.com/goplus/llgo/xtool/nm"
)

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "Usage: nmdump [flags] libfile")
		return
	}

	nm := llvm.New("").Nm()

	var flags []string
	libfile := os.Args[len(os.Args)-1]
	if len(os.Args) > 2 {
		flags = os.Args[1 : len(os.Args)-1]
	}

	items, err := nm.List(libfile, flags...)

	for _, item := range items {
		if item.File != "" {
			fmt.Printf("\n%s:\n", item.File)
		}
		for _, sym := range item.Symbols {
			var versionInfo string
			switch sym.VersionType {
			case nmtool.VersionSpecific:
				versionInfo = fmt.Sprintf("@%s", sym.Version)
			case nmtool.VersionDefault:
				versionInfo = fmt.Sprintf("@@%s", sym.Version)
			}
			if sym.FAddr {
				fmt.Printf("%016x %c %s%s\n", sym.Addr, sym.Type, sym.Name, versionInfo)
			} else {
				fmt.Printf("%16s %c %s%s\n", "", sym.Type, sym.Name, versionInfo)
			}
		}
	}
	if err != nil {
		log.Println(err)
	}
}
