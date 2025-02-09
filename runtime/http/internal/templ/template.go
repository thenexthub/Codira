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

package templ

import (
	"strings"
)

func Translate(text string) string {
	var b strings.Builder
	if TranslateEx(&b, text, "{{", "}}") {
		return b.String()
	}
	return text
}

type iBuilder interface {
	Grow(n int)
	WriteString(s string) (int, error)
	String() string
}

func TranslateEx[Builder iBuilder](b Builder, text, delimLeft, delimRight string) bool {
	offs := make([]int, 0, 16)
	base := 0
	for {
		pos := strings.Index(text[base:], delimLeft)
		if pos < 0 {
			break
		}
		begin := base + pos + 2 // script begin
		n := strings.Index(text[begin:], delimRight)
		if n < 0 {
			n = len(text) - begin // script length
		}
		base = begin + n
		code := text[begin:base]
		nonBlank := false
		for i := 0; i < n; i++ {
			c := code[i]
			if !isSpace(c) {
				nonBlank = true
			} else if c == '\n' && nonBlank {
				off := begin + i
				if i, nonBlank = findScript(code, i+1, n); !nonBlank {
					break // script not found
				}
				offs = append(offs, off) // insert }}{{
			}
		}
	}
	n := len(offs)
	if n == 0 {
		return false
	}
	b.Grow(len(text) + n*4)
	base = 0
	delimRightLeft := delimRight + delimLeft
	for i := 0; i < n; i++ {
		off := offs[i]
		b.WriteString(text[base:off])
		b.WriteString(delimRightLeft)
		base = off
	}
	b.WriteString(text[base:])
	return true
}

func isSpace(c byte) bool {
	switch c {
	case ' ', '\t', '\n', '\v', '\f', '\r', 0x85, 0xA0:
		return true
	}
	return false
}

func findScript(code string, i, n int) (int, bool) {
	for i < n {
		if !isSpace(code[i]) {
			return i, true
		}
		i++
	}
	return -1, false
}
