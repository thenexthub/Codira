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
	"testing"
	"unsafe"
)

func TestSetZero(t *testing.T) {
	a := 2
	v := reflect.ValueOf(&a).Elem()
	SetZero(v)
	if a != 0 {
		t.Fatal("SetZero:", a)
	}
}

func TestUnsafeAddr(t *testing.T) {
	if unsafe.Sizeof(value{}) != unsafe.Sizeof(reflect.Value{}) {
		panic("unexpected sizeof reflect.Value")
	}
	v := reflect.ValueOf(0)
	if UnsafeAddr(v) == 0 {
		t.Fatal("UnsafeAddr")
	}
}
