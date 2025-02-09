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

	c "github.com/goplus/llgo/runtime/internal/clite"
)

func boolCStr(v bool) *c.Char {
	if v {
		return c.Str("true")
	}
	return c.Str("false")
}

func PrintBool(v bool) {
	c.Fprintf(c.Stderr, boolCStr(v))
}

func PrintByte(v byte) {
	c.Fputc(c.Int(v), c.Stderr)
}

func PrintFloat(v float64) {
	switch {
	case v != v:
		c.Fprintf(c.Stderr, c.Str("NaN"))
		return
	case v+v == v && v != 0:
		if v > 0 {
			c.Fprintf(c.Stderr, c.Str("+Inf"))
		} else {
			c.Fprintf(c.Stderr, c.Str("-Inf"))
		}
		return
	}
	c.Fprintf(c.Stderr, c.Str("%+e"), v)
}

func PrintComplex(v complex128) {
	print("(", real(v), imag(v), "i)")
}

func PrintUint(v uint64) {
	c.Fprintf(c.Stderr, c.Str("%llu"), v)
}

func PrintInt(v int64) {
	c.Fprintf(c.Stderr, c.Str("%lld"), v)
}

func PrintHex(v uint64) {
	c.Fprintf(c.Stderr, c.Str("%llx"), v)
}

func PrintPointer(p unsafe.Pointer) {
	c.Fprintf(c.Stderr, c.Str("%p"), p)
}

func PrintString(s String) {
	c.Fwrite(s.data, 1, uintptr(s.len), c.Stderr)
}

func PrintSlice(s Slice) {
	print("[", s.len, "/", s.cap, "]", s.data)
}

func PrintEface(e Eface) {
	print("(", e._type, ",", e.data, ")")
}

func PrintIface(i Iface) {
	print("(", i.tab, ",", i.data, ")")
}
