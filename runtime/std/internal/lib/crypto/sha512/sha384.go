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

package sha512

import (
	"hash"
	"unsafe"

	c "github.com/goplus/llgo/runtime/internal/clite"
	"github.com/goplus/llgo/runtime/internal/clite/openssl"
)

type digest384 struct {
	ctx openssl.SHA384_CTX
}

func (d *digest384) Size() int { return Size384 }

func (d *digest384) BlockSize() int { return BlockSize }

func (d *digest384) Reset() {
	d.ctx.Init()
}

func (d *digest384) Write(p []byte) (nn int, err error) {
	d.ctx.UpdateBytes(p)
	return len(p), nil
}

func (d *digest384) Sum(in []byte) []byte {
	hash := (*[Size]byte)(c.Alloca(Size))
	d.ctx.Final((*byte)(unsafe.Pointer(hash)))
	return append(in, hash[:]...)
}

func New384() hash.Hash {
	d := new(digest384)
	d.ctx.Init()
	return d
}

func Sum384(data []byte) (ret [Size384]byte) {
	openssl.SHA384Bytes(data, &ret[0])
	return
}
