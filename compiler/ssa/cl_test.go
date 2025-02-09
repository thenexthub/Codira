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

package ssa_test

import (
	"flag"
	"go/types"
	"io"
	"log"
	"testing"

	"github.com/goplus/llgo/compiler/cl/cltest"
	"github.com/goplus/llgo/compiler/ssa"
	"github.com/goplus/llgo/compiler/ssa/ssatest"
)

func TestMain(m *testing.M) {
	flag.Parse()
	ssa.SetDebug(ssa.DbgFlagAll)
	if !testing.Verbose() {
		log.SetOutput(io.Discard)
	}
	m.Run()
}

func TestFromTestlibgo(t *testing.T) {
	cltest.FromDir(t, "", "../cl/_testlibgo")
}

func TestFromTestgo(t *testing.T) {
	cltest.FromDir(t, "", "../cl/_testgo")
}

func TestFromTestpy(t *testing.T) {
	cltest.FromDir(t, "", "../cl/_testpy")
}

func TestFromTestlibc(t *testing.T) {
	cltest.FromDir(t, "", "../cl/_testlibc")
}

func TestFromTestrt(t *testing.T) {
	cltest.FromDir(t, "", "../cl/_testrt")
}

func TestFromTestdata(t *testing.T) {
	cltest.FromDir(t, "", "../cl/_testdata")
}

func TestMakeInterface(t *testing.T) {
	prog := ssatest.NewProgram(t, &ssa.Target{GOARCH: "x86"})
	pkg := prog.NewPackage("foo", "foo")
	fn := pkg.NewFunc("main", types.NewSignatureType(nil, nil, nil, nil, nil, false), ssa.InC)
	b := fn.MakeBody(1)
	b.MakeInterface(prog.Any(), prog.IntVal(100, prog.Int64()))
	b.MakeInterface(prog.Any(), prog.FloatVal(100, prog.Float64()))
	b.DeferData()
	b.Return()
}

/*
func TestCallback(t *testing.T) {
	ctx := llvm.NewContext()
	mod := ctx.NewModule("foo/bar")

	tc := llvm.FunctionType(ctx.VoidType(), nil, false)
	callback := llvm.PointerType(tc, 0)
	params := []llvm.Type{callback}

	tfn := llvm.FunctionType(ctx.VoidType(), params, false)
	f := llvm.AddFunction(mod, "fn", tfn)
	b := ctx.NewBuilder()
	blk := llvm.AddBasicBlock(f, "")
	b.SetInsertPointAtEnd(blk)

	arg := f.Param(0)
	// arg = b.CreateLoad(tc, arg, "")
	b.CreateCall(tc, arg, nil, "")
	b.CreateRetVoid()

	expected := `; ModuleID = 'foo/bar'
`
	if v := mod.String(); v != expected {
		t.Fatalf("\n==> got:\n%s\n==> expected:\n%s\n", v, expected)
	}
}

/*
func TestMap(t *testing.T) {
	var m typeutil.Map
	sig := types.NewSignatureType(nil, nil, nil, nil, nil, false)
	m.Set(sig, 1)
	csig := (*ssa.CFuncPtr)(sig)
	m.Set(csig, 2)
	if v := m.At(sig); v.(int) != 1 {
		t.Fatal("At(sig):", v)
	}
	if v := m.At(csig); v.(int) != 2 {
		t.Fatal("At(csig):", v)
	}
}
*/
