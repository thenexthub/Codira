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

package deps

/*
import (
	"fmt"
	"log"

	"github.com/goplus/gop/cmd/internal/base"
	"github.com/goplus/gop/x/gopmod"
)

// -----------------------------------------------------------------------------

// gop deps
var Cmd = &base.Command{
	UsageLine: "gop deps [-v] [package]",
	Short:     "Show dependencies of a package or module",
}

var (
	flag = &Cmd.Flag
	_    = flag.Bool("v", false, "print verbose information.")
)

func init() {
	Cmd.Run = runCmd
}

func runCmd(cmd *base.Command, args []string) {
	err := flag.Parse(args)
	if err != nil {
		log.Fatalln("parse input arguments failed:", err)
	}

	var dir string
	narg := flag.NArg()
	if narg < 1 {
		dir = "."
	} else {
		dir = flag.Arg(0)
	}

	imports, err := gopmod.Imports(dir)
	check(err)
	for _, imp := range imports {
		fmt.Println(imp)
	}
}

func check(err error) {
	if err != nil {
		log.Fatalln(err)
	}
}
*/

// -----------------------------------------------------------------------------
