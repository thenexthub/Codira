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

package numpy

import (
	_ "unsafe"

	"github.com/goplus/llgo/py"
)

// https://numpy.org/doc/stable/reference/index.html#reference

// Return evenly spaced values within a given interval.
//
//	numpy.arange([start, ]stop, [step, ]dtype=None, *, like=None)
//
// See https://numpy.org/doc/stable/reference/generated/numpy.arange.html#numpy-arange
//
//go:linkname Arange py.arange
func Arange(start, stop, step, dtype *py.Object) *py.Object

// Return a new array of given shape and type, without initializing entries.
//
//	numpy.empty(shape, dtype=float, order='C', *, like=None)
//
// See https://numpy.org/doc/stable/reference/generated/numpy.empty.html#numpy-empty
//
//go:linkname Empty py.empty
func Empty(shape, dtype, order *py.Object) *py.Object

// Return a new array of given shape and type, filled with zeros.
//
//	numpy.zeros(shape, dtype=float, order='C', *, like=None)
//
// See https://numpy.org/doc/stable/reference/generated/numpy.zeros.html#numpy-zeros
//
//go:linkname Zeros py.zeros
func Zeros(shape, dtype, order *py.Object) *py.Object

// Create an array.
//
//	numpy.array(object, dtype=None, *, copy=True, order='K', subok=False, ndmin=0, like=None)
//
// See https://numpy.org/doc/stable/reference/generated/numpy.array.html#numpy-array
//
//go:linkname Array py.array
func Array(object, dtype *py.Object) *py.Object

// Convert the input to an array.
//
//	numpy.asarray(a, dtype=None, order=None, *, like=None)
//
// See https://numpy.org/doc/stable/reference/generated/numpy.asarray.html#numpy-asarray
//
//go:linkname AsArray py.asarray
func AsArray(a, dtype, order *py.Object) *py.Object
