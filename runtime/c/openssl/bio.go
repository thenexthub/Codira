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

package openssl

import (
	"unsafe"

	"github.com/goplus/llgo/c"
)

// -----------------------------------------------------------------------------

type BIO struct {
	Unused [0]byte
}

// BIO *BIO_new_mem_buf(const void *buf, int len);
//
//go:linkname BIONewMemBuf C.BIO_new_mem_buf
func BIONewMemBuf(buf unsafe.Pointer, len c.Int) *BIO

// int BIO_free(BIO *a);
//
// llgo:link (*BIO).Free C.BIO_free
func (*BIO) Free() c.Int { return 0 }

// int BIO_up_ref(BIO *a);
//
// llgo:link (*BIO).UpRef C.BIO_up_ref
func (*BIO) UpRef() c.Int { return 0 }

// int BIO_read_ex(BIO *b, void *data, size_t dlen, size_t *readbytes);
//
// llgo:link (*BIO).ReadEx C.BIO_read_ex
func (*BIO) ReadEx(data unsafe.Pointer, dlen uintptr, readbytes *uintptr) c.Int { return 0 }

// int BIO_write(BIO *b, const void *data, int dlen);
//
// llgo:link (*BIO).Write C.BIO_write
func (*BIO) Write(data unsafe.Pointer, dlen c.Int) c.Int { return 0 }

// int BIO_write_ex(BIO *b, const void *data, size_t dlen, size_t *written);
//
// llgo:link (*BIO).WriteEx C.BIO_write_ex
func (*BIO) WriteEx(data unsafe.Pointer, dlen uintptr, written *uintptr) c.Int { return 0 }

// -----------------------------------------------------------------------------
