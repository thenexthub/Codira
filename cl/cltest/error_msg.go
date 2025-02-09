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

package cltest

import (
	"os"
	"testing"

	"github.com/goplus/gop/ast"
	"github.com/goplus/gop/cl"
	"github.com/goplus/gop/parser"
	"github.com/goplus/gop/parser/fsx/memfs"
	"github.com/goplus/gop/scanner"
)

func Error(t *testing.T, msg, src string) {
	ErrorEx(t, "main", "bar.gop", msg, src)
}

func ErrorEx(t *testing.T, pkgname, filename, msg, src string) {
	fs := memfs.SingleFile("/foo", filename, src)
	pkgs, err := parser.ParseFSDir(Conf.Fset, fs, "/foo", parser.Config{})
	if err != nil {
		scanner.PrintError(os.Stderr, err)
		t.Fatal("parser.ParseFSDir failed")
	}
	conf := *Conf
	conf.NoFileLine = false
	conf.RelativeBase = "/foo"
	bar := pkgs[pkgname]
	_, err = cl.NewPackage("", bar, &conf)
	if err == nil {
		t.Fatal("no error?")
	}
	if ret := err.Error(); ret != msg {
		t.Fatalf("\nError: \"%s\"\nExpected: \"%s\"\n", ret, msg)
	}
}

func ErrorAst(t *testing.T, pkgname, filename, msg, src string) {
	f, _ := parser.ParseFile(Conf.Fset, filename, src, parser.AllErrors)
	pkg := &ast.Package{
		Name:  pkgname,
		Files: map[string]*ast.File{filename: f},
	}
	conf := *Conf
	conf.NoFileLine = false
	conf.RelativeBase = "/foo"
	_, err := cl.NewPackage("", pkg, &conf)
	if err == nil {
		t.Fatal("no error?")
	}
	if ret := err.Error(); ret != msg {
		t.Fatalf("\nError: \"%s\"\nExpected: \"%s\"\n", ret, msg)
	}
}
