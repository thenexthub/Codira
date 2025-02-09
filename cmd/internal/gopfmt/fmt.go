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

// Package gopfmt implements the “gop fmt” command.
package gopfmt

import (
	"bytes"
	"fmt"
	"io/fs"
	"log"
	"os"
	"path/filepath"
	"strings"

	"github.com/goplus/gop/cmd/internal/base"
	"github.com/goplus/gop/format"
	"github.com/goplus/gop/tool"

	goformat "go/format"
	"go/parser"
	"go/token"

	xformat "github.com/goplus/gop/x/format"
)

// Cmd - gop fmt
var Cmd = &base.Command{
	UsageLine: "gop fmt [flags] path ...",
	Short:     "Format Go+ packages",
}

var (
	flag        = &Cmd.Flag
	flagTest    = flag.Bool("t", false, "test if Go+ files are formatted or not.")
	flagNotExec = flag.Bool("n", false, "prints commands that would be executed.")
	flagMoveGo  = flag.Bool("mvgo", false, "move .go files to .gop files (only available in `--smart` mode).")
	flagSmart   = flag.Bool("smart", false, "convert Go code style into Go+ style.")
)

func init() {
	Cmd.Run = runCmd
}

var (
	testErrCnt = 0
	procCnt    = 0
	walkSubDir = false
	rootDir    = ""
)

func gopfmt(path string, class, smart, mvgo bool) (err error) {
	src, err := os.ReadFile(path)
	if err != nil {
		return
	}
	var target []byte
	if smart {
		target, err = xformat.GopstyleSource(src, path)
	} else {
		if !mvgo && filepath.Ext(path) == ".go" {
			fset := token.NewFileSet()
			f, err := parser.ParseFile(fset, path, src, parser.ParseComments)
			if err != nil {
				return err
			}
			var buf bytes.Buffer
			err = goformat.Node(&buf, fset, f)
			if err != nil {
				return err
			}
			target = buf.Bytes()
		} else {
			target, err = format.Source(src, class, path)
		}
	}
	if err != nil {
		return
	}
	if bytes.Equal(src, target) {
		return
	}
	fmt.Println(path)
	if *flagTest {
		testErrCnt++
		return nil
	}
	if mvgo {
		newPath := strings.TrimSuffix(path, ".go") + ".code"
		if err = os.WriteFile(newPath, target, 0666); err != nil {
			return
		}
		return os.Remove(path)
	}
	return writeFileWithBackup(path, target)
}

func writeFileWithBackup(path string, target []byte) (err error) {
	dir, file := filepath.Split(path)
	f, err := os.CreateTemp(dir, file)
	if err != nil {
		return
	}
	tmpfile := f.Name()
	_, err = f.Write(target)
	f.Close()
	if err != nil {
		return
	}
	err = os.Remove(path)
	if err != nil {
		return
	}
	return os.Rename(tmpfile, path)
}

type walker struct {
	dirMap map[string]func(ext string) (ok, class bool)
}

func newWalker() *walker {
	return &walker{dirMap: make(map[string]func(ext string) (ok, class bool))}
}

func (w *walker) walk(path string, d fs.DirEntry, err error) error {
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
	} else if d.IsDir() {
		if !walkSubDir && path != rootDir {
			return filepath.SkipDir
		}
	} else {
		dir, _ := filepath.Split(path)
		fn, ok := w.dirMap[dir]
		if !ok {
			if mod, err := tool.LoadMod(path); err == nil {
				fn = func(ext string) (ok bool, class bool) {
					switch ext {
					case ".go", ".code":
						ok = true
					case ".gox", ".spx", ".gmx":
						ok, class = true, true
					default:
						class = mod.IsClass(ext)
						ok = class
					}
					return
				}
			} else {
				fn = func(ext string) (ok bool, class bool) {
					switch ext {
					case ".go", ".code":
						ok = true
					case ".gox", ".spx", ".gmx":
						ok, class = true, true
					}
					return
				}
			}
			w.dirMap[dir] = fn
		}
		ext := filepath.Ext(path)
		smart := *flagSmart
		mvgo := smart && *flagMoveGo
		if ok, class := fn(ext); ok && (!mvgo || ext == ".go") {
			procCnt++
			if *flagNotExec {
				fmt.Println("gop fmt", path)
			} else {
				err = gopfmt(path, class, smart && (mvgo || ext != ".go"), mvgo)
				if err != nil {
					report(err)
				}
			}
		}
	}
	return err
}

func report(err error) {
	fmt.Println(err)
	os.Exit(2)
}

func runCmd(cmd *base.Command, args []string) {
	err := flag.Parse(args)
	if err != nil {
		log.Fatalln("parse input arguments failed:", err)
	}
	narg := flag.NArg()
	if narg < 1 {
		cmd.Usage(os.Stderr)
	}
	if *flagTest {
		defer func() {
			if testErrCnt > 0 {
				fmt.Printf("total %d files are not formatted.\n", testErrCnt)
				os.Exit(1)
			}
		}()
	}
	walker := newWalker()
	for i := 0; i < narg; i++ {
		path := flag.Arg(i)
		walkSubDir = strings.HasSuffix(path, "/...")
		if walkSubDir {
			path = path[:len(path)-4]
		}
		procCnt = 0
		rootDir = path
		filepath.WalkDir(path, walker.walk)
		if procCnt == 0 {
			fmt.Println("no Go+ files in", path)
		}
	}
}

// -----------------------------------------------------------------------------
