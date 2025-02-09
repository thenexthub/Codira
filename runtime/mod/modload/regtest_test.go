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

package modload_test

import (
	"os"
	"testing"

	"github.com/goplus/mod/gopmod"
	"github.com/goplus/mod/modload/modtest"
	"golang.org/x/mod/module"
)

func TestGopClass(t *testing.T) {
	modtest.GopClass(t)
}

func TestGoCompiler(t *testing.T) {
	modtest.LLGoCompiler(t)
	modtest.TinyGoCompiler(t)
}

func TestImport(t *testing.T) {
	modtest.Import(t)
}

func TestClassfile(t *testing.T) {
	t.Log(os.Getwd())
	modVer := module.Version{Path: "github.com/goplus/yap", Version: "v0.5.0"}
	if _, err := gopmod.LoadMod(modVer); err != nil {
		t.Fatal("gopmod.LoadMod:", err)
	}
}
