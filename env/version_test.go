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
	"os"
	"path/filepath"
	"testing"
)

func TestPanic(t *testing.T) {
	t.Run("initEnvPanic", func(t *testing.T) {
		defer func() {
			if e := recover(); e == nil {
				t.Fatal("initEnvPanic: no panic?")
			}
		}()
		buildVersion = "v1.2"
		initEnv()
	})
	t.Run("GOPROOT panic", func(t *testing.T) {
		defer func() {
			if e := recover(); e == nil {
				t.Fatal("GOPROOT: no panic?")
			}
		}()
		defaultGopRoot = ""
		os.Setenv(envGOPROOT, "")
		GOPROOT()
	})
}

func TestEnv(t *testing.T) {
	gopEnv = func() (string, error) {
		wd, _ := os.Getwd()
		root := filepath.Dir(wd)
		return "v1.0.0-beta1\n2023-10-18_17-45-50\n" + root + "\n", nil
	}
	buildVersion = ""
	initEnv()
	if !Installed() {
		t.Fatal("not Installed")
	}
	if Version() != "v1.0.0-beta1" {
		t.Fatal("TestVersion failed:", Version())
	}
	buildVersion = ""
	if Version() != "v"+MainVersion+".x" {
		t.Fatal("TestVersion failed:", Version())
	}
	if BuildDate() != "2023-10-18_17-45-50" {
		t.Fatal("BuildInfo failed:", BuildDate())
	}
}
