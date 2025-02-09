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

package ydb

import (
	"fmt"
	"log"
	"reflect"
	"strings"
	"unicode/utf8"

	"github.com/qiniu/x/ctype"
)

// -----------------------------------------------------------------------------

func recoverErr(e any) error {
	if v, ok := e.(error); ok {
		return v
	}
	return fmt.Errorf("%v", e)
}

// -----------------------------------------------------------------------------

func isSlice(v any) bool {
	return reflect.ValueOf(v).Kind() == reflect.Slice
}

func checkArgSlice(args []any) int {
	iArgSlice := -1
	for i, arg := range args {
		if isSlice(arg) {
			if iArgSlice >= 0 {
				log.Panicf(
					"query: multiple arguments (%dth, %dth) are slices (only one can be)\n",
					iArgSlice+1, i+1,
				)
			}
			iArgSlice = i
		}
	}
	return iArgSlice
}

// -----------------------------------------------------------------------------

func (p *Class) tblFromNames(names []string) (tbl string) {
	var v string
	tbl, names[0] = tblFromName(names[0])
	for i := 1; i < len(names); i++ {
		if v, names[i] = tblFromName(names[i]); v != tbl {
			log.Panicln("insert: multiple tables")
		}
	}
	if tbl == "" {
		tbl = p.tbl
	}
	return
}

func tblFromName(name string) (string, string) {
	if pos := strings.IndexByte(name, '.'); pos > 0 {
		return name[:pos], name[pos+1:]
	}
	return "", name
}

// -----------------------------------------------------------------------------

func (p *Class) exprTblname(cond string) string {
	tbls := exprTblnames(cond)
	tbl := ""
	switch len(tbls) {
	case 0:
	case 1:
		tbl = tbls[0]
	default:
		log.Panicln("query currently doesn't support multiple tables")
	}
	if tbl == "" {
		tbl = p.tbl
	}
	return tbl
}

func exprTblnames(expr string) (tbls []string) {
	for expr != "" {
		pos := ctype.ScanCSymbol(expr)
		if pos != 0 {
			name := ""
			if pos > 0 {
				switch expr[pos] {
				case '.':
					name = expr[:pos]
					expr = ctype.SkipCSymbol(expr[pos+1:])
				case '(': // function call, eg. SUM(...)
					expr = expr[pos+1:]
					continue
				default:
					expr = expr[pos:]
				}
			} else {
				expr = ""
			}
			switch name {
			case "AND", "OR":
			default:
				tbls = addTblname(tbls, name)
			}
			continue
		}
		pos = ctype.ScanTypeEx(ctype.FLOAT_FIRST_CHAT, ctype.CSYMBOL_NEXT_CHAR, expr)
		if pos == 0 {
			c, size := utf8.DecodeRuneInString(expr)
			switch c {
			case '\'':
				expr = skipStringConst(expr[1:], '\'')
			default:
				expr = expr[size:]
			}
		} else if pos < 0 {
			break
		} else {
			expr = expr[pos:]
		}
	}
	return
}

func skipStringConst(next string, quot rune) string {
	skip := false
	for i, c := range next {
		if skip {
			skip = false
		} else if c == '\\' {
			skip = true
		} else if c == quot {
			return next[i+1:]
		}
	}
	return ""
}

func addTblname(tbls []string, tbl string) []string {
	for _, v := range tbls {
		if v == tbl {
			return tbls
		}
	}
	return append(tbls, tbl)
}

// -----------------------------------------------------------------------------
