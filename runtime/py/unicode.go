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

import (
	_ "unsafe"

	"github.com/goplus/llgo/c"
)

// https://docs.python.org/3/c-api/unicode.html

//go:linkname Str llgo.pystr
func Str(s string) *Object

//go:linkname FromCStr C.PyUnicode_FromString
func FromCStr(str *c.Char) *Object

//go:linkname FromCStrAndLen C.PyUnicode_FromStringAndSize
func FromCStrAndLen(str *c.Char, n int) *Object

// FromGoString returns a new Unicode object from a Go string.
func FromGoString(s string) *Object {
	return FromCStrAndLen(c.GoStringData(s), len(s))
}

// Return a pointer to the UTF-8 encoding of the Unicode object, and store the
// size of the encoded representation (in bytes) in size. The size argument can
// be nil; in this case no size will be stored. The returned buffer always has
// an extra null byte appended (not included in size), regardless of whether
// there are any other null code points.
//
// In the case of an error, nil is returned with an exception set and no size is
// stored.
//
// This caches the UTF-8 representation of the string in the Unicode object, and
// subsequent calls will return a pointer to the same buffer. The caller is not
// responsible for deallocating the buffer. The buffer is deallocated and pointers
// to it become invalid when the Unicode object is garbage collected.
//
// llgo:link (*Object).CStrAndLen C.PyUnicode_AsUTF8AndSize
func (u *Object) CStrAndLen() (*c.Char, uintptr) { return nil, 0 }

// As CStrAndLen, but does not store the len.
//
// llgo:link (*Object).CStr C.PyUnicode_AsUTF8
func (u *Object) CStr() *c.Char { return nil }

// Same as CStr. Provided for Go+.
//
// llgo:link (*Object).Cstr C.PyUnicode_AsUTF8
func (u *Object) Cstr() *c.Char { return nil }
