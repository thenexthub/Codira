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

package gocmd

import (
	"fmt"
	"os"
	"os/exec"

	"github.com/goplus/gop/x/gopenv"
	"github.com/goplus/mod/env"
)

type GopEnv = env.Gop

type Config struct {
	Gop   *GopEnv
	GoCmd string
	Flags []string
	Run   func(cmd *exec.Cmd) error
}

// -----------------------------------------------------------------------------

func doWithArgs(dir, op string, conf *Config, args ...string) (err error) {
	if conf == nil {
		conf = new(Config)
	}
	goCmd := conf.GoCmd
	if goCmd == "" {
		goCmd = Name()
	}
	exargs := make([]string, 1, 16)
	exargs[0] = op
	exargs = appendLdflags(exargs, conf.Gop)
	exargs = append(exargs, conf.Flags...)
	exargs = append(exargs, args...)
	cmd := exec.Command(goCmd, exargs...)
	cmd.Dir = dir
	run := conf.Run
	if run == nil {
		run = runCmd
	}
	return run(cmd)
}

func runCmd(cmd *exec.Cmd) (err error) {
	cmd.Stdin = os.Stdin
	cmd.Stderr = os.Stderr
	cmd.Stdout = os.Stdout
	return cmd.Run()
}

// -----------------------------------------------------------------------------

const (
	ldFlagVersion   = "-X \"github.com/goplus/gop/env.buildVersion=%s\""
	ldFlagBuildDate = "-X \"github.com/goplus/gop/env.buildDate=%s\""
	ldFlagGopRoot   = "-X \"github.com/goplus/gop/env.defaultGopRoot=%s\""
)

const (
	ldFlagAll = ldFlagVersion + " " + ldFlagBuildDate + " " + ldFlagGopRoot
)

func loadFlags(env *GopEnv) string {
	return fmt.Sprintf(ldFlagAll, env.Version, env.BuildDate, env.Root)
}

func appendLdflags(exargs []string, env *GopEnv) []string {
	if env == nil {
		env = gopenv.Get()
	}
	return append(exargs, "-ldflags", loadFlags(env))
}

// -----------------------------------------------------------------------------

// Name returns name of the go command.
// It returns value of environment variable `GOP_GOCMD` if not empty.
// If not found, it returns `go`.
func Name() string {
	goCmd := os.Getenv("GOP_GOCMD")
	if goCmd == "" {
		goCmd = "go"
	}
	return goCmd
}

// -----------------------------------------------------------------------------
