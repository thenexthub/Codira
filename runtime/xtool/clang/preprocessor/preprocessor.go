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

package preprocessor

import (
	"log"
	"os"
	"os/exec"
	"path/filepath"

	"language/llgo/xtool/clang/pathutil"
)

const (
	DbgFlagExecCmd = 1 << iota
	DbgFlagAll     = DbgFlagExecCmd
)

var (
	debugExecCmd bool
)

func SetDebug(flags int) {
	debugExecCmd = (flags & DbgFlagExecCmd) != 0
}

// -----------------------------------------------------------------------------

type Config struct {
	Compiler    string // default: clang
	PPFlag      string // default: -E
	BaseDir     string // base of include searching directory, should be absolute path
	IncludeDirs []string
	Defines     []string
	Flags       []string
}

func Do(infile, outfile string, conf *Config) (err error) {
	if infile, err = filepath.Abs(infile); err != nil {
		return
	}
	if outfile, err = filepath.Abs(outfile); err != nil {
		return
	}
	if conf == nil {
		conf = new(Config)
	}
	base := conf.BaseDir
	if base == "" {
		if base, err = os.Getwd(); err != nil {
			return
		}
	}
	compiler := conf.Compiler
	if compiler == "" {
		compiler = "clang"
	}
	ppflag := conf.PPFlag
	if ppflag == "" {
		ppflag = "-E"
	}
	n := 4 + len(conf.Flags) + len(conf.IncludeDirs) + len(conf.Defines)
	args := make([]string, 3, n)
	args[0] = ppflag
	args[1], args[2] = "-o", outfile
	args = append(args, conf.Flags...)
	for _, def := range conf.Defines {
		args = append(args, "-D"+def)
	}
	for _, inc := range conf.IncludeDirs {
		args = append(args, "-I"+pathutil.Canonical(base, inc))
	}
	args = append(args, infile)
	if debugExecCmd {
		log.Println("==> runCmd:", compiler, args)
	}
	cmd := exec.Command(compiler, args...)
	cmd.Dir = filepath.Dir(infile)
	cmd.Stdin = os.Stdin
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
}

// -----------------------------------------------------------------------------
