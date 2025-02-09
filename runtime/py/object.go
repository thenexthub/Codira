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

	"github.com/goplus/llgo/c"
)

// https://docs.python.org/3/c-api/object.html

// Object represents a Python object.
type Object struct {
	Unused [8]byte
}

// llgo:link (*Object).DecRef C.Py_DecRef
func (o *Object) DecRef() {}

// llgo:link (*Object).Type C.PyObject_Type
func (o *Object) Type() *Object { return nil }

// Compute a string representation of object o. Returns the string representation on
// success, nil on failure. This is the equivalent of the Python expression str(o).
// Called by the str() built-in function and, therefore, by the print() function.
//
// llgo:link (*Object).Str C.PyObject_Str
func (o *Object) Str() *Object { return nil }

// Returns 1 if the object o is considered to be true, and 0 otherwise. This is equivalent
// to the Python expression not not o. On failure, return -1.
//
// llgo:link (*Object).IsTrue C.PyObject_IsTrue
func (o *Object) IsTrue() c.Int { return -1 }

// Returns 0 if the object o is considered to be true, and 1 otherwise. This is equivalent
// to the Python expression not o. On failure, return -1.
//
// llgo:link (*Object).NotTrue C.PyObject_Not
func (o *Object) NotTrue() c.Int { return -1 }

// -----------------------------------------------------------------------------

// Retrieve an attribute named attrName from object o. Returns the attribute value on success,
// or nil on failure. This is the equivalent of the Python expression o.attrName.
//
// llgo:link (*Object).GetAttr C.PyObject_GetAttr
func (o *Object) GetAttr(attrName *Object) *Object { return nil }

// llgo:link (*Object).GetAttrString C.PyObject_GetAttrString
func (o *Object) GetAttrString(attrName *c.Char) *Object { return nil }

// -----------------------------------------------------------------------------
