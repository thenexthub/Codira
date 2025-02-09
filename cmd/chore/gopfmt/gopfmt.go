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

package main

import (
	"bytes"
	"flag"
	"fmt"
	"go/parser"
	"go/token"
	"io"
	"io/fs"
	"os"
	"path/filepath"
	"strings"

	goformat "go/format"

	"github.com/goplus/gop/format"
	"github.com/goplus/gop/tool"
)

var (
	procCnt    = 0
	walkSubDir = false
	rootDir    = ""
)

var (
	// main operation modes
	write = flag.Bool("w", false, "write result to (source) file instead of stdout")
)

func usage() {
	fmt.Fprintf(os.Stderr, "usage: gopfmt [flags] [path ...]\n")
	flag.PrintDefaults()
}

func report(err error) {
	fmt.Println(err)
	os.Exit(2)
}

func processFile(filename string, class bool, in io.Reader, out io.Writer) error {
	if in == nil {
		var err error
		in, err = os.Open(filename)
		if err != nil {
			return err
		}
	}
	src, err := io.ReadAll(in)
	if err != nil {
		return err
	}
	var res []byte
	if filepath.Ext(filename) == ".go" {
		fset := token.NewFileSet()
		f, err := parser.ParseFile(fset, filename, src, parser.ParseComments)
		if err != nil {
			return err
		}
		var buf bytes.Buffer
		err = goformat.Node(&buf, fset, f)
		if err != nil {
			return err
		}
		res = buf.Bytes()
	} else {
		res, err = format.Source(src, class, filename)
	}
	if err != nil {
		return err
	}

	if *write {
		dir, file := filepath.Split(filename)
		f, err := os.CreateTemp(dir, file)
		if err != nil {
			return err
		}
		tmpfile := f.Name()
		_, err = f.Write(res)
		f.Close()
		if err != nil {
			return err
		}
		err = os.Remove(filename)
		if err != nil {
			return err
		}
		return os.Rename(tmpfile, filename)
	}
	if !*write {
		_, err = out.Write(res)
	}

	return err
}

type walker struct {
	dirMap map[string]func(ext string) (ok bool, class bool)
}

func newWalker() *walker {
	return &walker{dirMap: make(map[string]func(ext string) (ok bool, class bool))}
}

func (w *walker) walk(path string, d fs.DirEntry, err error) error {
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
	} else if d.IsDir() {
		if !walkSubDir && path != rootDir {
			return filepath.SkipDir
		}
	} else {
		// Directories are walked, ignoring non-Gop files.
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
		if ok, class := fn(ext); ok {
			procCnt++
			if err = processFile(path, class, nil, os.Stdout); err != nil {
				report(err)
			}
		}
	}
	return err
}

func main() {
	flag.Usage = usage
	flag.Parse()

	args := flag.Args()
	if len(args) == 0 {
		if *write {
			report(fmt.Errorf("error: cannot use -w with standard input"))
			return
		}
		if err := processFile("<standard input>", false, os.Stdin, os.Stdout); err != nil {
			report(err)
		}
		return
	}
	walker := newWalker()
	for _, path := range args {
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
