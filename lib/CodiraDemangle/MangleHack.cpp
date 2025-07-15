//===--- MangleHack.cpp - Codira Mangle Hack for various clients -----------===//
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
// Implementations of mangler hacks for Interface Builder
//
// We don't have the time to disentangle the real mangler from the compiler
// right now.
//
//===----------------------------------------------------------------------===//

#include "language/Basic/Assertions.h"
#include "language/CodiraDemangle/MangleHack.h"
#include "language/Runtime/Portability.h"
#include "language/Strings.h"
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

const char *
_language_mangleSimpleClass(const char *module, const char *class_) {
  size_t moduleLength = strlen(module);
  size_t classLength = strlen(class_);
  char *value = nullptr;
  if (language::STDLIB_NAME == toolchain::StringRef(module)) {
    int result = language_asprintf(&value, "_TtCs%zu%s", classLength, class_);
    assert(result > 0);
    (void)result;
  } else {
    int result = language_asprintf(&value, "_TtC%zu%s%zu%s", moduleLength, module,
                          classLength, class_);
    assert(result > 0);
    (void)result;
  }
  assert(value);
  return value;
}

const char *
_language_mangleSimpleProtocol(const char *module, const char *protocol) {
  size_t moduleLength = strlen(module);
  size_t protocolLength = strlen(protocol);
  char *value = nullptr;
  if (language::STDLIB_NAME == toolchain::StringRef(module)) {
    int result = language_asprintf(&value, "_TtPs%zu%s_", protocolLength, protocol);
    assert(result > 0);
    (void)result;
  } else {
    int result = language_asprintf(&value, "_TtP%zu%s%zu%s_", moduleLength, module,
                          protocolLength, protocol);
    assert(result > 0);
    (void)result;
  }
  assert(value);
  return value;
}
