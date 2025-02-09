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
	"go/token"
	"go/types"
	"unsafe"
)

// -----------------------------------------------------------------------------

// A Scope maintains a set of objects and links to its containing
// (parent) and contained (children) scopes. Objects may be inserted
// and looked up by name. The zero value for Scope is a ready-to-use
// empty scope.
type Scope struct {
	parent   *Scope
	children []*Scope
	number   int                     // parent.children[number-1] is this scope; 0 if there is no parent
	elems    map[string]types.Object // lazily allocated
	pos, end token.Pos               // scope extent; may be invalid
	comment  string                  // for debugging only
	isFunc   bool                    // set if this is a function scope (internal use only)
}

// ScopeDelete deletes an object from specified scope by its name.
func ScopeDelete(s *types.Scope, name string) types.Object {
	elems := (*Scope)(unsafe.Pointer(s)).elems
	if o, ok := elems[name]; ok {
		delete(elems, name)
		return o
	}
	return nil
}

// -----------------------------------------------------------------------------

// An Error describes a type-checking error; it implements the error interface.
// A "soft" error is an error that still permits a valid interpretation of a
// package (such as "unused variable"); "hard" errors may lead to unpredictable
// behavior if ignored.
type Error struct {
	Fset *token.FileSet // file set for interpretation of Pos
	Pos  token.Pos      // error position
	Msg  string         // error message
	Soft bool           // if set, error is "soft"

	// go116code is a future API, unexported as the set of error codes is large
	// and likely to change significantly during experimentation. Tools wishing
	// to preview this feature may read go116code using reflection (see
	// errorcodes_test.go), but beware that there is no guarantee of future
	// compatibility.
	go116code  int
	go116start token.Pos
	go116end   token.Pos
}

func SetErrorGo116(ret *types.Error, code int, start, end token.Pos) {
	e := (*Error)(unsafe.Pointer(ret))
	e.go116code = code
	e.go116start = start
	e.go116end = end
}

// -----------------------------------------------------------------------------

func init() {
	if unsafe.Sizeof(Scope{}) != unsafe.Sizeof(types.Scope{}) {
		panic("unexpected sizeof types.Scope")
	}
	if unsafe.Sizeof(Error{}) != unsafe.Sizeof(types.Error{}) {
		panic("unexpected sizeof types.Error")
	}
}

// -----------------------------------------------------------------------------
