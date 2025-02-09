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

package ar

import (
	"errors"
	"time"
)

var (
	errInvalidHeader = errors.New("ar: invalid header")
	errWriteTooLong  = errors.New("ar: write too long")
)

const (
	globalHeader    = "!<arch>\n"
	globalHeaderLen = len(globalHeader)
	headerByteSize  = 60
)

type recHeader struct {
	name    [16]byte
	modTime [12]byte
	uid     [6]byte
	gid     [6]byte
	mode    [8]byte
	size    [10]byte
	eol     [2]byte
}

type Header struct {
	Name    string
	ModTime time.Time
	Uid     int
	Gid     int
	Mode    int64
	Size    int64
}

type slicer []byte

func (sp *slicer) next(n int) (b []byte) {
	s := *sp
	b, *sp = s[0:n], s[n:]
	return
}
