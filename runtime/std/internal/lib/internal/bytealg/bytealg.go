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

package bytealg

// llgo:skip init CompareString
import (
	"unsafe"

	c "github.com/goplus/llgo/runtime/internal/clite"
	"github.com/goplus/llgo/runtime/internal/runtime"
)

func IndexByte(b []byte, ch byte) int {
	ptr := unsafe.Pointer(unsafe.SliceData(b))
	ret := c.Memchr(ptr, c.Int(ch), uintptr(len(b)))
	if ret != nil {
		return int(uintptr(ret) - uintptr(ptr))
	}
	return -1
}

func IndexByteString(s string, ch byte) int {
	ptr := unsafe.Pointer(unsafe.StringData(s))
	ret := c.Memchr(ptr, c.Int(ch), uintptr(len(s)))
	if ret != nil {
		return int(uintptr(ret) - uintptr(ptr))
	}
	return -1
}

func Count(b []byte, c byte) (n int) {
	for _, x := range b {
		if x == c {
			n++
		}
	}
	return
}

func CountString(s string, c byte) (n int) {
	for i := 0; i < len(s); i++ {
		if s[i] == c {
			n++
		}
	}
	return
}

// Index returns the index of the first instance of b in a, or -1 if b is not present in a.
// Requires 2 <= len(b) <= MaxLen.
func Index(a, b []byte) int {
	for i := 0; i <= len(a)-len(b); i++ {
		if Equal(a[i:i+len(b)], b) {
			return i
		}
	}
	return -1
}

func Equal(a, b []byte) bool {
	if n := len(a); n == len(b) {
		return c.Memcmp(unsafe.Pointer(unsafe.SliceData(a)), unsafe.Pointer(unsafe.SliceData(b)), uintptr(n)) == 0
	}
	return false
}

// IndexString returns the index of the first instance of b in a, or -1 if b is not present in a.
// Requires 2 <= len(b) <= MaxLen.
func IndexString(a, b string) int {
	for i := 0; i <= len(a)-len(b); i++ {
		if a[i:i+len(b)] == b {
			return i
		}
	}
	return -1
}

// MakeNoZero makes a slice of length and capacity n without zeroing the bytes.
// It is the caller's responsibility to ensure uninitialized bytes
// do not leak to the end user.
func MakeNoZero(n int) (r []byte) {
	s := (*sliceHead)(unsafe.Pointer(&r))
	s.data = runtime.AllocU(uintptr(n))
	s.len = n
	s.cap = n
	return
}

type sliceHead struct {
	data unsafe.Pointer
	len  int
	cap  int
}

func LastIndexByte(s []byte, c byte) int {
	for i := len(s) - 1; i >= 0; i-- {
		if s[i] == c {
			return i
		}
	}
	return -1
}

func LastIndexByteString(s string, c byte) int {
	for i := len(s) - 1; i >= 0; i-- {
		if s[i] == c {
			return i
		}
	}
	return -1
}

func CompareString(a, b string) int {
	l := len(a)
	if len(b) < l {
		l = len(b)
	}
	for i := 0; i < l; i++ {
		c1, c2 := a[i], b[i]
		if c1 < c2 {
			return -1
		}
		if c1 > c2 {
			return +1
		}
	}
	if len(a) < len(b) {
		return -1
	}
	if len(a) > len(b) {
		return +1
	}
	return 0
}
