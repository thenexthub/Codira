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

import (
	"github.com/goplus/gop/cl/internal/spx/pkg"
)

type Sprite struct {
	pos pkg.Vector
	Entry
}

type Entry struct {
	vec pkg.Vector
}

func (p *Entry) Vector() *pkg.Vector {
	return &p.vec
}

func (p *Sprite) SetCostume(costume interface{}) {
}

func (p *Sprite) Say(msg string, secs ...float64) {
}

func (p *Sprite) Position() *pkg.Vector {
	return &p.pos
}

type Mesher interface {
	Name() string
}

func Gopt_Sprite_Clone__0(sprite interface{}) {
}

func Gopt_Sprite_Clone__1(sprite interface{}, data interface{}) {
}

func Gopt_Sprite_OnKey__0(sprite interface{}, a string, fn func()) {
}

func Gopt_Sprite_OnKey__1(sprite interface{}, a string, fn func(key string)) {
}

func Gopt_Sprite_OnKey__2(sprite interface{}, a []string, fn func()) {
}

func Gopt_Sprite_OnKey__3(sprite interface{}, a []string, fn func(key string)) {
}

func Gopt_Sprite_OnKey__4(sprite interface{}, a []Mesher, fn func()) {
}

func Gopt_Sprite_OnKey__5(sprite interface{}, a []Mesher, fn func(key Mesher)) {
}

func Gopt_Sprite_OnKey__6(sprite interface{}, a []string, b []string, fn func(key string)) {
}

func Gopt_Sprite_OnKey__7(sprite interface{}, a []string, b []Mesher, fn func(key string)) {
}

func Gopt_Sprite_OnKey__8(sprite interface{}, x int, y int) {
}

func Gopt_Sprite_OnKey2(sprite interface{}, a string, fn func(key string)) {
}
