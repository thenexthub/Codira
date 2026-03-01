//===-- CFCMutableSet.cpp -------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "CFCMutableSet.h"


// CFCString constructor
CFCMutableSet::CFCMutableSet(CFMutableSetRef s)
    : CFCReleaser<CFMutableSetRef>(s) {}

// CFCMutableSet copy constructor
CFCMutableSet::CFCMutableSet(const CFCMutableSet &rhs) = default;

// CFCMutableSet copy constructor
const CFCMutableSet &CFCMutableSet::operator=(const CFCMutableSet &rhs) {
  if (this != &rhs)
    *this = rhs;
  return *this;
}

// Destructor
CFCMutableSet::~CFCMutableSet() = default;

CFIndex CFCMutableSet::GetCount() const {
  CFMutableSetRef set = get();
  if (set)
    return ::CFSetGetCount(set);
  return 0;
}

CFIndex CFCMutableSet::GetCountOfValue(const void *value) const {
  CFMutableSetRef set = get();
  if (set)
    return ::CFSetGetCountOfValue(set, value);
  return 0;
}

const void *CFCMutableSet::GetValue(const void *value) const {
  CFMutableSetRef set = get();
  if (set)
    return ::CFSetGetValue(set, value);
  return NULL;
}

const void *CFCMutableSet::AddValue(const void *value, bool can_create) {
  CFMutableSetRef set = get();
  if (set == NULL) {
    if (!can_create)
      return NULL;
    set = ::CFSetCreateMutable(kCFAllocatorDefault, 0, &kCFTypeSetCallBacks);
    reset(set);
  }
  if (set != NULL) {
    ::CFSetAddValue(set, value);
    return value;
  }
  return NULL;
}

void CFCMutableSet::RemoveValue(const void *value) {
  CFMutableSetRef set = get();
  if (set)
    ::CFSetRemoveValue(set, value);
}

void CFCMutableSet::RemoveAllValues() {
  CFMutableSetRef set = get();
  if (set)
    ::CFSetRemoveAllValues(set);
}
