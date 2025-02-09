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

package libuv

import (
	_ "unsafe"

	"github.com/goplus/llgo/c"
)

// struct uv_async_t
type Async struct {
	Handle
	// On macOS arm64, sizeof uv_async_t is 128 bytes.
	// Handle is 92 bytes, so we need 36 bytes to fill the gap.
	// Maybe reserve more for future use.
	Unused [36]byte
}

// typedef void (*uv_async_cb)(uv_async_t* handle);
// llgo:type C
type AsyncCb func(*Async)

// int uv_async_init(uv_loop_t*, uv_async_t* async, uv_async_cb async_cb);
//
// llgo:link (*Loop).Async C.uv_async_init
func (loop *Loop) Async(a *Async, cb AsyncCb) c.Int {
	return 0
}

// int uv_async_send(uv_async_t* async);
//
// llgo:link (*Async).Send C.uv_async_send
func (a *Async) Send() c.Int {
	return 0
}
