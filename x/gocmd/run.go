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

package gocmd

import (
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"syscall"
)

// -----------------------------------------------------------------------------

type RunConfig = Config

// RunDir runs a Go project by specified directory.
// If buildDir is not empty, it means split `go run` into `go build`
// in buildDir and run the built app in current directory.
func RunDir(buildDir, dir string, args []string, conf *RunConfig) (err error) {
	fis, err := os.ReadDir(dir)
	if err != nil {
		return
	}
	var files []string
	for _, fi := range fis {
		if !fi.IsDir() {
			if fname := fi.Name(); filterRunFname(fname) {
				files = append(files, filepath.Join(dir, fname))
			}
		}
	}
	return RunFiles(buildDir, files, args, conf)
}

func filterRunFname(fname string) bool {
	return strings.HasSuffix(fname, ".go") &&
		!(strings.HasSuffix(fname, "_test.go") || strings.HasPrefix(fname, "_"))
}

// -----------------------------------------------------------------------------

// RunFiles runs a Go project by specified files.
// If buildDir is not empty, it means split `go run` into `go build`
// in buildDir and run the built app in current directory.
func RunFiles(buildDir string, files []string, args []string, conf *RunConfig) (err error) {
	if len(files) == 0 {
		return syscall.ENOENT
	}
	if buildDir == "" {
		args = append(files, args...)
		return doWithArgs("", "run", conf, args...)
	}

	absFiles := make([]string, len(files))
	for i, file := range files {
		absFiles[i], _ = filepath.Abs(file)
	}

	f, err := os.CreateTemp("", "gobuild")
	if err != nil {
		return
	}
	tempf := f.Name()
	f.Close()
	os.Remove(tempf)
	defer os.Remove(tempf)

	buildArgs := append([]string{"-o", tempf}, absFiles...)
	if err = doWithArgs(buildDir, "build", conf, buildArgs...); err != nil {
		return
	}

	cmd := exec.Command(tempf, args...)
	return runCmd(cmd)
}

// -----------------------------------------------------------------------------
