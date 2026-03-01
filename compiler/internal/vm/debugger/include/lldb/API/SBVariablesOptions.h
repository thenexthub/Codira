//===-- SBVariablesOptions.h ------------------------------------------*- C++
//-*-===//
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

#ifndef LLDB_API_SBVARIABLESOPTIONS_H
#define LLDB_API_SBVARIABLESOPTIONS_H

#include "lldb/API/SBDefines.h"

class VariablesOptionsImpl;

namespace lldb {

class LLDB_API SBVariablesOptions {
public:
  SBVariablesOptions();

  SBVariablesOptions(const SBVariablesOptions &options);

  SBVariablesOptions &operator=(const SBVariablesOptions &options);

  ~SBVariablesOptions();

  explicit operator bool() const;

  bool IsValid() const;

  bool GetIncludeArguments() const;

  void SetIncludeArguments(bool);

  bool GetIncludeRecognizedArguments(const lldb::SBTarget &) const;

  void SetIncludeRecognizedArguments(bool);

  bool GetIncludeLocals() const;

  void SetIncludeLocals(bool);

  bool GetIncludeStatics() const;

  void SetIncludeStatics(bool);

  bool GetInScopeOnly() const;

  void SetInScopeOnly(bool);

  bool GetIncludeRuntimeSupportValues() const;

  void SetIncludeRuntimeSupportValues(bool);

  lldb::DynamicValueType GetUseDynamic() const;

  void SetUseDynamic(lldb::DynamicValueType);

protected:
  VariablesOptionsImpl *operator->();

  const VariablesOptionsImpl *operator->() const;

  VariablesOptionsImpl *get();

  VariablesOptionsImpl &ref();

  const VariablesOptionsImpl &ref() const;

  SBVariablesOptions(VariablesOptionsImpl *lldb_object_ptr);

  void SetOptions(VariablesOptionsImpl *lldb_object_ptr);

private:
  std::unique_ptr<VariablesOptionsImpl> m_opaque_up;
};

} // namespace lldb

#endif // LLDB_API_SBVARIABLESOPTIONS_H
