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

package modfile

import (
	"testing"
)

func TestSplitFname(t *testing.T) {
	type testCase struct {
		fname string
		ext   string
	}
	cases := []testCase{
		{"foo.spx", ".spx"},
		{"foo_yap.gox", "_yap.gox"},
		{"foo.gox", ".gox"},
	}
	for _, c := range cases {
		if ext := ClassExt(c.fname); ext != c.ext {
			t.Fatalf("ClassExt(%s): expect %s, got %s\n", c.fname, c.ext, ext)
		}
	}
}
