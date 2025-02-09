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

package watch

import (
	"log"
	"path/filepath"

	"github.com/goplus/gop/cmd/internal/base"
	"github.com/goplus/gop/tool"
	"github.com/goplus/gop/x/fsnotify"
	"github.com/goplus/gop/x/watcher"
)

// -----------------------------------------------------------------------------

// gop watch
var Cmd = &base.Command{
	UsageLine: "gop watch [-v -gentest] [dir]",
	Short:     "Monitor code changes in a Go+ workspace to generate Go files",
}

var (
	flag       = &Cmd.Flag
	verbose    = flag.Bool("v", false, "print verbose information.")
	debug      = flag.Bool("debug", false, "show all debug information.")
	genTestPkg = flag.Bool("gentest", false, "generate test package.")
)

func init() {
	Cmd.Run = runCmd
}

func runCmd(cmd *base.Command, args []string) {
	err := flag.Parse(args)
	if err != nil {
		log.Fatalln("parse input arguments failed:", err)
	}

	if *debug {
		fsnotify.SetDebug(fsnotify.DbgFlagAll)
	}
	if *debug || *verbose {
		watcher.SetDebug(watcher.DbgFlagAll)
	}

	args = flag.Args()
	if len(args) == 0 {
		args = []string{"."}
	}

	root, _ := filepath.Abs(args[0])
	log.Println("Watch", root)
	w := watcher.New(root)
	go w.Run()
	for {
		dir := w.Fetch(true)
		log.Println("GenGo", dir)
		_, _, err := tool.GenGo(dir, nil, *genTestPkg)
		if err != nil {
			log.Println(err)
		}
	}
}

// -----------------------------------------------------------------------------
