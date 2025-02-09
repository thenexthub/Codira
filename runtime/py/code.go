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

// https://docs.python.org/3/c-api/code.html

// Equivalent to the Python code getattr(co, 'co_code'). Returns a strong
// reference to a BytesObject representing the bytecode in a code object.
// On error, nil is returned and an exception is raised.
//
// This BytesObject may be created on-demand by the interpreter and does
// not necessarily represent the bytecode actually executed by CPython.
// The primary use case for this function is debuggers and profilers.
//
// llgo:link (*Object).CodeBytes C.PyCode_GetCode
func (o *Object) CodeBytes() *Object { return nil }

// Equivalent to the Python code getattr(co, 'co_varnames'). Returns a new
// reference to a TupleObject containing the names of the local variables.
// On error, nil is returned and an exception is raised.
//
// llgo:link (*Object).CodeVarnames C.PyCode_GetVarnames
func (o *Object) CodeVarnames() *Object { return nil }
