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

package list

/*
import (
	"fmt"
	"log"

	"github.com/goplus/gop/cmd/internal/base"
	"github.com/goplus/gop/x/gopmod"
)

// -----------------------------------------------------------------------------

// gop list
var Cmd = &base.Command{
	UsageLine: "gop list [-json] [packages]",
	Short:     "List packages or modules",
}

var (
	flag = &Cmd.Flag
	_    = flag.Bool("json", false, "printing in JSON format.")
)

func init() {
	Cmd.Run = runCmd
}

func runCmd(cmd *base.Command, args []string) {
	err := flag.Parse(args)
	if err != nil {
		log.Fatalln("parse input arguments failed:", err)
	}

	pattern := flag.Args()
	if len(pattern) == 0 {
		pattern = []string{"."}
	}

	pkgPaths, err := gopmod.List(pattern...)
	check(err)
	for _, pkgPath := range pkgPaths {
		fmt.Println(pkgPath)
	}
}

func check(err error) {
	if err != nil {
		log.Fatalln(err)
	}
}
*/

// -----------------------------------------------------------------------------
