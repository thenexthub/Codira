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

package yap

import (
	"reflect"
	"strings"
)

// Handler is worker class of YAP classfile (v2).
type Handler struct {
	Context
}

// Main is required by Go+ compiler as the entry of a YAP HTTP handler.
func (p *Handler) Main(ctx *Context) {
	p.Context = *ctx
}

var (
	repl = strings.NewReplacer("_", "/", "#", ":")
)

func parseClassfname(name string) (method, path string) {
	pos := strings.IndexByte(name, '_')
	if pos < 0 {
		return name, "/"
	}
	return name[:pos], repl.Replace(name[pos:])
}

// AppV2 is project class of YAP classfile (v2).
type AppV2 struct {
	App
}

type iHandler interface {
	Main(ctx *Context)
	Classfname() string
}

// Gopt_AppV2_Main is required by Go+ compiler as the entry of a YAP project.
func Gopt_AppV2_Main(app AppType, handlers ...iHandler) {
	app.InitYap()
	for _, h := range handlers {
		hVal := reflect.ValueOf(h).Elem()
		hVal.FieldByName("AppV2").Set(reflect.ValueOf(app))
		hType := hVal.Type()
		handle := func(ctx *Context) {
			// We must duplicate the handler instance for each request
			// to ensure state isolation.
			h2Val := reflect.New(hType).Elem()
			h2Val.Set(hVal)
			h2 := h2Val.Addr().Interface().(iHandler)
			h2.Main(ctx)
		}
		switch method, path := parseClassfname(h.Classfname()); method {
		case "handle":
			app.Handle(path, handle)
		default:
			app.Route(strings.ToUpper(method), path, handle)
		}
	}
	if me, ok := app.(interface{ MainEntry() }); ok {
		me.MainEntry()
	} else {
		app.Run("localhost:8080")
	}
}
