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

package cmplx

import (
	_ "unsafe"
)

const (
	LLGoPackage = true
)

// -----------------------------------------------------------------------------

//go:linkname Abs C.cabs
func Abs(z complex128) float64

//go:linkname Acos C.cacos
func Acos(z complex128) complex128

//go:linkname Acosh C.cacosh
func Acosh(z complex128) complex128

//go:linkname Asin C.casin
func Asin(z complex128) complex128

//go:linkname Asinh C.casinh
func Asinh(z complex128) complex128

//go:linkname Atan C.catan
func Atan(z complex128) complex128

//go:linkname Atanh C.catanh
func Atanh(z complex128) complex128

//go:linkname Cos C.ccos
func Cos(z complex128) complex128

//go:linkname Cosh C.ccosh
func Cosh(z complex128) complex128

//go:linkname Exp C.cexp
func Exp(z complex128) complex128

//go:linkname Log C.clog
func Log(z complex128) complex128

//go:linkname Log10 C.clog10
func Log10(z complex128) complex128

//go:linkname Phase C.carg
func Phase(z complex128) float64

//go:linkname Pow C.cpow
func Pow(x, y complex128) complex128

//go:linkname Sin C.csin
func Sin(z complex128) complex128

//go:linkname Sinh C.csinh
func Sinh(z complex128) complex128

//go:linkname Sqrt C.csqrt
func Sqrt(z complex128) complex128

//go:linkname Tan C.ctan
func Tan(z complex128) complex128

//go:linkname Tanh C.ctanh
func Tanh(z complex128) complex128

// -----------------------------------------------------------------------------
