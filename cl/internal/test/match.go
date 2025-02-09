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

package test

const (
	GopPackage = true
)

type basetype interface {
	string | int | bool | float64
}

type Case struct {
	CaseT
}

const (
	Gopo_Gopt_Case_Match = "Gopt_Case_MatchTBase,Gopt_Case_MatchAny"
)

func Gopt_Case_MatchTBase[T basetype](t CaseT, got, expected T, name ...string) {
}

func Gopt_Case_MatchAny(t CaseT, got, expected any, name ...string) {
}
