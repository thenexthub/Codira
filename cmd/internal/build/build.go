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

// Package build implements the “gop build” command.
package build

import (
	"fmt"
	"log"
	"os"
	"path/filepath"
	"reflect"

	"github.com/goplus/gogen"
	"github.com/goplus/gop/cl"
	"github.com/goplus/gop/cmd/internal/base"
	"github.com/goplus/gop/tool"
	"github.com/goplus/gop/x/gocmd"
	"github.com/goplus/gop/x/gopprojs"
)

// gop build
var Cmd = &base.Command{
	UsageLine: "gop build [-debug -o output] [packages]",
	Short:     "Build Go+ files",
}

var (
	flag       = &Cmd.Flag
	flagDebug  = flag.Bool("debug", false, "print debug information")
	flagOutput = flag.String("o", "", "gop build output file")
)

func init() {
	Cmd.Run = runCmd
}

func runCmd(cmd *base.Command, args []string) {
	pass := base.PassBuildFlags(cmd)
	err := flag.Parse(args)
	if err != nil {
		log.Panicln("parse input arguments failed:", err)
	}

	if *flagDebug {
		gogen.SetDebug(gogen.DbgFlagAll &^ gogen.DbgFlagComments)
		cl.SetDebug(cl.DbgFlagAll)
		cl.SetDisableRecover(true)
	}

	args = flag.Args()
	if len(args) == 0 {
		args = []string{"."}
	}

	proj, args, err := gopprojs.ParseOne(args...)
	if err != nil {
		log.Panicln(err)
	}
	if len(args) != 0 {
		log.Panicln("too many arguments:", args)
	}

	conf, err := tool.NewDefaultConf(".", tool.ConfFlagNoTestFiles, pass.Tags())
	if err != nil {
		log.Panicln("tool.NewDefaultConf:", err)
	}
	defer conf.UpdateCache()

	confCmd := conf.NewGoCmdConf()
	if *flagOutput != "" {
		output, err := filepath.Abs(*flagOutput)
		if err != nil {
			log.Panicln(err)
		}
		confCmd.Flags = []string{"-o", output}
	}
	confCmd.Flags = append(confCmd.Flags, pass.Args...)
	build(proj, conf, confCmd)
}

func build(proj gopprojs.Proj, conf *tool.Config, build *gocmd.BuildConfig) {
	const flags = tool.GenFlagPrompt
	var obj string
	var err error
	switch v := proj.(type) {
	case *gopprojs.DirProj:
		obj = v.Dir
		err = tool.BuildDir(obj, conf, build, flags)
	case *gopprojs.PkgPathProj:
		obj = v.Path
		err = tool.BuildPkgPath("", v.Path, conf, build, flags)
	case *gopprojs.FilesProj:
		err = tool.BuildFiles(v.Files, conf, build)
	default:
		log.Panicln("`gop build` doesn't support", reflect.TypeOf(v))
	}
	if tool.NotFound(err) {
		fmt.Fprintf(os.Stderr, "gop build %v: not found\n", obj)
	} else if err != nil {
		fmt.Fprintln(os.Stderr, err)
	} else {
		return
	}
	os.Exit(1)
}

// -----------------------------------------------------------------------------
