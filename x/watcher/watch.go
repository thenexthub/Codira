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

// Package watcher monitors code changes in a Go+ workspace.
package watcher

import (
	"github.com/goplus/gop/x/fsnotify"
)

var (
	debugMod bool
)

const (
	DbgFlagMod = 1 << iota
	DbgFlagAll = DbgFlagMod
)

func SetDebug(dbgFlags int) {
	debugMod = (dbgFlags & DbgFlagMod) != 0
}

// -----------------------------------------------------------------------------------------

type Runner struct {
	w fsnotify.Watcher
	c *Changes
}

func New(root string) Runner {
	w := fsnotify.New()
	c := NewChanges(root)
	return Runner{w: w, c: c}
}

func (p Runner) Fetch(fullPath bool) (dir string) {
	return p.c.Fetch(fullPath)
}

func (p Runner) Run() error {
	root := p.c.root
	root = root[:len(root)-1]
	return p.w.Run(root, p.c, p.c.Ignore)
}

func (p *Runner) Close() error {
	return p.w.Close()
}

// -----------------------------------------------------------------------------------------
