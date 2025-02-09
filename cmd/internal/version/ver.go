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

package version

import (
	"fmt"
	"runtime"

	"github.com/goplus/gop/cmd/internal/base"
	"github.com/goplus/gop/env"
)

// -----------------------------------------------------------------------------

// Cmd - gop version
var Cmd = &base.Command{
	UsageLine: "gop version [-v]",
	Short:     "Print Go+ version",
}

var (
	flag = &Cmd.Flag
	_    = flag.Bool("v", false, "print verbose information.")
)

func init() {
	Cmd.Run = runCmd
}

func runCmd(cmd *base.Command, args []string) {
	fmt.Printf("gop %s %s/%s\n", env.Version(), runtime.GOOS, runtime.GOARCH)
}

// -----------------------------------------------------------------------------
