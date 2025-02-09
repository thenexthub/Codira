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

package llgen

import (
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"github.com/goplus/llgo/compiler/internal/build"
)

func GenFrom(fileOrPkg string) string {
	pkg, err := genFrom(fileOrPkg)
	check(err)
	return pkg.LPkg.String()
}

func genFrom(pkgPath string) (build.Package, error) {
	oldDbg := os.Getenv("LLGO_DEBUG")
	dbg := isDbgSymEnabled(filepath.Join(pkgPath, "flags.txt"))
	if dbg {
		os.Setenv("LLGO_DEBUG", "1")
	}
	defer os.Setenv("LLGO_DEBUG", oldDbg)

	conf := &build.Config{
		Mode:   build.ModeGen,
		AppExt: build.DefaultAppExt(),
	}
	pkgs, err := build.Do([]string{pkgPath}, conf)
	if err != nil {
		return nil, err
	}
	return pkgs[0], nil
}

func DoFile(fileOrPkg, outFile string) {
	ret := GenFrom(fileOrPkg)
	err := os.WriteFile(outFile, []byte(ret), 0644)
	check(err)
}

func isDbgSymEnabled(flagsFile string) bool {
	data, err := os.ReadFile(flagsFile)
	if err != nil {
		return false
	}
	toks := strings.Split(strings.Join(strings.Split(string(data), "\n"), " "), " ")
	for _, tok := range toks {
		if tok == "-dbg" {
			return true
		}
	}
	return false
}

func SmartDoFile(pkgPath string) {
	pkg, err := genFrom(pkgPath)
	check(err)

	const autgenFile = "llgo_autogen.ll"
	dir, _ := filepath.Split(pkg.GoFiles[0])
	absDir, _ := filepath.Abs(dir)
	absDir = filepath.ToSlash(absDir)
	fname := autgenFile
	if inCompilerDir(absDir) {
		fname = "out.ll"
	}
	outFile := dir + fname

	b, err := os.ReadFile(outFile)
	if err == nil && len(b) == 1 && b[0] == ';' {
		return // skip to gen
	}

	if err = os.WriteFile(outFile, []byte(pkg.LPkg.String()), 0644); err != nil {
		panic(err)
	}
	if false && fname == autgenFile {
		genZip(absDir, "llgo_autogen.lla", autgenFile)
	}
}

func genZip(dir string, outFile, inFile string) {
	cmd := exec.Command("zip", outFile, inFile)
	cmd.Dir = dir
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	cmd.Run()
}

func inCompilerDir(dir string) bool {
	return strings.Contains(dir, "/cl/_test")
}
