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

package env

import (
	"bytes"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

const (
	MainVersion = "1.3"
)

// buildVersion is the GoPlus tree's version string at build time.
// This is set by the linker.
var (
	buildVersion string
)

func init() {
	initEnv()
}

func initEnv() {
	if buildVersion == "" {
		initEnvByGop()
		return
	}
	if !strings.HasPrefix(buildVersion, "v"+MainVersion+".") {
		panic("gop/env: [FATAL] Invalid buildVersion: " + buildVersion)
	}
}

func initEnvByGop() {
	if fname := filepath.Base(os.Args[0]); !isGopCmd(fname) {
		if ret, err := gopEnv(); err == nil {
			parts := strings.SplitN(strings.TrimRight(ret, "\n"), "\n", 3)
			if len(parts) == 3 {
				buildVersion, buildDate, defaultGopRoot = parts[0], parts[1], parts[2]
			}
		}
	}
}

var gopEnv = func() (string, error) {
	var b bytes.Buffer
	cmd := exec.Command("gop", "env", "GOPVERSION", "BUILDDATE", "GOPROOT")
	cmd.Stdout = &b
	err := cmd.Run()
	return b.String(), err
}

// Installed checks is `gop` installed or not.
// If returns false, it means `gop` is not installed or not in PATH.
func Installed() bool {
	return buildVersion != ""
}

// Version returns the GoPlus tree's version string.
// It is either the commit hash and date at the time of the build or,
// when possible, a release tag like "v1.0.0-rc1".
func Version() string {
	if buildVersion == "" {
		return "v" + MainVersion + ".x"
	}
	return buildVersion
}
