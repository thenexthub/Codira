//===-- CFCMutableArray.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCMUTABLEARRAY_H
#define LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCMUTABLEARRAY_H

#include "CFCReleaser.h"

class CFCMutableArray : public CFCReleaser<CFMutableArrayRef> {
public:
  // Constructors and Destructors
  CFCMutableArray(CFMutableArrayRef array = NULL);
  CFCMutableArray(const CFCMutableArray &rhs); // This will copy the array
                                               // contents into a new array
  CFCMutableArray &operator=(const CFCMutableArray &rhs); // This will re-use
                                                          // the same array and
                                                          // just bump the ref
                                                          // count
  ~CFCMutableArray() override;

  CFIndex GetCount() const;
  CFIndex GetCountOfValue(const void *value) const;
  CFIndex GetCountOfValue(CFRange range, const void *value) const;
  const void *GetValueAtIndex(CFIndex idx) const;
  bool SetValueAtIndex(CFIndex idx, const void *value);
  bool AppendValue(const void *value,
                   bool can_create = true); // Appends value and optionally
                                            // creates a CFCMutableArray if this
                                            // class doesn't contain one
  bool
  AppendCStringAsCFString(const char *cstr,
                          CFStringEncoding encoding = kCFStringEncodingUTF8,
                          bool can_create = true);
  bool AppendFileSystemRepresentationAsCFString(const char *s,
                                                bool can_create = true);
};

#endif // LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCMUTABLEARRAY_H
