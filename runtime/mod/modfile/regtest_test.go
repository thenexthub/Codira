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

package modfile_test

import (
	"testing"

	"github.com/goplus/mod/gopmod"
	"github.com/goplus/mod/modload/modtest"
)

func TestGopMod(t *testing.T) {
	mod := gopmod.Default
	if mod.HasModfile() {
		t.Fatal("gopmod.Default HasModfile?")
	}
	if path := mod.Modfile(); path != "" {
		t.Fatal("gopmod.Default.Modfile?", path)
	}
	if path := mod.Path(); path != "" {
		t.Fatal("gopmod.Default.Path?", path)
	}
	if path := mod.Root(); path != "" {
		t.Fatal("gopmod.Default.Root?", path)
	}
	if pt := mod.PkgType("foo"); pt != gopmod.PkgtStandard {
		t.Fatal("PkgType foo:", pt)
	}
}

func TestGopClass(t *testing.T) {
	modtest.GopClass(t)
}

func TestImport(t *testing.T) {
	modtest.Import(t)
}
