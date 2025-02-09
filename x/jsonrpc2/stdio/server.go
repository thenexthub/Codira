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
	"io"
	"os"
	"sync/atomic"

	"github.com/goplus/gop/x/fakenet"
	"github.com/goplus/gop/x/jsonrpc2"
	"github.com/goplus/gop/x/jsonrpc2/jsonrpc2test"
)

// -----------------------------------------------------------------------------

type listener struct {
}

func (p *listener) Close() error {
	return nil
}

// Accept blocks waiting for an incoming connection to the listener.
func (p *listener) Accept(context.Context) (io.ReadWriteCloser, error) {
	connCnt := &connCnt[server]
	if atomic.AddInt32(connCnt, 1) != 1 {
		atomic.AddInt32(connCnt, -1)
		return nil, ErrTooManyConnections
	}
	return fakenet.NewConn("stdio", os.Stdin, os.Stdout), nil
}

func (p *listener) Dialer() jsonrpc2.Dialer {
	return nil
}

// Listener returns a jsonrpc2.Listener based on stdin and stdout.
func Listener(allowLocalDail bool) jsonrpc2.Listener {
	if allowLocalDail {
		return jsonrpc2test.NetPipeListener()
	}
	return &listener{}
}

// -----------------------------------------------------------------------------
