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

// https://docs.python.org/3/c-api/type.html

// Return the type’s name. Equivalent to getting the type’s __name__ attribute.
//
// llgo:link (*Object).TypeName C.PyType_GetName
func (t *Object) TypeName() *Object { return nil }

// Return the tp_flags member of type. This function is primarily meant for use
// with Py_LIMITED_API; the individual flag bits are guaranteed to be stable across
// Python releases, but access to tp_flags itself is not part of the limited API.
//
// llgo:link (*Object).TypeFlags C.PyType_GetFlags
func (t *Object) TypeFlags() uint32 { return 0 }

// Return the module object associated with the given type when the type was created
// using PyType_FromModuleAndSpec().
//
// If no module is associated with the given type, sets TypeError and returns nil.
//
// This function is usually used to get the module in which a method is defined. Note
// that in such a method, Py_TYPE(self).Module() may not return the intended result.
// Py_TYPE(self) may be a subclass of the intended class, and subclasses are not
// necessarily defined in the same module as their superclass. See PyCMethod to get
// the class that defines the method. See ModuleByDef() for cases when PyCMethod
// cannot be used.
//
// llgo:link (*Object).TypeModule C.PyType_GetModule
func (t *Object) TypeModule() *Object { return nil }

// -llgo:link (*Object).TypeModuleByDef C.PyType_GetModuleByDef
// func (t *Object) TypeModuleByDef(def *ModuleDef) *Object { return nil }
