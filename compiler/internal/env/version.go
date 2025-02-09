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

package env

import (
	"runtime/debug"
)

const (
	devel = "(devel)"
)

// buildVersion is the LLGo tree's version string at build time. It should be
// set by the linker.
var buildVersion string

// Version returns the version of the running LLGo binary.
//
//export LLGoVersion
func Version() string {
	if buildVersion != "" {
		return buildVersion
	}
	info, ok := debug.ReadBuildInfo()
	if ok && info.Main.Version != "" {
		return info.Main.Version
	}
	return devel
}

func Devel() bool {
	return Version() == devel
}
