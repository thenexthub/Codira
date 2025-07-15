//===--- MangleHack.h - Codira Mangler hack for various clients --*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//
//
// Codira support for Interface Builder
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_MANGLEHACK_H
#define LANGUAGE_MANGLEHACK_H

// This returns a C string that must be deallocated with free().
extern "C" const char *
_language_mangleSimpleClass(const char *module, const char *class_);

// This returns a C string that must be deallocated with free().
extern "C" const char *
_language_mangleSimpleProtocol(const char *module, const char *protocol);

#endif
