//go:build !go1.19
// +build !go1.19

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

package tokenutil

import (
	"go/token"
	"sync"
	"unsafe"
)

// -----------------------------------------------------------------------------
// File

// A File is a handle for a file belonging to a FileSet.
// A File has a name, size, and line offset table.
type File struct {
	set  *token.FileSet
	name string // file name as provided to AddFile
	base int    // Pos value range for this file is [base...base+size]
	size int    // file size as provided to AddFile

	// lines and infos are protected by mutex
	mutex sync.Mutex
	lines []int // lines contains the offset of the first character for each line (the first entry is always 0)
	infos []lineInfo
}

// A lineInfo object describes alternative file, line, and column
// number information (such as provided via a //line directive)
// for a given file offset.
type lineInfo struct {
	// fields are exported to make them accessible to gob
	Offset       int
	Filename     string
	Line, Column int
}

// Lines returns the effective line offset table of the form described by SetLines.
// Callers must not mutate the result.
func (f *File) Lines() []int {
	f.mutex.Lock()
	lines := f.lines
	f.mutex.Unlock()
	return lines
}

// Lines returns the effective line offset table of the form described by SetLines.
// Callers must not mutate the result.
func Lines(f *token.File) []int {
	file := (*File)(unsafe.Pointer(f))
	return file.Lines()
}

// -----------------------------------------------------------------------------
