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

package gopget

import (
	"fmt"
	"log"
	"os"

	"github.com/goplus/gop/cmd/internal/base"
	"github.com/goplus/gop/tool"
	"github.com/goplus/mod/modcache"
	"github.com/goplus/mod/modfetch"
	"github.com/goplus/mod/modload"
)

// -----------------------------------------------------------------------------

// Cmd - gop get
var Cmd = &base.Command{
	UsageLine: "gop get [-v] [packages]",
	Short:     `Add dependencies to current module and install them`,
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
	narg := flag.NArg()
	if narg < 1 {
		log.Fatalln("TODO: not impl")
	}
	for i := 0; i < narg; i++ {
		get(flag.Arg(i))
	}
}

func get(pkgPath string) {
	modBase := ""
	mod, err := modload.Load(".")
	noMod := tool.NotFound(err)
	if !noMod {
		check(err)
		modBase = mod.Path()
	}

	pkgModVer, _, err := modfetch.GetPkg(pkgPath, modBase)
	check(err)
	if noMod {
		return
	}

	pkgModRoot, err := modcache.Path(pkgModVer)
	check(err)

	pkgMod, err := modload.Load(pkgModRoot)
	check(err)

	check(mod.AddRequire(pkgModVer.Path, pkgModVer.Version, pkgMod.HasProject()))
	fmt.Fprintf(os.Stderr, "gop get: added %s %s\n", pkgModVer.Path, pkgModVer.Version)

	check(mod.Save())
}

func check(err error) {
	if err != nil {
		log.Fatalln(err)
	}
}

// -----------------------------------------------------------------------------
