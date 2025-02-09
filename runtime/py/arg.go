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

// https://docs.python.org/3/c-api/arg.html

// Create a new value based on a format string similar to those accepted by the
// PyArg_Parse* family of functions and a sequence of values. Returns the value or
// nil in the case of an error; an exception will be raised if nil is returned.
// See https://docs.python.org/3/c-api/arg.html#c.Py_BuildValue
//
//go:linkname BuildValue C.Py_BuildValue
func BuildValue(format *c.Char, __llgo_va_list ...any) *Object
