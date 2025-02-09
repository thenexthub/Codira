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

package token

import (
	"reflect"
	"testing"
)

func TestArrowOp(t *testing.T) {
	if v := BIDIARROW.IsOperator(); !v {
		t.Fatal("BIDIARROW not op?")
	}
	if v := SRARROW.IsOperator(); !v {
		t.Fatal("SRARROW not op?")
	}
	if BIDIARROW.Precedence() != NEQ.Precedence() {
		t.Fatal("BIDIARROW.Precedence")
	}
	if v := BIDIARROW.String(); v != "<>" {
		t.Fatal("BIDIARROW.String:", v)
	}
	if v := (additional_end + 100).String(); v != "token(189)" {
		t.Fatal("token.String:", v)
	}
}

func TestPrecedence(t *testing.T) {
	cases := map[Token]int{
		LOR:   1,
		LAND:  2,
		EQL:   3,
		SUB:   4,
		MUL:   5,
		ARROW: LowestPrec,
	}
	for op, prec := range cases {
		if v := op.Precedence(); v != prec {
			t.Fatal("Precedence:", op, v)
		}
	}
}

func TestLookup(t *testing.T) {
	if v := Lookup("type"); v != TYPE {
		t.Fatal("TestLookup type:", v)
	} else if !v.IsKeyword() {
		t.Fatal("v.IsKeyword:", v)
	}
	if v := Lookup("new"); v != IDENT {
		t.Fatal("TestLookup new:", v)
	} else if !v.IsLiteral() {
		t.Fatal("v.IsLiteral:", v)
	}
}

func TestBasic(t *testing.T) {
	if !IsExported("Name") {
		t.Fatal("IsExported")
	}
	if !IsKeyword("func") {
		t.Fatal("IsKeyword")
	}
	if !IsIdentifier("new") {
		t.Fatal("IsIdentifier")
	}
}

func TestLines(t *testing.T) {
	fset := NewFileSet()
	f := fset.AddFile("foo.go", 100, 100)
	lines := []int{0, 10, 50}
	f.SetLines(lines)
	ret := Lines(f)
	if !reflect.DeepEqual(ret, lines) {
		t.Fatal("TestLines failed:", ret)
	}
}
