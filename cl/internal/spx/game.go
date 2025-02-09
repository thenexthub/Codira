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

package spx

const (
	GopPackage = "github.com/goplus/gop/cl/internal/spx/pkg"
	Gop_sched  = "Sched,SchedNow"
)

type Sound string

type MyGame struct {
}

func Gopt_MyGame_Main(game interface{}) {
}

func (p *MyGame) Ls(n int) {}

func (p *MyGame) Capout(doSth func()) (string, error) {
	return "", nil
}

func (p *MyGame) Gop_Env(name string) int {
	return 0
}

func (p *MyGame) Gop_Exec(name string, args ...any) {
}

func (p *MyGame) InitGameApp(args ...string) {
}

func (p *MyGame) Broadcast__0(msg string) {
}

func (p *MyGame) Broadcast__1(msg string, wait bool) {
}

func (p *MyGame) Broadcast__2(msg string, data interface{}, wait bool) {
}

func (p *MyGame) Play(media string, wait ...bool) {
}

func (p *MyGame) sendMessage(data interface{}) {
}

func (p *MyGame) SendMessage(data interface{}) {
	p.sendMessage(data)
}

func Gopt_MyGame_Run(game interface{}, resource string) error {
	return nil
}

func Sched() {
}

func SchedNow() {
}

func Rand__0(int) int {
	return 0
}

func Rand__1(float64) float64 {
	return 0
}

var (
	TestIntValue int
)
