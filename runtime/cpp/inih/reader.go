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

package inih

import (
	_ "unsafe"

	"github.com/goplus/llgo/c"
	"github.com/goplus/llgo/cpp/std"
)

// -----------------------------------------------------------------------------

// llgo:type C
type Reader struct {
	Unused [32]byte
}

// llgo:link (*Reader).InitFromBuffer C._ZN9INIReaderC1EPKcm
func (r *Reader) InitFromBuffer(buffer *c.Char, bufferSize uintptr) {}

// llgo:link (*Reader).InitFromFile C._ZN9INIReaderC1ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE
func (r *Reader) InitFromFile(fileName *std.String) {}

// llgo:link (*Reader).Dispose C.INIReaderDispose
func (r *Reader) Dispose() {}

// -----------------------------------------------------------------------------

// NewReader creates a new INIReader instance.
func NewReader(buffer *c.Char, bufferSize uintptr) (ret Reader) {
	ret.InitFromBuffer(buffer, bufferSize)
	return
}

// NewReaderFile creates a new INIReader instance.
func NewReaderFile(fileName *std.String) (ret Reader) {
	ret.InitFromFile(fileName)
	return
}

// -----------------------------------------------------------------------------

// llgo:link (*Reader).ParseError C._ZNK9INIReader10ParseErrorEv
func (r *Reader) ParseError() c.Int { return 0 }

// -----------------------------------------------------------------------------

// llgo:link (*Reader).GetInteger C._ZNK9INIReader10GetIntegerERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_l
func (r *Reader) GetInteger(section *std.String, name *std.String, defaultValue c.Long) c.Long {
	return 0
}

// llgo:link (*Reader).GetBoolean C._ZNK9INIReader10GetBooleanERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_b
func (r *Reader) GetBoolean(section *std.String, name *std.String, defaultValue bool) bool {
	return false
}

// llgo:link (*Reader).GetString C._ZNK9INIReader9GetStringERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_S8_
func (r *Reader) GetString(section *std.String, name *std.String, defaultValue *std.String) (ret std.String) {
	return
}

// -----------------------------------------------------------------------------
