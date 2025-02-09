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
package modfile

import (
	"syscall"
	"testing"
)

// -----------------------------------------------------------------------------

var addParseExtTests = []struct {
	desc    string
	ext     string
	want    string
	wantErr string
}{
	{
		"spx ok",
		".spx",
		".spx",
		"",
	},
	{
		"yap ok",
		"_yap.gox",
		"_yap.gox",
		"",
	},
	{
		"not a ext",
		"gmx",
		"",
		"ext gmx invalid: invalid ext format",
	},
}

func TestParseExt(t *testing.T) {
	if (&InvalidExtError{Err: syscall.EINVAL}).Unwrap() != syscall.EINVAL {
		t.Fatal("InvalidExtError.Unwrap failed")
	}
	if (&InvalidSymbolError{Err: syscall.EINVAL}).Unwrap() != syscall.EINVAL {
		t.Fatal("InvalidSymbolError.Unwrap failed")
	}
	for _, tt := range addParseExtTests {
		t.Run(tt.desc, func(t *testing.T) {
			ext, err := parseExt(&tt.ext)
			if err != nil {
				if err.Error() != tt.wantErr {
					t.Fatalf("wanterr: %s, but got: %s", tt.wantErr, err)
				}
			}
			if ext != tt.want {
				t.Fatalf("want: %s, but got: %s", tt.want, ext)
			}
		})
	}
}

func TestIsDirectoryPath(t *testing.T) {
	if !IsDirectoryPath("./...") {
		t.Fatal("IsDirectoryPath failed")
	}
}

func TestFormat(t *testing.T) {
	if b := Format(&FileSyntax{}); len(b) != 0 {
		t.Fatal("Format failed:", b)
	}
}

func TestForma2t(t *testing.T) {
	f := New("/foo/gop.mod", "1.2.0")
	if b := string(Format(f.Syntax)); b != "gop 1.2.0\n" {
		t.Fatal("Format failed:", b)
	}
}

func TestMustQuote(t *testing.T) {
	if !MustQuote("") {
		t.Fatal("MustQuote failed")
	}
}

// -----------------------------------------------------------------------------
