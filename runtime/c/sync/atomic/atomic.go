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

package atomic

import (
	"unsafe"
)

const (
	LLGoPackage = "decl"
)

type valtype interface {
	~int | ~uint | ~uintptr | ~int32 | ~uint32 | ~int64 | ~uint64 | ~unsafe.Pointer
}

// llgo:link Add llgo.atomicAdd
func Add[T valtype](ptr *T, v T) T { return v }

// llgo:link Sub llgo.atomicSub
func Sub[T valtype](ptr *T, v T) T { return v }

// llgo:link And llgo.atomicAnd
func And[T valtype](ptr *T, v T) T { return v }

// llgo:link NotAnd llgo.atomicNand
func NotAnd[T valtype](ptr *T, v T) T { return v }

// llgo:link Or llgo.atomicOr
func Or[T valtype](ptr *T, v T) T { return v }

// llgo:link Xor llgo.atomicXor
func Xor[T valtype](ptr *T, v T) T { return v }

// llgo:link Max llgo.atomicMax
func Max[T valtype](ptr *T, v T) T { return v }

// llgo:link Min llgo.atomicMin
func Min[T valtype](ptr *T, v T) T { return v }

// llgo:link UMax llgo.atomicUMax
func UMax[T valtype](ptr *T, v T) T { return v }

// llgo:link UMin llgo.atomicUMin
func UMin[T valtype](ptr *T, v T) T { return v }

// llgo:link Load llgo.atomicLoad
func Load[T valtype](ptr *T) T { return *ptr }

// llgo:link Store llgo.atomicStore
func Store[T valtype](ptr *T, v T) {}

// llgo:link Exchange llgo.atomicXchg
func Exchange[T valtype](ptr *T, v T) T { return v }

// llgo:link CompareAndExchange llgo.atomicCmpXchg
func CompareAndExchange[T valtype](ptr *T, old, new T) (T, bool) { return old, false }
