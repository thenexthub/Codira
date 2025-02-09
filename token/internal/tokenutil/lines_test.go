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

package tokenutil

import (
	"go/token"
	"reflect"
	"testing"
)

func TestLines(t *testing.T) {
	fset := token.NewFileSet()
	f := fset.AddFile("foo.go", 100, 100)
	lines := []int{0, 10, 50}
	f.SetLines(lines)
	ret := Lines(f)
	if !reflect.DeepEqual(ret, lines) {
		t.Fatal("TestLines failed:", ret)
	}
}
