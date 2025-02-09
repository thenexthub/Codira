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

package build

import (
	"fmt"
	"os"
	"path"
	"path/filepath"

	"github.com/goplus/llgo/compiler/internal/packages"
)

var (
	// TODO(xsw): complete clean flags
	cleanFlags = map[string]bool{
		"-v": false, // -v: print the paths of packages as they are clean
	}
)

func Clean(args []string, conf *Config) {
	flags, patterns, verbose := ParseArgs(args, cleanFlags)
	cfg := &packages.Config{
		Mode:       loadSyntax | packages.NeedExportFile,
		BuildFlags: flags,
	}

	if patterns == nil {
		patterns = []string{"."}
	}
	initial, err := packages.LoadEx(nil, nil, cfg, patterns...)
	check(err)

	cleanPkgs(initial, verbose)

	for _, pkg := range initial {
		if pkg.Name == "main" {
			cleanMainPkg(pkg, conf, verbose)
		}
	}
}

func cleanMainPkg(pkg *packages.Package, conf *Config, verbose bool) {
	pkgPath := pkg.PkgPath
	name := path.Base(pkgPath)
	fname := name + conf.AppExt
	app := filepath.Join(conf.BinPath, fname)
	removeFile(app, verbose)
	if len(pkg.CompiledGoFiles) > 0 {
		dir := filepath.Dir(pkg.CompiledGoFiles[0])
		buildApp := filepath.Join(dir, fname)
		removeFile(buildApp, verbose)
	}
}

func cleanPkgs(initial []*packages.Package, verbose bool) {
	packages.Visit(initial, nil, func(p *packages.Package) {
		file := p.ExportFile + ".ll"
		removeFile(file, verbose)
	})
}

func removeFile(file string, verbose bool) {
	if _, err := os.Stat(file); os.IsNotExist(err) {
		return
	}
	if verbose {
		fmt.Fprintln(os.Stderr, "Remove", file)
	}
	os.Remove(file)
}
