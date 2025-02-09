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
)

// https://docs.python.org/3/c-api/float.html

//go:linkname Float C.PyFloat_FromDouble
func Float(v float64) *Object

//go:linkname FloatFromSring C.PyFloat_FromString
func FloatFromSring(v *Object) *Object

// llgo:link (*Object).Float64 C.PyFloat_AsDouble
func (o *Object) Float64() float64 { return 0 }
