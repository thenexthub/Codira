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

package rand

import (
	_ "unsafe"

	"github.com/goplus/llgo/c"
)

const (
	LLGoPackage = "decl"
)

// -----------------------------------------------------------------------------

//go:linkname Rand C.rand
func Rand() c.Int

//go:linkname RandR C.rand_r
func RandR(*c.Uint) c.Int

//go:linkname Srand C.srand
func Srand(c.Uint)

//go:linkname Sranddev C.sranddev
func Sranddev()

// -----------------------------------------------------------------------------

//go:linkname Random C.random
func Random() c.Long

//go:linkname Srandom C.srandom
func Srandom(c.Uint)

//go:linkname Srandomdev C.srandomdev
func Srandomdev()

// -----------------------------------------------------------------------------
