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

package builtin

const (
	GopPackage = true // to indicate this is a Go+ package
)

// -----------------------------------------------------------------------------

type IntRange struct {
	Start, End, Step int
}

func NewRange__0(start, end, step int) *IntRange {
	return &IntRange{Start: start, End: end, Step: step}
}

func (p *IntRange) Gop_Enum() *intRangeIter {
	step := p.Step
	n := p.End - p.Start + step
	if step > 0 {
		n = (n - 1) / step
	} else {
		n = (n + 1) / step
	}
	return &intRangeIter{n: n, val: p.Start, step: p.Step}
}

// -----------------------------------------------------------------------------

type intRangeIter struct {
	n, val, step int
}

func (p *intRangeIter) Next() (val int, ok bool) {
	if p.n > 0 {
		val, ok = p.val, true
		p.val += p.step
		p.n--
	}
	return
}

// -----------------------------------------------------------------------------
