//===-- CFCBundle.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCBUNDLE_H
#define LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCBUNDLE_H

#include "CFCReleaser.h"

class CFCBundle : public CFCReleaser<CFBundleRef> {
public:
  // Constructors and Destructors
  CFCBundle(const char *path = NULL);
  CFCBundle(CFURLRef url);

  ~CFCBundle() override;

  CFURLRef CopyExecutableURL() const;

  CFStringRef GetIdentifier() const;

  CFTypeRef GetValueForInfoDictionaryKey(CFStringRef key) const;

  bool GetPath(char *dst, size_t dst_len);

  bool SetPath(const char *path);

private:
  // Disallow copy and assignment constructors
  CFCBundle(const CFCBundle &) = delete;

  const CFCBundle &operator=(const CFCBundle &) = delete;
};

#endif // LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCBUNDLE_H
