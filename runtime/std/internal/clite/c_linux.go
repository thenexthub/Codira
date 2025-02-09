//go:build linux
// +build linux

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

package clite

import _ "unsafe"

//go:linkname Stdin stdin
var Stdin FilePtr

//go:linkname Stdout stdout
var Stdout FilePtr

//go:linkname Stderr stderr
var Stderr FilePtr
