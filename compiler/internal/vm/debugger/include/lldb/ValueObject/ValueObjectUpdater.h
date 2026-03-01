//===-- ValueObjectUpdater.h ------------------------------------*- C++ -*-===//
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

#ifndef LLDB_VALUEOBJECT_VALUEOBJECTUPDATER_H
#define LLDB_VALUEOBJECT_VALUEOBJECTUPDATER_H

#include "lldb/ValueObject/ValueObject.h"

namespace lldb_private {

/// A value object class that is seeded with the static variable value
/// and it vends the user facing value object. If the type is dynamic it can
/// vend the dynamic type. If this user type also has a synthetic type
/// associated with it, it will vend the synthetic type. The class watches the
/// process' stop ID and will update the user type when needed.
class ValueObjectUpdater {
  /// The root value object is the static typed variable object.
  lldb::ValueObjectSP m_root_valobj_sp;
  /// The user value object is the value object the user wants to see.
  lldb::ValueObjectSP m_user_valobj_sp;
  /// The stop ID that m_user_valobj_sp is valid for.
  uint32_t m_stop_id = UINT32_MAX;

public:
  ValueObjectUpdater(lldb::ValueObjectSP in_valobj_sp);

  /// Gets the correct value object from the root object for a given process
  /// stop ID. If dynamic values are enabled, or if synthetic children are
  /// enabled, the value object that the user wants to see might change while
  /// debugging.
  lldb::ValueObjectSP GetSP();

  lldb::ProcessSP GetProcessSP() const;
};

} // namespace lldb_private

#endif // LLDB_VALUEOBJECT_VALUEOBJECTUPDATER_H
