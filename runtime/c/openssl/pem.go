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

// typedef int (*pem_password_cb)(char *buf, int size, int rwflag, void *userdata);
//
// llgo:type C
type PemPasswordCb func(buf *c.Char, size, rwflag c.Int, userdata unsafe.Pointer) c.Int

// RSA *PEM_read_bio_RSAPrivateKey(BIO *bp, RSA **x, pem_password_cb *cb, void *u);
//
//go:linkname PEMReadBioRSAPrivateKey C.PEM_read_bio_RSAPrivateKey
func PEMReadBioRSAPrivateKey(bp *BIO, x **RSA, cb PemPasswordCb, u unsafe.Pointer) *RSA

// -----------------------------------------------------------------------------
