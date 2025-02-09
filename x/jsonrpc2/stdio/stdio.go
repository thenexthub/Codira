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

package stdio

import (
	"context"
	"errors"
	"io"
	"sync/atomic"

	"github.com/goplus/gop/x/fakenet"
	"github.com/goplus/gop/x/jsonrpc2"
)

var (
	ErrTooManyConnections = errors.New("too many connections")
)

const (
	client = iota
	server
)

var (
	connCnt [2]int32
)

// -----------------------------------------------------------------------------

type dialer struct {
	in  io.ReadCloser
	out io.WriteCloser
}

// Dial returns a new communication byte stream to a listening server.
func (p *dialer) Dial(ctx context.Context) (io.ReadWriteCloser, error) {
	dailCnt := &connCnt[client]
	if atomic.AddInt32(dailCnt, 1) != 1 {
		atomic.AddInt32(dailCnt, -1)
		return nil, ErrTooManyConnections
	}
	return fakenet.NewConn("stdio.dialer", p.in, p.out), nil
}

// Dialer returns a jsonrpc2.Dialer based on in and out.
func Dialer(in io.ReadCloser, out io.WriteCloser) jsonrpc2.Dialer {
	return &dialer{in: in, out: out}
}

// Dial makes a new connection based on in and out, wraps the returned
// reader and writer using the framer to make a stream, and then builds a
// connection on top of that stream using the binder.
//
// The returned Connection will operate independently using the Preempter and/or
// Handler provided by the Binder, and will release its own resources when the
// connection is broken, but the caller may Close it earlier to stop accepting
// (or sending) new requests.
func Dial(in io.ReadCloser, out io.WriteCloser, binder jsonrpc2.Binder, onDone func()) (*jsonrpc2.Connection, error) {
	return jsonrpc2.Dial(context.Background(), Dialer(in, out), binder, onDone)
}

// -----------------------------------------------------------------------------
