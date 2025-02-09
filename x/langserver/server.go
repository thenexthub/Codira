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
	"encoding/json"
	"path/filepath"
	"sync"
	"time"

	"github.com/goplus/gop/tool"
	"github.com/goplus/gop/x/gopprojs"
	"github.com/goplus/gop/x/jsonrpc2"
)

// -----------------------------------------------------------------------------

// Listener is implemented by protocols to accept new inbound connections.
type Listener = jsonrpc2.Listener

// Server is a running server that is accepting incoming connections.
type Server = jsonrpc2.Server

// Config holds the options for new connections.
type Config struct {
	// Framer allows control over the message framing and encoding.
	// If nil, HeaderFramer will be used.
	Framer jsonrpc2.Framer
}

// NewServer creates a new LangServer and returns it.
func NewServer(ctx context.Context, listener Listener, conf *Config) (ret *Server) {
	h := newHandle()
	ret = jsonrpc2.NewServer(ctx, listener, jsonrpc2.BinderFunc(
		func(ctx context.Context, c *jsonrpc2.Connection) (ret jsonrpc2.ConnectionOptions) {
			if conf != nil {
				ret.Framer = conf.Framer
			}
			ret.Handler = h
			// ret.OnInternalError = h.OnInternalError
			return
		}))
	h.server = ret
	go h.runLoop()
	return
}

// -----------------------------------------------------------------------------

type none = struct{}

type handler struct {
	mutex sync.Mutex
	dirty map[string]none

	server *Server
}

func newHandle() *handler {
	return &handler{
		dirty: make(map[string]none),
	}
}

/*
func (p *handler) OnInternalError(err error) {
	panic("jsonrpc2: " + err.Error())
}
*/

func (p *handler) runLoop() {
	const (
		duration = time.Second / 100
	)
	for {
		var dir string
		p.mutex.Lock()
		for dir = range p.dirty {
			delete(p.dirty, dir)
			break
		}
		p.mutex.Unlock()
		if dir == "" {
			time.Sleep(duration)
			continue
		}
		tool.GenGoEx(dir, nil, true, tool.GenFlagPrompt)
	}
}

func (p *handler) Changed(files []string) {
	p.mutex.Lock()
	defer p.mutex.Unlock()

	for _, file := range files {
		dir := filepath.Dir(file)
		p.dirty[dir] = none{}
	}
}

func (p *handler) Handle(ctx context.Context, req *jsonrpc2.Request) (result interface{}, err error) {
	switch req.Method {
	case methodChanged:
		var files []string
		err = json.Unmarshal(req.Params, &files)
		if err != nil {
			return
		}
		p.Changed(files)
	case methodGenGo:
		var pattern []string
		err = json.Unmarshal(req.Params, &pattern)
		if err != nil {
			return
		}
		err = GenGo(pattern...)
	}
	return
}

func GenGo(pattern ...string) (err error) {
	projs, err := gopprojs.ParseAll(pattern...)
	if err != nil {
		return
	}
	conf, _ := tool.NewDefaultConf(".", 0)
	if conf != nil {
		defer conf.UpdateCache()
	}
	for _, proj := range projs {
		switch v := proj.(type) {
		case *gopprojs.DirProj:
			tool.GenGoEx(v.Dir, conf, true, 0)
		case *gopprojs.PkgPathProj:
			if v.Path == "builtin" {
				continue
			}
			tool.GenGoPkgPathEx("", v.Path, conf, true, 0)
		}
	}
	return
}

// -----------------------------------------------------------------------------
