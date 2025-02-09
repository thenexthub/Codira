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

package goptest

import (
	"go/token"

	"github.com/goplus/gop/ast/gopq"
	"github.com/goplus/gop/parser/fsx/memfs"
)

// -----------------------------------------------------------------------------

// New creates a nodeset object that represents a Go+ dom tree.
func New(script string) (gopq.NodeSet, error) {
	fset := token.NewFileSet()
	fs := memfs.SingleFile("/foo", "bar.gop", script)
	return gopq.NewSourceFrom(fset, fs, "/foo", nil, 0)
}

// -----------------------------------------------------------------------------
