//===- ObjCRuntime.cpp - Objective-C Runtime Handling ---------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// This file implements the ObjCRuntime class, which represents the
// target Objective-C runtime.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/ObjCRuntime.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/VersionTuple.h"
#include "toolchain/Support/raw_ostream.h"
#include <cstddef>
#include <string>

using namespace language::Core;

std::string ObjCRuntime::getAsString() const {
  std::string Result;
  {
    toolchain::raw_string_ostream Out(Result);
    Out << *this;
  }
  return Result;
}

raw_ostream &language::Core::operator<<(raw_ostream &out, const ObjCRuntime &value) {
  switch (value.getKind()) {
  case ObjCRuntime::MacOSX: out << "macosx"; break;
  case ObjCRuntime::FragileMacOSX: out << "macosx-fragile"; break;
  case ObjCRuntime::iOS: out << "ios"; break;
  case ObjCRuntime::WatchOS: out << "watchos"; break;
  case ObjCRuntime::GNUstep: out << "gnustep"; break;
  case ObjCRuntime::GCC: out << "gcc"; break;
  case ObjCRuntime::ObjFW: out << "objfw"; break;
  }
  if (value.getVersion() > VersionTuple(0)) {
    out << '-' << value.getVersion();
  }
  return out;
}

bool ObjCRuntime::tryParse(StringRef input) {
  // Look for the last dash.
  std::size_t dash = input.rfind('-');

  // We permit dashes in the runtime name, and we also permit the
  // version to be omitted, so if we see a dash not followed by a
  // digit then we need to ignore it.
  if (dash != StringRef::npos && dash + 1 != input.size() &&
      (input[dash+1] < '0' || input[dash+1] > '9')) {
    dash = StringRef::npos;
  }

  // Everything prior to that must be a valid string name.
  Kind kind;
  StringRef runtimeName = input.substr(0, dash);
  Version = VersionTuple(0);
  if (runtimeName == "macosx") {
    kind = ObjCRuntime::MacOSX;
  } else if (runtimeName == "macosx-fragile") {
    kind = ObjCRuntime::FragileMacOSX;
  } else if (runtimeName == "ios") {
    kind = ObjCRuntime::iOS;
  } else if (runtimeName == "watchos") {
    kind = ObjCRuntime::WatchOS;
  } else if (runtimeName == "gnustep") {
    // If no version is specified then default to the most recent one that we
    // know about.
    Version = VersionTuple(1, 6);
    kind = ObjCRuntime::GNUstep;
  } else if (runtimeName == "gcc") {
    kind = ObjCRuntime::GCC;
  } else if (runtimeName == "objfw") {
    kind = ObjCRuntime::ObjFW;
    Version = VersionTuple(0, 8);
  } else {
    return true;
  }
  TheKind = kind;

  if (dash != StringRef::npos) {
    StringRef verString = input.substr(dash + 1);
    if (Version.tryParse(verString))
      return true;
  }

  if (kind == ObjCRuntime::ObjFW && Version > VersionTuple(0, 8))
    Version = VersionTuple(0, 8);

  return false;
}
