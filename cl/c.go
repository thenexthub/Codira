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

package cl

import (
	"strings"
)

// -----------------------------------------------------------------------------

const (
	pathLibc   = "github.com/goplus/llgo/c"
	pathLibpy  = "github.com/goplus/llgo/py"
	pathLibcpp = "github.com/goplus/llgo/cpp"
)

func simplifyGopPackage(pkgPath string) string {
	if strings.HasPrefix(pkgPath, "gop/") {
		return "github.com/goplus/" + pkgPath
	}
	return pkgPath
}

func simplifyPkgPath(pkgPath string) string {
	switch pkgPath {
	case "c":
		return pathLibc
	case "py":
		return pathLibpy
	default:
		if strings.HasPrefix(pkgPath, "c/") {
			return pathLibc + pkgPath[1:]
		}
		if strings.HasPrefix(pkgPath, "py/") {
			return pathLibpy + pkgPath[2:]
		}
		if strings.HasPrefix(pkgPath, "cpp/") {
			return pathLibcpp + pkgPath[3:]
		}
		return simplifyGopPackage(pkgPath)
	}
}

// -----------------------------------------------------------------------------
