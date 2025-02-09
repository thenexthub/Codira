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

package modfile

import (
	"fmt"
	"path"
	"strings"

	"github.com/qiniu/x/errors"
)

// can be "_[class].gox" or ".[class]"
func isExt(s string) bool {
	return len(s) > 1 && (s[0] == '_' || s[0] == '.')
}

func parseExt(s *string) (t string, err error) {
	t, err = parseString(s)
	if err != nil {
		goto failed
	}
	if isExt(t) {
		return
	}
	err = errors.New("invalid ext format")
failed:
	return "", &InvalidExtError{
		Ext: *s,
		Err: err,
	}
}

type InvalidExtError struct {
	Ext string
	Err error
}

func (e *InvalidExtError) Error() string {
	return fmt.Sprintf("ext %s invalid: %s", e.Ext, e.Err)
}

func (e *InvalidExtError) Unwrap() error { return e.Err }

// SplitFname splits fname into (className, classExt).
func SplitFname(fname string) (className, classExt string) {
	classExt = path.Ext(fname)
	className = fname[:len(fname)-len(classExt)]
	if hasGoxExt := (classExt == ".gox"); hasGoxExt {
		if n := strings.LastIndexByte(className, '_'); n > 0 {
			className, classExt = fname[:n], fname[n:]
		}
	}
	return
}

// ClassExt returns classExt of specified fname.
func ClassExt(fname string) string {
	_, ext := SplitFname(fname)
	return ext
}
