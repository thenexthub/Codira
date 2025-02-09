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

package jsonrpc2test

import (
	"context"
	"io"
	"net"
	"testing"
)

func TestNetPipeDone(t *testing.T) {
	np := &netPiper{
		done:   make(chan struct{}, 1),
		dialed: make(chan io.ReadWriteCloser),
	}
	np.done <- struct{}{}
	if f, err := np.Accept(context.Background()); f != nil || err != net.ErrClosed {
		t.Fatal("np.Accept:", f, err)
	}
	np.done <- struct{}{}
	if f, err := np.Dial(context.Background()); f != nil || err != net.ErrClosed {
		t.Fatal("np.Dial:", f, err)
	}
}
