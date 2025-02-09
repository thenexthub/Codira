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

package env

import (
	"log"
	"os"
	"path/filepath"
	"syscall"
)

var (
	// This is set by the linker.
	defaultGopRoot string
)

// GOPROOT returns the root of the Go+ tree. It uses the GOPROOT environment variable,
// if set at process start, or else the root used during the Go+ build.
func GOPROOT() string {
	gopRoot, err := findGopRoot()
	if err != nil {
		log.Panicln("GOPROOT not found:", err)
	}
	return gopRoot
}

const (
	envGOPROOT = "GOPROOT"
)

func findGopRoot() (string, error) {
	envGopRoot := os.Getenv(envGOPROOT)
	if envGopRoot != "" {
		// GOPROOT must valid
		if isValidGopRoot(envGopRoot) {
			return envGopRoot, nil
		}
		log.Panicf("\n%s (%s) is not valid\n", envGOPROOT, envGopRoot)
	}

	// if parent directory is a valid gop root, use it
	exePath, err := executableRealPath()
	if err == nil {
		dir := filepath.Dir(exePath)
		parentDir := filepath.Dir(dir)
		if parentDir != dir && isValidGopRoot(parentDir) {
			return parentDir, nil
		}
	}

	// check defaultGopRoot, if it is valid, use it
	if defaultGopRoot != "" && isValidGopRoot(defaultGopRoot) {
		return defaultGopRoot, nil
	}

	// Compatible with old GOPROOT
	if home := HOME(); home != "" {
		gopRoot := filepath.Join(home, "gop")
		if isValidGopRoot(gopRoot) {
			return gopRoot, nil
		}
		goplusRoot := filepath.Join(home, "goplus")
		if isValidGopRoot(goplusRoot) {
			return goplusRoot, nil
		}
	}
	return "", syscall.ENOENT
}

// Mockable for testing.
var executable = func() (string, error) {
	return os.Executable()
}

func executableRealPath() (path string, err error) {
	path, err = executable()
	if err == nil {
		path, err = filepath.EvalSymlinks(path)
		if err == nil {
			path, err = filepath.Abs(path)
		}
	}
	return
}

func isFileExists(path string) bool {
	st, err := os.Stat(path)
	return err == nil && !st.IsDir()
}

func isDirExists(path string) bool {
	st, err := os.Stat(path)
	return err == nil && st.IsDir()
}

func isValidGopRoot(path string) bool {
	return isDirExists(filepath.Join(path, "cmd/gop")) && isFileExists(filepath.Join(path, "go.mod"))
}
