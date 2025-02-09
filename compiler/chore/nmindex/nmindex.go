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

	"github.com/goplus/llgo/xtool/env/llvm"
	"github.com/goplus/llgo/xtool/nm/nmindex"
)

func main() {
	if len(os.Args) < 2 {
		fmt.Fprint(os.Stderr, `Usage:
	nmindex <command> [arguments]

The commands are:

	mk		Create index file
	q		Query a symbol

`)
		return
	}

	cmd := os.Args[1]
	switch cmd {
	case "mk":
		makeIndex()
	case "q":
		if len(os.Args) < 3 {
			fmt.Fprint(os.Stderr, "Usage: nmindex q <symbol>\n")
			return
		}
		query(os.Args[2])
	default:
		fmt.Fprintf(os.Stderr, "unknown command: %s\n", cmd)
	}
}

func makeIndex() {
	env := llvm.New("")
	idxDir := indexDir()
	os.MkdirAll(idxDir, 0755)

	b := nmindex.NewIndexBuilder(env.Nm())
	libDirs := []string{
		usrLib(false),
		usrLib(true),
		stdLib("LLGO_STDROOT"),
		stdLib("LLGO_USRROOT"),
		pythonLib(),
	}
	err := b.Index(libDirs, idxDir, func(path string) {
		fmt.Println("==>", path)
	})
	check(err)
}

func query(q string) {
	if len(q) > 0 {
		if c := q[0]; c != '*' && c != '_' {
			q = "_" + q
		}
	}
	files, err := nmindex.Query(indexDir(), q)
	check(err)
	for _, f := range files {
		fmt.Printf("%s:\n", f.ArFile)
		for _, item := range f.Items {
			fmt.Printf("  %c %s %s\n", item.Type, item.Symbol, item.ObjFile)
		}
	}
}

func indexDir() string {
	home, err := os.UserHomeDir()
	check(err)
	return home + "/.llgo/nmindex"
}

func stdLib(where string) string {
	dir := os.Getenv(where)
	if dir != "" {
		dir += "/lib"
	}
	return dir
}

func usrLib(local bool) string {
	if local {
		return "/usr/local/lib"
	}
	return "/usr/lib"
}

func pythonLib() string {
	return os.Getenv("LLGO_PYTHON_ROOT")
}

func check(err error) {
	if err != nil {
		panic(err)
	}
}
