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

// https://docs.python.org/3/c-api/set.html

// Return a new set containing objects returned by the iterable. The iterable
// may be nil to create a new empty set. Return the new set on success or nil
// on failure. set a TypeError exception if iterable is not actually iterable.
// The constructor is also useful for copying a set (c=set(s)).
//
//go:linkname NewSet C.PySet_New
func NewSet(iterable *Object) *Object { return nil }

// Return the length of a set or frozenset object. Equivalent to len(anyset).
// Set a SystemError if anyset is not a set, frozenset, or an instance of a
// subtype.
//
// llgo:link (*Object).SetLen C.PySet_Size
func (s *Object) SetLen() int { return 0 }

// Return 1 if found, 0 if not found, and -1 if an error is encountered.
// Unlike the Python __contains__() method, this function does not automatically
// convert unhashable sets into temporary frozensets. Set a TypeError if the key
// is unhashable. Set SystemError if s is not a set, frozenset, or an instance
// of a subtype.
//
// llgo:link (*Object).SetContains C.PySet_Contains
func (s *Object) SetContains(key *Object) int { return 0 }

// Add key to a set instance. Also works with frozenset instances (like
// PyTuple_SetItem() it can be used to fill in the values of brand new
// frozensets before they are exposed to other code). Return 0 on success or -1
// on failure. Set a TypeError if the key is unhashable. Set a MemoryError if
// there is no room to grow. Set a SystemError if set is not an instance of set
// or its subtype.
//
// llgo:link (*Object).SetAdd C.PySet_Add
func (s *Object) SetAdd(key *Object) int { return 0 }

// Return 1 if found and removed, 0 if not found (no action taken), and -1 if an
// error is encountered. Does not set KeyError for missing keys. set a TypeError
// if the key is unhashable. Unlike the Python discard() method, this function
// does not automatically convert unhashable sets into temporary frozensets.
// Set SystemError if set is not an instance of set or its subtype.
//
// llgo:link (*Object).SetDiscard C.PySet_Discard
func (s *Object) SetDiscard(key *Object) int { return 0 }

// Return a new reference to an arbitrary object in the set, and removes the
// object from the set. Return nil on failure. Set KeyError if the set is empty.
// Set a SystemError if set is not an instance of set or its subtype.
//
// llgo:link (*Object).SetPop C.PySet_Pop
func (s *Object) SetPop() *Object { return nil }

// Empty an existing set of all elements. Return 0 on success. Return -1 and
// set SystemError if set is not an instance of set or its subtype.
//
// llgo:link (*Object).SetClear C.PySet_Clear
func (s *Object) SetClear() *Object { return nil }
