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

package md5

// llgo:skipall
import (
	"crypto"
	"hash"
	"unsafe"

	c "github.com/goplus/llgo/runtime/internal/clite"
	"github.com/goplus/llgo/runtime/internal/clite/openssl"
)

func init() {
	crypto.RegisterHash(crypto.MD5, New)
}

// The blocksize of MD5 in bytes.
const BlockSize = 64

// The size of an MD5 checksum in bytes.
const Size = 16

type digest struct {
	ctx openssl.MD5_CTX
}

func (d *digest) Size() int { return Size }

func (d *digest) BlockSize() int { return BlockSize }

func (d *digest) Reset() {
	d.ctx.Init()
}

func (d *digest) Write(p []byte) (nn int, err error) {
	d.ctx.UpdateBytes(p)
	return len(p), nil
}

func (d *digest) Sum(in []byte) []byte {
	hash := (*[Size]byte)(c.Alloca(Size))
	d.ctx.Final((*byte)(unsafe.Pointer(hash)))
	return append(in, hash[:]...)
}

func New() hash.Hash {
	d := new(digest)
	d.ctx.Init()
	return d
}

func Sum(data []byte) (ret [Size]byte) {
	openssl.MD5Bytes(data, &ret[0])
	return
}
