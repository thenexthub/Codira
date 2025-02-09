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

package langserver

import (
	"context"

	"github.com/goplus/gop/x/jsonrpc2"
)

// -----------------------------------------------------------------------------

const (
	methodGenGo   = "gengo"
	methodChanged = "changed"
)

// -----------------------------------------------------------------------------

// Dialer is used by clients to dial a server.
type Dialer = jsonrpc2.Dialer

type AsyncCall = jsonrpc2.AsyncCall

// Client is a client of the language server.
type Client struct {
	conn *jsonrpc2.Connection
}

// Open uses the dialer to make a new connection and returns a client of the LangServer
// based on the connection.
func Open(ctx context.Context, dialer Dialer, onDone func()) (ret Client, err error) {
	c, err := jsonrpc2.Dial(ctx, dialer, jsonrpc2.BinderFunc(
		func(ctx context.Context, c *jsonrpc2.Connection) (ret jsonrpc2.ConnectionOptions) {
			return
		}), onDone)
	if err != nil {
		return
	}
	ret = Client{c}
	return
}

func (p Client) Close() error {
	return p.conn.Close()
}

func (p Client) AsyncGenGo(ctx context.Context, pattern ...string) *AsyncCall {
	return p.conn.Call(ctx, methodGenGo, pattern)
}

func (p Client) GenGo(ctx context.Context, pattern ...string) (err error) {
	return p.AsyncGenGo(ctx, pattern...).Await(ctx, nil)
}

func (p Client) Changed(ctx context.Context, files ...string) (err error) {
	return p.conn.Notify(ctx, methodChanged, files)
}

// -----------------------------------------------------------------------------
