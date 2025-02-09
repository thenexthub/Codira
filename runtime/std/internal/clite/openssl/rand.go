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

	c "github.com/goplus/llgo/runtime/internal/clite"
)

// -----------------------------------------------------------------------------

// int RAND_bytes(unsigned char *buf, int num);
//
//go:linkname RANDBufferWithLen C.RAND_bytes
func RANDBufferWithLen(buf *byte, num c.Int) c.Int

func RANDBytes(buf []byte) c.Int {
	return RANDBufferWithLen(unsafe.SliceData(buf), c.Int(len(buf)))
}

// int RAND_priv_bytes(unsigned char *buf, int num);
//
//go:linkname RANDPrivBufferWithLen C.RAND_priv_bytes
func RANDPrivBufferWithLen(buf *byte, num c.Int) c.Int

func RANDPrivBytes(buf []byte) c.Int {
	return RANDPrivBufferWithLen(unsafe.SliceData(buf), c.Int(len(buf)))
}

// void RAND_seed(const void *buf, int num);
//
//go:linkname RANDSeed C.RAND_seed
func RANDSeed(buf unsafe.Pointer, num c.Int)

// void RAND_keep_random_devices_open(int keep);
//
//go:linkname RANDKeepRandomDevicesOpen C.RAND_keep_random_devices_open
func RANDKeepRandomDevicesOpen(keep c.Int)

// int RAND_load_file(const char *file, long max_bytes);
//
//go:linkname RANDLoadFile C.RAND_load_file
func RANDLoadFile(file *c.Char, maxBytes c.Long) c.Int

// int RAND_write_file(const char *file);
//
//go:linkname RANDWriteFile C.RAND_write_file
func RANDWriteFile(file *c.Char) c.Int

// const char *RAND_file_name(char *file, size_t num);
//
//go:linkname RANDFileName C.RAND_file_name
func RANDFileName(file *c.Char, num uintptr) *c.Char

// int RAND_status(void);
//
//go:linkname RANDStatus C.RAND_status
func RANDStatus() c.Int

// int RAND_poll(void);
//
//go:linkname RANDPoll C.RAND_poll
func RANDPoll() c.Int

// -----------------------------------------------------------------------------
