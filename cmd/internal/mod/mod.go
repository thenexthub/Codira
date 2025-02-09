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
	"log"
	"os"

	"github.com/goplus/gop/cmd/internal/base"
)

var Cmd = &base.Command{
	UsageLine: "gop mod",
	Short:     "Module maintenance",

	Commands: []*base.Command{
		cmdInit,
		cmdDownload,
		cmdTidy,
	},
}

func check(err error) {
	if err != nil {
		log.Panicln(err)
	}
}

func fatal(msg interface{}) {
	fmt.Fprintln(os.Stderr, msg)
	os.Exit(1)
}
