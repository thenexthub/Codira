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
	"strings"

	"github.com/goplus/llgo/compiler/internal/llgen"
	"github.com/goplus/mod"
)

func main() {
	dir, _, err := mod.FindGoMod(".")
	check(err)

	llgenDir(dir + "/cl/_testlibc")
	llgenDir(dir + "/cl/_testlibgo")
	llgenDir(dir + "/cl/_testrt")
	llgenDir(dir + "/cl/_testgo")
	llgenDir(dir + "/cl/_testpy")
	llgenDir(dir + "/cl/_testdata")
}

func llgenDir(dir string) {
	fis, err := os.ReadDir(dir)
	check(err)
	for _, fi := range fis {
		name := fi.Name()
		if !fi.IsDir() || strings.HasPrefix(name, "_") {
			continue
		}
		testDir := dir + "/" + name
		fmt.Fprintln(os.Stderr, "llgen", testDir)
		check(os.Chdir(testDir))
		llgen.SmartDoFile(testDir)
	}
}

func check(err error) {
	if err != nil {
		panic(err)
	}
}
