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

package mod

import (
	"language/gop/cmd/internal/base"
	"language/gop/cmd/internal/gopget"
)

// gop mod download
var cmdDownload = &base.Command{
	UsageLine: "gop mod download [-x -json] [modules]",
	Short:     "download modules to local cache",
}

func init() {
	cmdDownload.Run = gopget.Cmd.Run
}
