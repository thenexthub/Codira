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

package typepatch

import (
	"go/token"
	"go/types"
	"unsafe"
)

type typesPackage struct {
	path      string
	name      string
	scope     *types.Scope
	imports   []*types.Package
	complete  bool
	fake      bool   // scope lookup errors are silently dropped if package is fake (internal use only)
	cgo       bool   // uses of this package will be rewritten into uses of declarations from _cgo_gotypes.go
	goVersion string // minimum Go version required for package (by Config.GoVersion, typically from go.mod)
}

type typesScope struct {
	parent   *types.Scope
	children []*types.Scope
	number   int
	elems    map[string]types.Object // TODO(xsw): ensure offset of elems
	pos, end token.Pos
	comment  string
	isFunc   bool
}

const (
	tagPatched = 0x17
)

func IsPatched(pkg *types.Package) bool {
	if pkg == nil {
		return false
	}
	p := (*typesPackage)(unsafe.Pointer(pkg))
	return *(*uint8)(unsafe.Pointer(&p.complete)) == tagPatched
}

func setPatched(pkg *types.Package) {
	p := (*typesPackage)(unsafe.Pointer(pkg))
	*(*uint8)(unsafe.Pointer(&p.complete)) = tagPatched
}

func setScope(pkg *types.Package, scope *types.Scope) {
	p := (*typesPackage)(unsafe.Pointer(pkg))
	p.scope = scope
}

func getElems(scope *types.Scope) map[string]types.Object {
	s := (*typesScope)(unsafe.Pointer(scope))
	return s.elems
}

func setElems(scope *types.Scope, elems map[string]types.Object) {
	s := (*typesScope)(unsafe.Pointer(scope))
	s.elems = elems
}

func Clone(alt *types.Package) *types.Package {
	ret := *alt
	return &ret
}

func Merge(alt, pkg *types.Package, skips map[string]struct{}, skipall bool) {
	setPatched(pkg)
	if skipall {
		return
	}

	scope := *alt.Scope()
	old := getElems(&scope)
	elems := make(map[string]types.Object, len(old))

	for name, o := range old {
		elems[name] = o
	}

	setElems(&scope, elems)
	setScope(alt, &scope)

	for name, o := range getElems(pkg.Scope()) {
		if _, ok := elems[name]; ok {
			continue
		}
		if _, ok := skips[name]; ok {
			continue
		}
		elems[name] = o
	}
}
