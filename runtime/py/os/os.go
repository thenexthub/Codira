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

package os

import (
	_ "unsafe"

	"github.com/goplus/llgo/py"
)

// https://docs.python.org/3/library/os.html

// Rename the file or directory src to dst. If dst exists, the operation will
// fail with an OSError subclass in a number of cases:
//
// On Windows, if dst exists a FileExistsError is always raised. The operation
// may fail if src and dst are on different filesystems. Use shutil.move() to
// support moves to a different filesystem.
//
// On Unix, if src is a file and dst is a directory or vice-versa, an IsADirectoryError
// or a NotADirectoryError will be raised respectively. If both are directories and dst
// is empty, dst will be silently replaced. If dst is a non-empty directory, an OSError
// is raised. If both are files, dst will be replaced silently if the user has permission.
// The operation may fail on some Unix flavors if src and dst are on different filesystems.
// If successful, the renaming will be an atomic operation (this is a POSIX requirement).
//
//go:linkname Rename py.rename
func Rename(src, dst *py.Object) *py.Object
