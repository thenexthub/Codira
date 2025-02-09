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

package mod

import (
	"fmt"
	"os"

	"github.com/goplus/gop/cmd/internal/base"
	"github.com/goplus/gop/tool"
	"github.com/goplus/gop/x/gopenv"
)

// gop mod tidy
var cmdTidy = &base.Command{
	UsageLine: "gop mod tidy [-e -v]",
	Short:     "add missing and remove unused modules",
}

func init() {
	cmdTidy.Run = runTidy
}

func runTidy(cmd *base.Command, args []string) {
	err := tool.Tidy(".", gopenv.Get())
	if err != nil {
		if tool.NotFound(err) {
			fmt.Fprintln(os.Stderr, "go.mod not found")
		} else {
			fmt.Fprintln(os.Stderr, err)
		}
		os.Exit(1)
	}
}
