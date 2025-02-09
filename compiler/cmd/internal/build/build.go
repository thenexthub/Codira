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

// Package build implements the "llgo build" command.
package build

import (
	"fmt"
	"os"

	"github.com/goplus/llgo/compiler/cmd/internal/base"
	"github.com/goplus/llgo/compiler/internal/build"
	"github.com/goplus/llgo/compiler/internal/mockable"
)

// llgo build
var Cmd = &base.Command{
	UsageLine: "llgo build [-o output] [build flags] [packages]",
	Short:     "Compile packages and dependencies",
}

func init() {
	Cmd.Run = runCmd
}

func runCmd(cmd *base.Command, args []string) {
	conf := &build.Config{
		Mode:   build.ModeBuild,
		AppExt: build.DefaultAppExt(),
	}
	if len(args) >= 2 && args[0] == "-o" {
		conf.OutFile = args[1]
		args = args[2:]
	}
	_, err := build.Do(args, conf)
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		mockable.Exit(1)
	}
}
