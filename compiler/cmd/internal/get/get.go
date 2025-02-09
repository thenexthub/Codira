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

// Package get implements the "llgo get" command.
package get

import (
	"github.com/goplus/llgo/compiler/cmd/internal/base"
)

// llgo get
var Cmd = &base.Command{
	UsageLine: "llgo get [-t -u -v] [build flags] [packages]",
	Short:     "Add dependencies to current module and install them",
}

func init() {
	Cmd.Run = runCmd
}

func runCmd(cmd *base.Command, args []string) {
	panic("todo")
}
