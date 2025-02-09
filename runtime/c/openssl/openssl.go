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

package openssl

import (
	"unsafe"

	"github.com/goplus/llgo/c"
)

// -----------------------------------------------------------------------------

const (
	LLGoFiles   = "$(pkg-config --cflags openssl): _wrap/openssl.c"
	LLGoPackage = "link: $(pkg-config --libs openssl); -lssl -lcrypto"
)

//go:linkname Free C.opensslFree
func Free(ptr unsafe.Pointer)

//go:linkname FreeCStr C.opensslFree
func FreeCStr(ptr *c.Char)

// -----------------------------------------------------------------------------
