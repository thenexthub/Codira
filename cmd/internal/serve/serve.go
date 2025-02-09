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

// Package serve implements the “gop serve command.
package serve

import (
	"context"

	"github.com/goplus/gop/cmd/internal/base"
	"github.com/goplus/gop/x/jsonrpc2"
	"github.com/goplus/gop/x/jsonrpc2/stdio"
	"github.com/goplus/gop/x/langserver"
	"github.com/qiniu/x/log"
)

// gop serve
var Cmd = &base.Command{
	UsageLine: "gop serve [flags]",
	Short:     "Serve as a Go+ LangServer",
}

var (
	flag        = &Cmd.Flag
	flagVerbose = flag.Bool("v", false, "print verbose information")
)

func init() {
	Cmd.Run = runCmd
}

func runCmd(cmd *base.Command, args []string) {
	err := flag.Parse(args)
	if err != nil {
		log.Fatalln("parse input arguments failed:", err)
	}

	if *flagVerbose {
		jsonrpc2.SetDebug(jsonrpc2.DbgFlagCall)
	}

	listener := stdio.Listener(false)
	defer listener.Close()

	server := langserver.NewServer(context.Background(), listener, nil)
	server.Wait()
}

// -----------------------------------------------------------------------------
