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

package types

import (
	"go/token"
	"go/types"
	"unsafe"

	"github.com/goplus/gogen"
)

// -----------------------------------------------------------------------------

var (
	Void          = types.Typ[types.UntypedNil]
	UnsafePointer = types.Typ[types.UnsafePointer]

	Int     = types.Typ[types.Int32]
	Uint    = types.Typ[types.Uint32]
	Long    = types.Typ[uintptr(types.Int32)+unsafe.Sizeof(0)>>3]  // int32/int64
	Ulong   = types.Typ[uintptr(types.Uint32)+unsafe.Sizeof(0)>>3] // uint32/uint64
	NotImpl = UnsafePointer

	LongDouble = types.Typ[types.Float64]
)

func NotVoid(t types.Type) bool {
	return t != Void
}

func MangledName(tag, name string) string {
	return tag + "_" + name // TODO: use sth to replace _
}

// -----------------------------------------------------------------------------

var (
	ValistTag types.Type
	Valist    types.Type = types.NewSlice(gogen.TyEmptyInterface)
)

func init() {
	vaTag := types.NewTypeName(token.NoPos, types.Unsafe, MangledName("struct", "__va_list_tag"), nil)
	ValistTag = types.NewNamed(vaTag, types.NewStruct(nil, nil), nil)
	types.Universe.Insert(vaTag)
}

// -----------------------------------------------------------------------------

func NewFunc(params, results *types.Tuple, variadic bool) *types.Signature {
	panic("todo")
}

func NewPointer(typ types.Type) types.Type {
	panic("todo")
}

/*
func NewPointer(typ types.Type) types.Type {
	switch t := typ.(type) {
	case *types.Basic:
		if t == Void {
			return types.Typ[types.UnsafePointer]
		}
	case *types.Signature:
		if gogen.IsCSignature(t) {
			return types.NewSignatureType(nil, nil, nil, t.Params(), t.Results(), t.Variadic())
		}
	case *types.Named:
		if typ == ValistTag {
			return Valist
		}
	}
	return types.NewPointer(typ)
}

func IsFunc(typ types.Type) bool {
	sig, ok := typ.(*types.Signature)
	if ok {
		ok = gogen.IsCSignature(sig)
	}
	return ok
}

func Identical(typ1, typ2 types.Type) bool {
	return types.Identical(typ1, typ2)
}
*/

// -----------------------------------------------------------------------------
