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

package gopprojs

import "testing"

// -----------------------------------------------------------------------------

func TestIsLocal(t *testing.T) {
	if !isLocal(".") || !isLocal("/") {
		t.Fatal(`isLocal(".") || isLocal("/")`)
	}
	if !isLocal("c:/foo") {
		t.Fatal(`isLocal("c:/foo")`)
	}
	if !isLocal("C:/foo") {
		t.Fatal(`isLocal("C:/foo")`)
	}
	if isLocal("") {
		t.Fatal(`isLocal("")`)
	}
}

func TestParseOne(t *testing.T) {
	proj, next, err := ParseOne("a.go", "b.go", "abc")
	if err != nil || len(next) != 1 || next[0] != "abc" {
		t.Fatal("ParseOne failed:", proj, next, err)
	}
}

func TestParseAll_wildcard1(t *testing.T) {
	projs, err := ParseAll("*.go")
	if err != nil || len(projs) != 1 {
		t.Fatal("ParseAll failed:", projs, err)
	}
	if proj, ok := projs[0].(*FilesProj); !ok || len(proj.Files) != 1 || proj.Files[0] != "*.go" {
		t.Fatal("ParseAll failed:", projs)
	}
}

func TestParseAll_wildcard2(t *testing.T) {
	projs, err := ParseAll("t/*.go")
	if err != nil || len(projs) != 1 {
		t.Fatal("ParseAll failed:", projs, err)
	}
	if proj, ok := projs[0].(*FilesProj); !ok || len(proj.Files) != 1 || proj.Files[0] != "t/*.go" {
		t.Fatal("ParseAll failed:", projs)
	}
}

func TestParseAll_multiFiles(t *testing.T) {
	projs, err := ParseAll("a.gop", "b.go")
	if err != nil || len(projs) != 1 {
		t.Fatal("ParseAll failed:", projs, err)
	}
	if proj, ok := projs[0].(*FilesProj); !ok || len(proj.Files) != 2 || proj.Files[0] != "a.gop" {
		t.Fatal("ParseAll failed:", proj)
	}
	projs[0].projObj()
}

func TestParseAll_multiProjs(t *testing.T) {
	projs, err := ParseAll("a/...", "./a/...", "/a")
	if err != nil || len(projs) != 3 {
		t.Fatal("ParseAll failed:", projs, err)
	}
	if proj, ok := projs[0].(*PkgPathProj); !ok || proj.Path != "a/..." {
		t.Fatal("ParseAll failed:", proj)
	}
	if proj, ok := projs[1].(*DirProj); !ok || proj.Dir != "./a/..." {
		t.Fatal("ParseAll failed:", proj)
	}
	if proj, ok := projs[2].(*DirProj); !ok || proj.Dir != "/a" {
		t.Fatal("ParseAll failed:", proj)
	}
	for _, proj := range projs {
		proj.projObj()
	}
}

func TestParseAllErr(t *testing.T) {
	_, err := ParseAll("a/...", "./a/...", "/a", "*.go")
	if err != ErrMixedFilesProj {
		t.Fatal("ParseAll:", err)
	}
}

// -----------------------------------------------------------------------------
