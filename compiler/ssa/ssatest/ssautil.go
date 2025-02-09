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

package ssatest

import (
	"go/token"
	"go/types"
	"testing"

	"github.com/goplus/gogen/packages"
	"github.com/goplus/llgo/compiler/ssa"
)

func NewProgram(t *testing.T, target *ssa.Target) ssa.Program {
	fset := token.NewFileSet()
	imp := packages.NewImporter(fset)
	return NewProgramEx(t, target, imp)
}

func NewProgramEx(t *testing.T, target *ssa.Target, imp types.Importer) ssa.Program {
	prog := ssa.NewProgram(target)
	prog.SetRuntime(func() *types.Package {
		rt, err := imp.Import(ssa.PkgRuntime)
		if err != nil {
			t.Fatal("load runtime failed:", err)
		}
		return rt
	})
	prog.SetPython(func() *types.Package {
		rt, err := imp.Import(ssa.PkgPython)
		if err != nil {
			t.Fatal("load python failed:", err)
		}
		return rt
	})
	return prog
}

func Assert(t *testing.T, p ssa.Package, expected string) {
	t.Helper()
	if v := p.String(); v != expected {
		t.Fatalf("\n==> got:\n%s\n==> expected:\n%s\n", v, expected)
	}
}
