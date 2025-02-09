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

package cl_test

import (
	"testing"

	"github.com/goplus/gop/cl/cltest"
)

func TestTestgop(t *testing.T) {
	cltest.FromDir(t, "", "./_testgop")
}

func TestTestc(t *testing.T) {
	cltest.FromDir(t, "", "./_testc")
}

func TestTestpy(t *testing.T) {
	cltest.FromDir(t, "", "./_testpy")
}
