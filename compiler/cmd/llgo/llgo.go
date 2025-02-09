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
	"flag"
	"fmt"
	"os"
	"strings"

	"github.com/qiniu/x/log"

	"github.com/goplus/llgo/compiler/cmd/internal/base"
	"github.com/goplus/llgo/compiler/cmd/internal/build"
	"github.com/goplus/llgo/compiler/cmd/internal/clean"
	"github.com/goplus/llgo/compiler/cmd/internal/get"
	"github.com/goplus/llgo/compiler/cmd/internal/help"
	"github.com/goplus/llgo/compiler/cmd/internal/install"
	"github.com/goplus/llgo/compiler/cmd/internal/run"
	"github.com/goplus/llgo/compiler/cmd/internal/version"
	"github.com/goplus/llgo/compiler/internal/mockable"
)

func mainUsage() {
	help.PrintUsage(os.Stderr, base.Llgo)
}

func init() {
	flag.Usage = mainUsage
	base.Llgo.Commands = []*base.Command{
		build.Cmd,
		install.Cmd,
		get.Cmd,
		run.Cmd,
		run.CmpTestCmd,
		clean.Cmd,
		version.Cmd,
	}
}

func main() {
	flag.Parse()
	args := flag.Args()
	if len(args) < 1 {
		flag.Usage()
		return
	}
	log.SetFlags(log.Ldefault &^ log.LstdFlags)

	base.CmdName = args[0] // for error messages
	if args[0] == "help" {
		help.Help(os.Stderr, args[1:])
		return
	}

BigCmdLoop:
	for bigCmd := base.Llgo; ; {
		for _, cmd := range bigCmd.Commands {
			if cmd.Name() != args[0] {
				continue
			}
			args = args[1:]
			if len(cmd.Commands) > 0 {
				bigCmd = cmd
				if len(args) == 0 {
					help.PrintUsage(os.Stderr, bigCmd)
					mockable.Exit(2)
				}
				if args[0] == "help" {
					help.Help(os.Stderr, append(strings.Split(base.CmdName, " "), args[1:]...))
					return
				}
				base.CmdName += " " + args[0]
				continue BigCmdLoop
			}
			if !cmd.Runnable() {
				continue
			}
			cmd.Run(cmd, args)
			return
		}
		helpArg := ""
		if i := strings.LastIndex(base.CmdName, " "); i >= 0 {
			helpArg = " " + base.CmdName[:i]
		}
		fmt.Fprintf(os.Stderr, "llgo %s: unknown command\nRun 'llgo help%s' for usage.\n", base.CmdName, helpArg)
		mockable.Exit(2)
	}
}
