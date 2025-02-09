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

package mysql

import (
	"log"
	"os"

	"github.com/go-sql-driver/mysql"
	"github.com/goplus/yap/ydb"
)

// Register registers a user-defined testDataSource for `mysql` engine.
// eg. testDataSource = `root@/test?autodrop`
func Register(testDataSource string) {
	ydb.Register(&ydb.Engine{
		Name:       "mysql",
		TestSource: testDataSource,
		WrapErr:    wrapErr,
	})
}

func init() {
	ydb.Register(&ydb.Engine{
		Name:       "mysql",
		TestSource: testDataSource,
		WrapErr:    wrapErr,
	})
}

const (
	ER_DUP_ENTRY = 1062
)

func wrapErr(prompt string, err error) error {
	if prompt == "insert:" {
		if e, ok := err.(*mysql.MySQLError); ok && e.Number == ER_DUP_ENTRY {
			return ydb.ErrDuplicated
		}
	}
	return err
}

func testDataSource() string {
	dataSource := os.Getenv("YDB_MYSQL_TEST")
	if dataSource == "" {
		log.Panicln("env `YDB_MYSQL_TEST` not found, please set it before running")
	}
	return dataSource
}
