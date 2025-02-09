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

package runtime

import (
	"unsafe"

	c "github.com/goplus/llgo/runtime/internal/clite"
)

func CString(s string) *int8 {
	p := c.Malloc(uintptr(len(s)) + 1)
	return CStrCopy(p, *(*String)(unsafe.Pointer(&s)))
}

func CBytes(b []byte) *int8 {
	p := c.Malloc(uintptr(len(b)))
	c.Memcpy(p, unsafe.Pointer(&b[0]), uintptr(len(b)))
	return (*int8)(p)
}

func GoString(p *int8) string {
	return GoStringN(p, int(c.Strlen(p)))
}

func GoStringN(p *int8, n int) string {
	return string((*[1 << 30]byte)(unsafe.Pointer(p))[:n:n])
}

func GoBytes(p *int8, n int) []byte {
	return (*[1 << 30]byte)(unsafe.Pointer(p))[:n:n]
}
