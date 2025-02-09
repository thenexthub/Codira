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

package clang

import (
	_ "unsafe"

	"github.com/goplus/llgo/c"
)

/**
 * A character string.
 *
 * The \c CXString type is used to return strings from the interface when
 * the ownership of that string might differ from one call to the next.
 * Use \c clang_getCString() to retrieve the string data and, once finished
 * with the string data, call \c clang_disposeString() to free the string.
 */
type String struct {
	Data         c.Pointer
	PrivateFlags c.Uint
}

/**
 * Retrieve the character data associated with the given string.
 */
// llgo:link String.CStr C.clang_getCString
func (String) CStr() *c.Char { return nil }

/**
 * Free the given string.
 */
// llgo:link String.Dispose C.clang_disposeString
func (String) Dispose() {}

type StringSet struct {
	Strings *String
	Count   c.Uint
}

/**
 * Free the given string set.
 */
// llgo:link (*StringSet).Dispose C.clang_disposeStringSet
func (*StringSet) Dispose() {}

func GoString(clangStr String) (str string) {
	defer clangStr.Dispose()
	cstr := clangStr.CStr()
	if cstr != nil {
		str = c.GoString(cstr)
	}
	return
}
