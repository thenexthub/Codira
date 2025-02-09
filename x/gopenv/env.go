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

package gopenv

import (
	"github.com/goplus/gop/env"
	modenv "github.com/goplus/mod/env"
)

func Get() *modenv.Gop {
	return &modenv.Gop{
		Version:   env.Version(),
		BuildDate: env.BuildDate(),
		Root:      env.GOPROOT(),
	}
}
