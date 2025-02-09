//go:build go1.21
// +build go1.21

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
)

// Lines returns the effective line offset table of the form described by SetLines.
// Callers must not mutate the result.
func Lines(f *token.File) []int {
	return f.Lines()
}
