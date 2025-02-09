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

package scanner

import (
	"go/scanner"
	"io"

	"github.com/goplus/gop/token"
	"github.com/qiniu/x/byteutil"
)

// -----------------------------------------------------------------------------

// Error is an alias of go/scanner.Error
type Error = scanner.Error

// ErrorList is an alias of go/scanner.ErrorList
type ErrorList = scanner.ErrorList

// PrintError is a utility function that prints a list of errors to w,
// one error per line, if the err parameter is an ErrorList. Otherwise
// it prints the err string.
func PrintError(w io.Writer, err error) {
	scanner.PrintError(w, err)
}

// -----------------------------------------------------------------------------

// New creates a scanner to tokenize the text src.
//
// Calls to Scan will invoke the error handler err if they encounter a
// syntax error and err is not nil. Also, for each error encountered,
// the Scanner field ErrorCount is incremented by one. The mode parameter
// determines how comments are handled.
func New(src string, err ErrorHandler, mode Mode) *Scanner {
	fset := token.NewFileSet()
	file := fset.AddFile("", fset.Base(), len(src))
	scanner := new(Scanner)
	scanner.Init(file, byteutil.Bytes(src), err, mode)
	return scanner
}

// -----------------------------------------------------------------------------
