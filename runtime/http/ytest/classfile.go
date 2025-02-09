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

package ytest

import (
	"io"
	"net/http"
	"net/http/httptest"
	"os"
	"reflect"
	"strings"
	"testing"

	"github.com/goplus/yap"
	"github.com/goplus/yap/test"
	"github.com/goplus/yap/test/logt"
	"github.com/qiniu/x/mockhttp"
)

const (
	GopPackage   = "github.com/goplus/yap/test"
	GopTestClass = true
)

// -----------------------------------------------------------------------------

type App struct {
	hosts     map[string]string
	transport http.RoundTripper
}

func (p *App) initApp() *App {
	p.hosts = make(map[string]string)
	p.transport = http.DefaultTransport
	return p
}

// Mock runs a YAP server by mockhttp.
func (p *App) Mock(host string, app yap.AppType) {
	tr := mockhttp.NewTransport()
	p.transport = tr
	app.InitYap()
	app.SetLAS(func(addr string, h http.Handler) error {
		return tr.ListenAndServe(host, h)
	})
	app.(interface{ Main() }).Main()
}

// TestServer runs a YAP server by httptest.Server.
func (p *App) TestServer(host string, app yap.AppType) {
	app.InitYap()
	app.SetLAS(func(addr string, h http.Handler) error {
		svr := httptest.NewServer(h)
		p.Host("http://"+host, svr.URL)
		return nil
	})
	app.(interface{ Main() }).Main()
}

// RunMock runs a HTTP server by mockhttp.
func (p *App) RunMock(host string, h http.Handler) {
	tr := mockhttp.NewTransport()
	p.transport = tr
	tr.ListenAndServe(host, h)
}

// RunTestServer runs a HTTP server by httptest.Server.
func (p *App) RunTestServer(host string, h http.Handler) {
	svr := httptest.NewServer(h)
	p.Host("http://"+host, svr.URL)
}

// Gopt_App_TestMain is required by Go+ compiler as the TestMain entry of a YAP testing project.
func Gopt_App_TestMain(app interface{ initApp() *App }, m *testing.M) {
	app.initApp()
	if me, ok := app.(interface{ MainEntry() }); ok {
		me.MainEntry()
	}
	os.Exit(m.Run())
}

// Gopt_App_Main is required by Go+ compiler as the Main entry of a YAP testing project.
func Gopt_App_Main(app interface{ initApp() *App }, workers ...interface{ initCase(*App, CaseT) }) {
	a := app.initApp()
	if me, ok := app.(interface{ MainEntry() }); ok {
		me.MainEntry()
	}
	t := logt.New()
	for _, worker := range workers {
		worker.initCase(a, t)
		reflect.ValueOf(worker).Elem().Field(1).Set(reflect.ValueOf(app)) // (*worker).App = app
		worker.(interface{ Main() }).Main()
	}
}

// Host replaces a host into real. For example:
//
//	host "https://example.com", "http://localhost:8080"
//	host "http://example.com", "http://localhost:8888"
func (p *App) Host(host, real string) {
	if !strings.HasPrefix(host, "http") {
		test.Fatalf("invalid host `%s`: should start with http:// or https://\n", host)
	}
	p.hosts[host] = real
}

func (p *App) hostOf(url string) (host string, url2 string, ok bool) {
	// http://host/xxx or https://host/xxx
	var istart int
	if strings.HasPrefix(url[4:], "://") {
		istart = 7
	} else if strings.HasPrefix(url[4:], "s://") {
		istart = 8
	} else {
		return
	}

	next := url[istart:]
	n := strings.IndexByte(next, '/')
	if n < 1 {
		return
	}

	host = next[:n]
	portal, ok := p.hosts[url[:istart+n]]
	if ok {
		url2 = portal + url[istart+n:]
	}
	return
}

func (p *App) newRequest(method, url string, body io.Reader) (req *http.Request, err error) {
	if host, url2, ok := p.hostOf(url); ok {
		req, err = http.NewRequest(method, url2, body)
		req.Host = host
		return
	}
	return http.NewRequest(method, url, body)
}

// -----------------------------------------------------------------------------
