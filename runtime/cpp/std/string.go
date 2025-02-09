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

package std

import (
	"unsafe"

	"github.com/goplus/llgo/c"
)

// -----------------------------------------------------------------------------

// StringView represents a C++ std::string_view object.
type StringView = string

// -----------------------------------------------------------------------------

// String represents a C++ std::string object.
type String struct {
	Unused [3 * unsafe.Sizeof(0)]byte
}

// llgo:link (*String).InitEmpty C.stdStringInitEmpty
func (s *String) InitEmpty() {}

// llgo:link (*String).InitFrom C.stdStringInitFrom
func (s *String) InitFrom(v *String) {}

// llgo:link (*String).InitFromCStr C.stdStringInitFromCStr
func (s *String) InitFromCStr(cstr *c.Char) {}

// llgo:link (*String).InitFromCStrLen C.stdStringInitFromCStrLen
func (s *String) InitFromCStrLen(cstr *c.Char, n uintptr) {}

// llgo:link (*String).Dispose C.stdStringDispose
func (s *String) Dispose() {}

// -----------------------------------------------------------------------------

// NewString creates a C++ std::string object.
func NewString(v string) *String {
	ret := allocString()
	ret.InitFromCStrLen(c.GoStringData(v), uintptr(len(v)))
	return ret
}

// NewStringEmpty creates an empty std::string object.
func NewStringEmpty() *String {
	ret := allocString()
	ret.InitEmpty()
	return ret
}

// NewStringFrom creates a copy of a C++ std::string object.
func NewStringFrom(v *String) *String {
	ret := allocString()
	ret.InitFrom(v)
	return ret
}

// NewStringFromCStr creates a C++ std::string object.
func NewStringFromCStr(cstr *c.Char) *String {
	ret := allocString()
	ret.InitFromCStr(cstr)
	return ret
}

// NewStringFromCStrLen creates a C++ std::string object.
func NewStringFromCStrLen(cstr *c.Char, n uintptr) *String {
	ret := allocString()
	ret.InitFromCStrLen(cstr, n)
	return ret
}

// -----------------------------------------------------------------------------

// Str returns a Go string (it doesn't clone data of the C++ std::string object).
func (s *String) Str() string {
	return unsafe.String((*byte)(unsafe.Pointer(s.Data())), s.Size())
}

// llgo:link (*String).CStr C.stdStringCStr
func (s *String) CStr() *c.Char { return nil }

// llgo:link (*String).Data C.stdStringData
func (s *String) Data() *c.Char { return nil }

// llgo:link (*String).Size C.stdStringSize
func (s *String) Size() uintptr { return 0 }

// -----------------------------------------------------------------------------

// GoString converts a C++ std::string object to a Go string.
func GoString(v *String) string {
	return c.GoString(v.Data(), v.Size())
}

// Str creates a constant C++ std::string object.
func Str(v string) *String {
	return NewString(v) // TODO(xsw): optimize it
}

// -----------------------------------------------------------------------------
