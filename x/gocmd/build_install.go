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

// -----------------------------------------------------------------------------

type InstallConfig = Config

func Install(arg string, conf *InstallConfig) (err error) {
	return doWithArgs("", "install", conf, arg)
}

func InstallFiles(files []string, conf *InstallConfig) (err error) {
	return doWithArgs("", "install", conf, files...)
}

// -----------------------------------------------------------------------------

type BuildConfig = Config

func Build(arg string, conf *BuildConfig) (err error) {
	return doWithArgs("", "build", conf, arg)
}

func BuildFiles(files []string, conf *BuildConfig) (err error) {
	return doWithArgs("", "build", conf, files...)
}

// -----------------------------------------------------------------------------

type TestConfig = Config

func Test(arg string, conf *TestConfig) (err error) {
	return doWithArgs("", "test", conf, arg)
}

func TestFiles(files []string, conf *TestConfig) (err error) {
	return doWithArgs("", "test", conf, files...)
}

// -----------------------------------------------------------------------------
