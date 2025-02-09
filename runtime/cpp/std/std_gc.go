//go:build !nogc
// +build !nogc

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
	"github.com/goplus/llgo/c/bdwgc"
)

// -----------------------------------------------------------------------------

func allocString() *String {
	ptr := bdwgc.Malloc(unsafe.Sizeof(String{}))
	bdwgc.RegisterFinalizer(ptr, func(obj, data c.Pointer) {
		(*String)(obj).Dispose()
	}, nil, nil, nil)
	return (*String)(ptr)
}

// -----------------------------------------------------------------------------
