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
	"context"
	"database/sql"
	"log"
	"reflect"
)

// -----------------------------------------------------------------------------

// Delete deltes rows by cond.
//   - delete <cond>, <arg1>, <arg2>, ...
func (p *Class) Delete(cond string, args ...any) (sql.Result, error) {
	tbl := p.exprTblname(cond)
	query := makeDeleteExpr(tbl, cond)
	iArgSlice := checkArgSlice(args)
	if iArgSlice >= 0 {
		return p.deleteMulti(context.TODO(), query, iArgSlice, args)
	}
	return p.deleteOne(context.TODO(), query, args)
}

func makeDeleteExpr(tbl string, cond string) string {
	query := make([]byte, 0, 64)
	query = append(query, "DELETE FROM "...)
	query = append(query, tbl...)
	query = append(query, " WHERE "...)
	query = append(query, cond...)
	return string(query)
}

func (p *Class) deleteRet(result sql.Result, err error) (sql.Result, error) {
	if err != nil {
		p.handleErr("delete:", err)
	}
	return result, err
}

func (p *Class) deleteOne(ctx context.Context, query string, args []any) (sql.Result, error) {
	if debugExec {
		log.Println("==>", query, args)
	}
	result, err := p.db.ExecContext(ctx, query, args...)
	return p.deleteRet(result, err)
}

func (p *Class) deleteMulti(ctx context.Context, query string, iArgSlice int, args []any) (sql.Result, error) {
	argSlice := args[iArgSlice]
	defer func() {
		args[iArgSlice] = argSlice
	}()
	vArgSlice := reflect.ValueOf(argSlice)
	nAffected := deleteResult(0)
	for i, n := 0, vArgSlice.Len(); i < n; i++ {
		arg := vArgSlice.Index(i).Interface()
		args[iArgSlice] = arg
		result, err := p.deleteOne(ctx, query, args)
		if err != nil {
			return nAffected, err
		}
		if v, e := result.RowsAffected(); e == nil {
			nAffected += deleteResult(v)
		}
	}
	return nAffected, nil
}

// -----------------------------------------------------------------------------

type deleteResult int64

func (n deleteResult) LastInsertId() (int64, error) {
	panic("not impl")
}

func (n deleteResult) RowsAffected() (int64, error) {
	return int64(n), nil
}

// -----------------------------------------------------------------------------
