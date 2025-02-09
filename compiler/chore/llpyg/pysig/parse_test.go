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

import "testing"

func TestParse(t *testing.T) {
	type testCase struct {
		sig  string
		args []*Arg
	}
	cases := []testCase{
		{"(start=None, *, unit: 'str | None' = None) -> 'TimedeltaIndex'", []*Arg{
			{Name: "start", DefVal: "None"},
			{Name: "*"},
			{Name: "unit", Type: "'str | None'", DefVal: "None"},
		}},
		{"()", nil},
		{"(a =", []*Arg{{Name: "a"}}},
		{"(a) -> int", []*Arg{{Name: "a"}}},
		{"(a: int)", []*Arg{{Name: "a", Type: "int"}}},
		{"(a: int = 1, b: float)", []*Arg{{Name: "a", Type: "int", DefVal: "1"}, {Name: "b", Type: "float"}}},
		{"(a = <1>, b = 2.0)", []*Arg{{Name: "a", DefVal: "<1>"}, {Name: "b", DefVal: "2.0"}}},
		{"(a: 'Suffixes' = ('_x', '_y'))", []*Arg{{Name: "a", Type: "'Suffixes'", DefVal: "('_x', '_y')"}}},
	}
	for _, c := range cases {
		args := Parse(c.sig)
		if len(args) != len(c.args) {
			t.Fatalf("%s: len(args) = %v, want %v", c.sig, len(args), len(c.args))
		}
		for i, arg := range args {
			want := c.args[i]
			if arg.Name != want.Name || arg.Type != want.Type || arg.DefVal != want.DefVal {
				t.Fatalf("%s: args[%v] = %v, want %v", c.sig, i, arg, want)
			}
		}
	}
}
