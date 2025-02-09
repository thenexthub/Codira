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
	"strconv"
	"strings"

	goast "go/ast"
	gotoken "go/token"

	"github.com/goplus/gop/ast"
	"github.com/goplus/gop/token"
)

// ----------------------------------------------------------------------------

type Deps struct {
	HandlePkg func(pkgPath string)
}

func (p Deps) Load(pkg *ast.Package, withGopStd bool) {
	for _, f := range pkg.Files {
		p.LoadFile(f, withGopStd)
	}
	for _, f := range pkg.GoFiles {
		p.LoadGoFile(f)
	}
}

func (p Deps) LoadGoFile(f *goast.File) {
	for _, imp := range f.Imports {
		path := imp.Path
		if path.Kind == gotoken.STRING {
			if s, err := strconv.Unquote(path.Value); err == nil {
				if s == "C" {
					continue
				}
				p.HandlePkg(s)
			}
		}
	}
}

func (p Deps) LoadFile(f *ast.File, withGopStd bool) {
	for _, imp := range f.Imports {
		path := imp.Path
		if path.Kind == token.STRING {
			if s, err := strconv.Unquote(path.Value); err == nil {
				p.gopPkgPath(s, withGopStd)
			}
		}
	}
}

func (p Deps) gopPkgPath(s string, withGopStd bool) {
	if strings.HasPrefix(s, "gop/") {
		if !withGopStd {
			return
		}
		s = "github.com/goplus/gop/" + s[4:]
	} else if strings.HasPrefix(s, "C") {
		if len(s) == 1 {
			s = "github.com/goplus/libc"
		} else if s[1] == '/' {
			s = s[2:]
			if strings.IndexByte(s, '/') < 0 {
				s = "github.com/goplus/" + s
			}
		}
	}
	p.HandlePkg(s)
}

// ----------------------------------------------------------------------------
