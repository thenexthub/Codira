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

package runtime

import (
	"unsafe"

	"github.com/goplus/llgo/runtime/abi"
)

// Map represents a Go map.
type Map = hmap
type maptype = abi.MapType
type arraytype = abi.ArrayType
type structtype = abi.StructType

type slice struct {
	array unsafe.Pointer
	len   int
	cap   int
}

func typedmemmove(typ *_type, dst, src unsafe.Pointer) {
	Typedmemmove(typ, dst, src)
}

// MakeSmallMap creates a new small map.
func MakeSmallMap() *Map {
	return makemap_small()
}

func MakeMap(t *maptype, hint int) *hmap {
	return makemap(t, hint, nil)
}

func MapAssign(t *maptype, h *Map, key unsafe.Pointer) unsafe.Pointer {
	return mapassign(t, h, key)
}

func MapAccess1(t *maptype, h *hmap, key unsafe.Pointer) unsafe.Pointer {
	return mapaccess1(t, h, key)
}

func MapAccess2(t *maptype, h *hmap, key unsafe.Pointer) (unsafe.Pointer, bool) {
	return mapaccess2(t, h, key)
}

func MapDelete(t *maptype, h *hmap, key unsafe.Pointer) {
	mapdelete(t, h, key)
}

func MapClear(t *maptype, h *hmap) {
	mapclear(t, h)
}

func NewMapIter(t *maptype, h *hmap) *hiter {
	var it hiter
	mapiterinit(t, h, &it)
	return &it
}

func MapIterNext(it *hiter) (ok bool, k unsafe.Pointer, v unsafe.Pointer) {
	if it.key == nil {
		return
	}
	ok = true
	k, v = it.key, it.elem
	mapiternext(it)
	return
}

func MapLen(h *Map) int {
	if h == nil {
		return 0
	}
	return h.count
}
