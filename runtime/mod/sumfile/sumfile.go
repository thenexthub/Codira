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

package sumfile

import (
	"os"
	"sort"
	"strings"
)

type File struct {
	lines []string
	gosum string
}

func Load(gosum string) (sumf *File, err error) {
	var lines []string
	b, err := os.ReadFile(gosum)
	if err != nil {
		if !os.IsNotExist(err) {
			return
		}
	} else {
		text := string(b)
		lines = strings.Split(strings.TrimRight(text, "\n"), "\n")
	}
	return &File{lines, gosum}, nil
}

func (p *File) Save() (err error) {
	n := 0
	for _, line := range p.lines {
		n += 1 + len(line)
	}
	b := make([]byte, 0, n)
	for _, line := range p.lines {
		b = append(b, line...)
		b = append(b, '\n')
	}
	return os.WriteFile(p.gosum, b, 0666)
}

func (p *File) Lookup(modPath string) []string {
	prefix := modPath + " "
	lines := p.lines
	for i, line := range lines {
		if line > prefix {
			if strings.HasPrefix(line, prefix) {
				for j, line := range lines[i+1:] {
					if !strings.HasPrefix(line, prefix) {
						lines = lines[:i+1+j]
						break
					}
				}
				return lines[i:]
			}
			break
		}
	}
	return nil
}

func (p *File) Add(lines []string) {
	p.lines = append(p.lines, lines...)
	sort.Strings(p.lines)
}
