//go:build !linux
// +build !linux

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

package c

import _ "unsafe"

//go:linkname Stdin __stdinp
var Stdin FilePtr

//go:linkname Stdout __stdoutp
var Stdout FilePtr

//go:linkname Stderr __stderrp
var Stderr FilePtr
