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

package memfs

import (
	"bytes"
	"io"
	"io/fs"
	"os"
	"path/filepath"
)

type FileFS struct {
	data []byte
	info fs.DirEntry
}

type dirEntry struct {
	fs.FileInfo
}

func (p *dirEntry) Type() fs.FileMode {
	return p.FileInfo.Mode().Type()
}

func (p *dirEntry) Info() (fs.FileInfo, error) {
	return p.FileInfo, nil
}

func File(filename string, src interface{}) (f *FileFS, err error) {
	var data []byte
	var info fs.DirEntry
	if src != nil {
		data, err = readSource(src)
		if err != nil {
			return
		}
		info = &memFileInfo{name: filename, size: len(data)}
	} else {
		fi, e := os.Stat(filename)
		if e != nil {
			return nil, e
		}
		data, err = os.ReadFile(filename)
		if err != nil {
			return
		}
		info = &dirEntry{fi}
	}
	return &FileFS{data: data, info: info}, nil
}

func (p *FileFS) ReadDir(dirname string) ([]fs.DirEntry, error) {
	return []fs.DirEntry{p.info}, nil
}

func (p *FileFS) ReadFile(filename string) ([]byte, error) {
	return p.data, nil
}

func (p *FileFS) Join(elem ...string) string {
	return filepath.Join(elem...)
}

func (p *FileFS) Base(filename string) string {
	return filepath.Base(filename)
}

func (p *FileFS) Abs(path string) (string, error) {
	return path, nil
}

func readSource(src interface{}) ([]byte, error) {
	switch s := src.(type) {
	case string:
		return []byte(s), nil
	case []byte:
		return s, nil
	case *bytes.Buffer:
		// is io.Reader, but src is already available in []byte form
		if s != nil {
			return s.Bytes(), nil
		}
	case io.Reader:
		return io.ReadAll(s)
	}
	return nil, os.ErrInvalid
}
