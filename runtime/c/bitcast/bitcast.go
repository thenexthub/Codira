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

package bitcast

import _ "unsafe"

const (
	LLGoFiles   = "_cast/cast.c"
	LLGoPackage = "link"
)

//go:linkname ToFloat64 C.llgoToFloat64
func ToFloat64(v uintptr) float64

//go:linkname ToFloat32 C.llgoToFloat32
func ToFloat32(v uintptr) float32

//go:linkname FromFloat64 C.llgoFromFloat64
func FromFloat64(v float64) uintptr

//go:linkname FromFloat32 C.llgoFromFloat32
func FromFloat32(v float32) uintptr
