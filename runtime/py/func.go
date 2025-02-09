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

package py

import (
	_ "unsafe"
)

// https://docs.python.org/3/c-api/function.html

// Return a new function object associated with the code object code.
// globals must be a dictionary with the global variables accessible
// to the function.
//
// The function’s docstring and name are retrieved from the code object.
// __module__ is retrieved from globals. The argument defaults, annotations
// and closure are set to nil. __qualname__ is set to the same value as
// the code object’s co_qualname field.
//
//go:linkname NewFunc C.PyFunction_New
func NewFunc(code, globals *Object) *Object

// As NewFunc, but also allows setting the function object’s __qualname__
// attribute. qualname should be a unicode object or nil; if nil, the
// __qualname__ attribute is set to the same value as the code object’s
// co_qualname field.
//
//go:linkname NewFuncWithQualName C.PyFunction_NewWithQualName
func NewFuncWithQualName(code, globals, qualname *Object) *Object

/*
// Return true if o is a function object (has type PyFunction_Type). The
// parameter must not be nil. This function always succeeds.
//
//- llgo:link (*Object).FuncCheck C.PyFunction_Check
func (o *Object) FuncCheck() c.Int { return 0 }
*/

// Return the code object associated with the function object op.
//
// llgo:link (*Object).FuncCode C.PyFunction_GetCode
func (f *Object) FuncCode() *Object { return nil }
