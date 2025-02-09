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

package py

/*

import (
	_ "unsafe"

	"github.com/goplus/llgo/c"
)

// https://docs.python.org/3/c-api/bytes.html

// String returns a new bytes object from a C string.
//
//go:linkname FromCStr C.PyBytes_FromString
func FromCStr(s *c.Char) *Object

// FromString returns a new bytes object from a Go string.
func FromString(s string) *Object {
	return stringFromStringAndSize(c.GoStringData(s), uintptr(len(s)))
}

//go:linkname stringFromStringAndSize C.PyBytes_FromStringAndSize
func stringFromStringAndSize(s *c.Char, size uintptr) *Object

// CStr returns the content of a bytes object as a C string.
//
// llgo:link (*Object).CStr C.PyBytes_AsString
func (o *Object) CStr() *c.Char { return nil }

// llgo:link (*Object).Strlen C.PyBytes_Size
func (o *Object) Strlen() uintptr { return 0 }

*/
