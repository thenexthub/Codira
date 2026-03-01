//===-- CFCMutableArray.cpp -----------------------------------------------===//
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

#include "CFCMutableArray.h"
#include "CFCString.h"

// CFCString constructor
CFCMutableArray::CFCMutableArray(CFMutableArrayRef s)
    : CFCReleaser<CFMutableArrayRef>(s) {}

// CFCMutableArray copy constructor
CFCMutableArray::CFCMutableArray(const CFCMutableArray &rhs) =
    default; // NOTE: this won't make a copy of the
             // array, just add a new reference to
             // it

// CFCMutableArray copy constructor
CFCMutableArray &CFCMutableArray::operator=(const CFCMutableArray &rhs) {
  if (this != &rhs)
    *this = rhs; // NOTE: this operator won't make a copy of the array, just add
                 // a new reference to it
  return *this;
}

// Destructor
CFCMutableArray::~CFCMutableArray() = default;

CFIndex CFCMutableArray::GetCount() const {
  CFMutableArrayRef array = get();
  if (array)
    return ::CFArrayGetCount(array);
  return 0;
}

CFIndex CFCMutableArray::GetCountOfValue(CFRange range,
                                         const void *value) const {
  CFMutableArrayRef array = get();
  if (array)
    return ::CFArrayGetCountOfValue(array, range, value);
  return 0;
}

CFIndex CFCMutableArray::GetCountOfValue(const void *value) const {
  CFMutableArrayRef array = get();
  if (array)
    return ::CFArrayGetCountOfValue(array, CFRangeMake(0, GetCount()), value);
  return 0;
}

const void *CFCMutableArray::GetValueAtIndex(CFIndex idx) const {
  CFMutableArrayRef array = get();
  if (array) {
    const CFIndex num_array_items = ::CFArrayGetCount(array);
    if (0 <= idx && idx < num_array_items) {
      return ::CFArrayGetValueAtIndex(array, idx);
    }
  }
  return NULL;
}

bool CFCMutableArray::SetValueAtIndex(CFIndex idx, const void *value) {
  CFMutableArrayRef array = get();
  if (array != NULL) {
    const CFIndex num_array_items = ::CFArrayGetCount(array);
    if (0 <= idx && idx < num_array_items) {
      ::CFArraySetValueAtIndex(array, idx, value);
      return true;
    }
  }
  return false;
}

bool CFCMutableArray::AppendValue(const void *value, bool can_create) {
  CFMutableArrayRef array = get();
  if (array == NULL) {
    if (!can_create)
      return false;
    array =
        ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    reset(array);
  }
  if (array != NULL) {
    ::CFArrayAppendValue(array, value);
    return true;
  }
  return false;
}

bool CFCMutableArray::AppendCStringAsCFString(const char *s,
                                              CFStringEncoding encoding,
                                              bool can_create) {
  CFMutableArrayRef array = get();
  if (array == NULL) {
    if (!can_create)
      return false;
    array =
        ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    reset(array);
  }
  if (array != NULL) {
    CFCString cf_str(s, encoding);
    ::CFArrayAppendValue(array, cf_str.get());
    return true;
  }
  return false;
}

bool CFCMutableArray::AppendFileSystemRepresentationAsCFString(
    const char *s, bool can_create) {
  CFMutableArrayRef array = get();
  if (array == NULL) {
    if (!can_create)
      return false;
    array =
        ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    reset(array);
  }
  if (array != NULL) {
    CFCString cf_path;
    cf_path.SetFileSystemRepresentation(s);
    ::CFArrayAppendValue(array, cf_path.get());
    return true;
  }
  return false;
}
