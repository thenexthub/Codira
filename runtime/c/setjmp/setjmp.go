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

package setjmp

// #include <setjmp.h>
import "C"

import (
	_ "unsafe"

	"github.com/goplus/llgo/c"
)

const (
	LLGoPackage = "decl"
)

type (
	JmpBuf    = C.jmp_buf
	SigjmpBuf = C.sigjmp_buf
)

// -----------------------------------------------------------------------------

//go:linkname Setjmp C.setjmp
func Setjmp(env *JmpBuf) c.Int

//go:linkname Longjmp C.longjmp
func Longjmp(env *JmpBuf, val c.Int)

// -----------------------------------------------------------------------------

//go:linkname Siglongjmp C.siglongjmp
func Siglongjmp(env *SigjmpBuf, val c.Int)

// -----------------------------------------------------------------------------
