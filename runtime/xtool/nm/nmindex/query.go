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

package nmindex

import (
	"bufio"
	"os"
	"strings"

	"github.com/goplus/llgo/xtool/nm"
)

// MatchedItem represents a matched item
type MatchedItem struct {
	ObjFile string
	Symbol  string
	Type    nm.SymbolType
}

// MatchedFile represents a matched file
type MatchedFile struct {
	ArFile string
	Items  []*MatchedItem
}

// Query queries symbol in index files (allow wildcard).
func Query(dir string, query string) (files []*MatchedFile, err error) {
	fis, err := os.ReadDir(dir)
	if err != nil {
		return
	}
	dir += "/"
	for _, fi := range fis {
		if fi.IsDir() {
			continue
		}
		idxFile := fi.Name()
		if !strings.HasSuffix(idxFile, ".pub") {
			continue
		}
		files = queryIndex(files, dir+fi.Name(), query)
	}
	return
}

func queryIndex(files []*MatchedFile, idxFile, query string) []*MatchedFile {
	f, err := os.Open(idxFile)
	if err != nil {
		return files
	}
	defer f.Close()

	r := bufio.NewReader(f)
	line, err := r.ReadString('\n')
	if err != nil || !strings.HasPrefix(line, "nm ") {
		return files
	}
	var items []*MatchedItem
	arFile := line[3 : len(line)-1]
	objFile := ""
	query, flags := parseQuery(query)
	for {
		line, err = r.ReadString('\n')
		if err != nil {
			break
		}
		if strings.HasPrefix(line, "file ") {
			objFile = line[5 : len(line)-1]
			continue
		}
		typ := line[0]
		sym := line[2 : len(line)-1]
		if !match(sym, query, flags) {
			continue
		}
		items = append(items, &MatchedItem{
			ObjFile: objFile,
			Symbol:  sym,
			Type:    nm.SymbolType(typ),
		})
	}
	if len(items) > 0 {
		files = append(files, &MatchedFile{ArFile: arFile, Items: items})
	}
	return files
}

const (
	flagSuffix = 1 << iota
	flagPrefix
)

func parseQuery(query string) (text string, flags int) {
	if strings.HasSuffix(query, "*") {
		query = query[:len(query)-1]
		flags = flagPrefix
	}
	if strings.HasPrefix(query, "*") {
		query = query[1:]
		flags |= flagSuffix
	}
	text = query
	return
}

func match(s, query string, flags int) bool {
	switch flags {
	case 0:
		return s == query
	case flagPrefix:
		return strings.HasPrefix(s, query)
	case flagSuffix:
		return strings.HasSuffix(s, query)
	default:
		return strings.Contains(s, query)
	}
}
