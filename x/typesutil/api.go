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

package typesutil

import (
	"go/types"

	"github.com/goplus/gop/ast"
	"github.com/goplus/gop/token"
)

// A CheckConfig specifies the configuration for type checking.
// The zero value for Config is a ready-to-use default configuration.
type CheckConfig types.Config

// Check type-checks a package and returns the resulting package object and
// the first error if any. Additionally, if info != nil, Check populates each
// of the non-nil maps in the Info struct.
//
// The package is marked as complete if no errors occurred, otherwise it is
// incomplete. See Config.Error for controlling behavior in the presence of
// errors.
//
// The package is specified by a list of *ast.Files and corresponding
// file set, and the package path the package is identified with.
// The clean path must not be empty or dot (".").
func (conf *CheckConfig) Check(path string, fset *token.FileSet, files []*ast.File, info *Info) (ret *types.Package, err error) {
	ret = types.NewPackage(path, "")
	c := NewChecker((*types.Config)(conf), &Config{Types: ret, Fset: fset}, nil, info)
	err = c.Files(nil, files)
	return
}
