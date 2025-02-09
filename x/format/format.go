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

package format

import (
	"github.com/goplus/gop/ast"
	"github.com/goplus/gop/token"
)

// -----------------------------------------------------------------------------

var printFuncs = [][2]string{
	{"Errorf", "errorf"},
	{"Print", "print"},
	{"Printf", "printf"},
	{"Println", "println"},
	{"Fprint", "fprint"},
	{"Fprintf", "fprintf"},
	{"Fprintln", "fprintln"},
	{"Sprint", "sprint"},
	{"Sprintf", "sprintf"},
	{"Sprintln", "sprintln"},
}

func fmtToBuiltin(ctx *importCtx, sel *ast.Ident, ref *ast.Expr) bool {
	if ctx.pkgPath == "fmt" {
		for _, fns := range printFuncs {
			if fns[0] == sel.Name || fns[1] == sel.Name {
				name := fns[1]
				if name == "println" {
					name = "echo"
				}
				*ref = &ast.Ident{NamePos: sel.NamePos, Name: name}
				return true
			}
		}
	}
	return false
}

// -----------------------------------------------------------------------------

func commandStyleFirst(v *ast.CallExpr) {
	switch v.Fun.(type) {
	case *ast.Ident, *ast.SelectorExpr:
		if v.NoParenEnd == token.NoPos {
			v.NoParenEnd = v.Rparen
		}
	}
}

// -----------------------------------------------------------------------------

func fncallStartingLowerCase(v *ast.CallExpr) {
	switch fn := v.Fun.(type) {
	case *ast.SelectorExpr:
		startWithLowerCase(fn.Sel)
	}
}

// -----------------------------------------------------------------------------

func funcLitToLambdaExpr(v *ast.FuncLit, ret *ast.Expr) {
	nres, named := checkResult(v.Type.Results)
	if len(named) > 0 {
		return
	}
	var lsh []*ast.Ident
	for _, p := range v.Type.Params.List {
		if p.Names == nil {
			lsh = append(lsh, ast.NewIdent("_"))
		} else {
			lsh = append(lsh, p.Names...)
		}
	}
	if len(v.Body.List) == 1 {
		if stmt, ok := v.Body.List[0].(*ast.ReturnStmt); ok && len(stmt.Results) == nres {
			*ret = &ast.LambdaExpr{First: v.Pos(), Last: v.Pos(), Lhs: lsh, Rhs: stmt.Results, LhsHasParen: len(lsh) > 1, RhsHasParen: len(stmt.Results) > 1}
			return
		}
	}
	*ret = &ast.LambdaExpr2{Lhs: lsh, Body: v.Body, LhsHasParen: len(lsh) > 1}
}

func checkResult(v *ast.FieldList) (nres int, named []*ast.Ident) {
	if v != nil {
		for _, f := range v.List {
			if f.Names == nil {
				nres++
			} else {
				nres += len(f.Names)
				named = append(named, f.Names...)
			}
		}
	}
	return
}

// -----------------------------------------------------------------------------
