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

package install_name_tool

import (
	"io"
	"os/exec"
	"path/filepath"
)

// Cmd represents a install_name_tool command.
type Cmd struct {
	app string

	Stdout io.Writer
	Stderr io.Writer
}

// New creates a new install_name_tool command.
func New(app string) *Cmd {
	if app == "" {
		app = "install_name_tool"
	}
	return &Cmd{app: app}
}

// Exec executes a install_name_tool command.
func (p *Cmd) Exec(args ...string) error {
	cmd := exec.Command(p.app, args...)
	cmd.Stdout = p.Stdout
	cmd.Stderr = p.Stderr
	return cmd.Run()
}

// Change represents a dependent shared library install name change.
type Change struct {
	Old string
	New string
}

// Change changes dependent shared library install name.
func (p *Cmd) Change(target string, chgs ...Change) error {
	args := make([]string, 0, len(chgs)*3+1)
	for _, chg := range chgs {
		args = append(args, "-change", chg.Old, chg.New)
	}
	args = append(args, target)
	return p.Exec(args...)
}

// ChangeToRpath changes dependent shared library install name to @rpath.
func (p *Cmd) ChangeToRpath(target string, dylibDeps ...string) error {
	args := make([]string, 0, len(dylibDeps)*3+1)
	for _, dep := range dylibDeps {
		args = append(args, "-change", dep, "@rpath/"+filepath.Base(dep))
	}
	args = append(args, target)
	return p.Exec(args...)
}
