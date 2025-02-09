//go:build go1.20
// +build go1.20

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

package reflectutil

import (
	"reflect"
)

// SetZero sets v to be the zero value of v's type.
// It panics if CanSet returns false.
func SetZero(v reflect.Value) {
	v.SetZero()
}
