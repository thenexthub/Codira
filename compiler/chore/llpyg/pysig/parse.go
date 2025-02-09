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

package pysig

import (
	"strings"
)

type Arg struct {
	Name   string
	Type   string
	DefVal string
}

// Parse parses a Python function signature.
func Parse(sig string) (args []*Arg) {
	sig = strings.TrimPrefix(sig, "(")
	for {
		pos := strings.IndexAny(sig, ",:=)")
		if pos <= 0 {
			return
		}
		arg := &Arg{Name: strings.TrimSpace(sig[:pos])}
		args = append(args, arg)
		c := sig[pos]
		sig = sig[pos+1:]
		switch c {
		case ',':
			continue
		case ':':
			arg.Type, sig = parseType(sig)
			if strings.HasPrefix(sig, "=") {
				arg.DefVal, sig = parseDefVal(sig[1:])
			}
		case '=':
			arg.DefVal, sig = parseDefVal(sig)
		case ')':
			return
		}
		sig = strings.TrimPrefix(sig, ",")
	}
}

const (
	allSpecials = "([<'\""
)

var pairStops = map[byte]string{
	'(':  ")" + allSpecials,
	'[':  "]" + allSpecials,
	'<':  ">" + allSpecials,
	'\'': "'" + allSpecials,
	'"':  "\"",
}

func parseText(sig string, stops string) (left string) {
	for {
		pos := strings.IndexAny(sig, stops)
		if pos < 0 {
			return sig
		}
		if c := sig[pos]; c != stops[0] {
			if pstop, ok := pairStops[c]; ok {
				sig = strings.TrimPrefix(parseText(sig[pos+1:], pstop), pstop[:1])
				continue
			}
		}
		return sig[pos:]
	}
}

// stops: "=,)"
func parseType(sig string) (string, string) {
	left := parseText(sig, "=,)"+allSpecials)
	return resultOf(sig, left), left
}

// stops: ",)"
func parseDefVal(sig string) (string, string) {
	left := parseText(sig, ",)"+allSpecials)
	return resultOf(sig, left), left
}

func resultOf(sig, left string) string {
	return strings.TrimSpace(sig[:len(sig)-len(left)])
}
