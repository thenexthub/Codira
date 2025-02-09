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

package math

import (
	_ "unsafe"

	"github.com/goplus/llgo/py"
)

// https://docs.python.org/3/library/math.html

//go:linkname Pi py.pi
var Pi *py.Object

// With one argument, return the natural logarithm of x (to base e).
//
//go:linkname Log py.log
func Log(x *py.Object) *py.Object

// With two arguments, return the logarithm of x to the given base, calculated
// as log(x)/log(base).
//
//go:linkname LogOf py.log
func LogOf(x, base *py.Object) *py.Object

// Return the natural logarithm of 1+x (base e). The result is calculated in
// a way which is accurate for x near zero.
//
//go:linkname Log1p py.log1p
func Log1p(x *py.Object) *py.Object

// Return the Euclidean norm, sqrt(sum(x**2 for x in coordinates)). This is the
// length of the vector from the origin to the point given by the coordinates.
//
//go:linkname Hypot py.hypot
func Hypot(coordinates ...*py.Object) *py.Object
