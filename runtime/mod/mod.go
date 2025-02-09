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

package mod

import (
	"os"
	"path/filepath"
	"strings"
	"syscall"
)

var (
	ErrNotFound = syscall.ENOENT
)

// -----------------------------------------------------------------------------

func FindGoMod(dirFrom string) (dir, file string, err error) {
	if dirFrom == "" {
		dirFrom = "."
	}
	if dir, err = filepath.Abs(dirFrom); err != nil {
		return
	}
	for dir != "" {
		file = filepath.Join(dir, "go.mod")
		if fi, e := os.Lstat(file); e == nil && !fi.IsDir() {
			return
		}
		if dir, file = filepath.Split(strings.TrimRight(dir, "/\\")); file == "" {
			break
		}
	}
	err = ErrNotFound
	return
}

func GOMOD(dirFrom string) (file string, err error) {
	_, file, err = FindGoMod(dirFrom)
	return
}

func GOPMOD(dirFrom string) (file string, err error) {
	dir, _, err := FindGoMod(dirFrom)
	if err != nil {
		return
	}
	return filepath.Join(dir, "gop.mod"), nil
}

// -----------------------------------------------------------------------------
