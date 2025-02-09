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

package spx3

const (
	GopPackage = true
)

type Game struct {
}

func New() *Game {
	return nil
}

func (p *Game) initGame() {}

func (p *Game) Run() {}

type Sprite struct {
}

func (p *Sprite) Name() string {
	return "sprite"
}

func (p *Sprite) Main(name string) {}

func Gopt_Game_Main(game interface{ initGame() }, workers ...interface{ Main(name string) }) {
}
