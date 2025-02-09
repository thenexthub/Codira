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

package doc

import (
	"go/doc"
	"strings"
)

const (
	goptPrefix = "Gopt_" // template method
	gopoPrefix = "Gopo_" // overload function/method
	gopxPrefix = "Gopx_" // type as parameters function/method
	gopPackage = "GopPackage"
)

func isGopPackage(in *doc.Package) bool {
	for _, v := range in.Consts {
		for _, name := range v.Names {
			if name == gopPackage {
				return true
			}
		}
	}
	return false
}

func isGopoConst(name string) bool {
	return strings.HasPrefix(name, gopoPrefix)
}

func hasGopoConst(in *doc.Value) bool {
	for _, name := range in.Names {
		if isGopoConst(name) {
			return true
		}
	}
	return false
}

func isOverload(name string) bool {
	n := len(name)
	return n > 3 && name[n-3:n-1] == "__"
}

// Func (no _ func name)
// _Func (with _ func name)
// TypeName_Method (no _ method name)
// _TypeName__Method (with _ method name)
func checkTypeMethod(name string) mthd {
	if pos := strings.IndexByte(name, '_'); pos >= 0 {
		if pos == 0 {
			t := name[1:]
			if pos = strings.Index(t, "__"); pos <= 0 {
				return mthd{"", t} // _Func
			}
			return mthd{t[:pos], t[pos+2:]} // _TypeName__Method
		}
		return mthd{name[:pos], name[pos+1:]} // TypeName_Method
	}
	return mthd{"", name} // Func
}
