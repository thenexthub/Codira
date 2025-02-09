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

package sqlite3

import (
	"github.com/goplus/yap/ydb"
	"github.com/mattn/go-sqlite3"
)

func wrapErr(prompt string, err error) error {
	if prompt == "insert:" {
		if e, ok := err.(sqlite3.Error); ok && e.Code == sqlite3.ErrConstraint {
			return ydb.ErrDuplicated
		}
	}
	return err
}

func init() {
	ydb.Register(&ydb.Engine{
		Name:       "sqlite3",
		TestSource: "file:test.db?cache=shared&mode=memory",
		WrapErr:    wrapErr,
	})
}
