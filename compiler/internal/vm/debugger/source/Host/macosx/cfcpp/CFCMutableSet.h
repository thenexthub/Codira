//===-- CFCMutableSet.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCMUTABLESET_H
#define LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCMUTABLESET_H

#include "CFCReleaser.h"

class CFCMutableSet : public CFCReleaser<CFMutableSetRef> {
public:
  // Constructors and Destructors
  CFCMutableSet(CFMutableSetRef s = NULL);
  CFCMutableSet(const CFCMutableSet &rhs);
  ~CFCMutableSet() override;

  // Operators
  const CFCMutableSet &operator=(const CFCMutableSet &rhs);

  CFIndex GetCount() const;
  CFIndex GetCountOfValue(const void *value) const;
  const void *GetValue(const void *value) const;
  const void *AddValue(const void *value, bool can_create);
  void RemoveValue(const void *value);
  void RemoveAllValues();

protected:
  // Classes that inherit from CFCMutableSet can see and modify these

private:
  // For CFCMutableSet only
};

#endif // LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCMUTABLESET_H
