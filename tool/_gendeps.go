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

package tool

import (
	"fmt"
	"go/token"
	"io/fs"
	"os"
	"path/filepath"
	"strings"

	"github.com/goplus/gop/parser"
	"github.com/goplus/mod/gopmod"
	"github.com/goplus/mod/modfetch"

	astmod "github.com/goplus/gop/ast/mod"
)

// -----------------------------------------------------------------------------

func GenDepMods(mod *gopmod.Module, dir string, recursively bool) (ret map[string]struct{}, err error) {
	modBase := mod.Path()
	ret = make(map[string]struct{})
	for _, r := range mod.Opt.Import {
		ret[r.ClassfileMod] = struct{}{}
	}
	err = HandleDeps(mod, dir, recursively, func(pkgPath string) {
		modPath, _ := modfetch.Split(pkgPath, modBase)
		if modPath != "" && modPath != modBase {
			ret[modPath] = struct{}{}
		}
	})
	return
}

func HandleDeps(mod *gopmod.Module, dir string, recursively bool, h func(pkgPath string)) (err error) {
	g := depsGen{
		deps: astmod.Deps{HandlePkg: h},
		mod:  mod,
		fset: token.NewFileSet(),
	}
	if recursively {
		err = filepath.WalkDir(dir, func(path string, d fs.DirEntry, err error) error {
			if err == nil && d.IsDir() {
				if strings.HasPrefix(d.Name(), "_") { // skip _
					return filepath.SkipDir
				}
				err = g.gen(path)
				if err != nil {
					fmt.Fprintln(os.Stderr, err)
				}
			}
			return err
		})
	} else {
		err = g.gen(dir)
	}
	return
}

type depsGen struct {
	deps astmod.Deps
	mod  *gopmod.Module
	fset *token.FileSet
}

func (p depsGen) gen(dir string) (err error) {
	pkgs, err := parser.ParseDirEx(p.fset, dir, parser.Config{
		ClassKind: p.mod.ClassKind,
		Mode:      parser.ImportsOnly,
	})
	if err != nil {
		return
	}

	for _, pkg := range pkgs {
		p.deps.Load(pkg, false)
	}
	return
}

// -----------------------------------------------------------------------------
