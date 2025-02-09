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

package clang

import (
	"io"
	"os"
	"os/exec"
)

// -----------------------------------------------------------------------------

// Cmd represents a clang command.
type Cmd struct {
	app string

	Stdout io.Writer
	Stderr io.Writer
}

// New creates a new clang command.
func New(app string) *Cmd {
	if app == "" {
		app = "clang"
	}
	return &Cmd{app, os.Stdout, os.Stderr}
}

// Exec executes a clang command.
func (p *Cmd) Exec(args ...string) error {
	cmd := exec.Command(p.app, args...)
	cmd.Stdout = p.Stdout
	cmd.Stderr = p.Stderr
	return cmd.Run()
}

// -----------------------------------------------------------------------------
