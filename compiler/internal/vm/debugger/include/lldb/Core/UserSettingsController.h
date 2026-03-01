//====-- UserSettingsController.h --------------------------------*- C++-*-===//
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

#ifndef LLDB_CORE_USERSETTINGSCONTROLLER_H
#define LLDB_CORE_USERSETTINGSCONTROLLER_H

#include "lldb/Interpreter/OptionValueProperties.h"
#include "lldb/Utility/Status.h"
#include "lldb/lldb-forward.h"
#include "lldb/lldb-private-enumerations.h"

#include "llvm/ADT/StringRef.h"

#include <vector>

#include <cstddef>
#include <cstdint>

namespace lldb_private {
class CommandInterpreter;
class ExecutionContext;
class Property;
class Stream;
}

namespace lldb_private {

class Properties {
public:
  Properties();

  Properties(const lldb::OptionValuePropertiesSP &collection_sp);

  virtual ~Properties();

  lldb::OptionValuePropertiesSP GetValueProperties() const {
    return m_collection_sp;
  }

  virtual lldb::OptionValueSP GetPropertyValue(const ExecutionContext *exe_ctx,
                                               llvm::StringRef property_path,
                                               Status &error) const;

  virtual Status SetPropertyValue(const ExecutionContext *exe_ctx,
                                  VarSetOperationType op,
                                  llvm::StringRef property_path,
                                  llvm::StringRef value);

  virtual Status DumpPropertyValue(const ExecutionContext *exe_ctx,
                                   Stream &strm, llvm::StringRef property_path,
                                   uint32_t dump_mask, bool is_json = false);

  virtual void DumpAllPropertyValues(const ExecutionContext *exe_ctx,
                                     Stream &strm, uint32_t dump_mask,
                                     bool is_json = false);

  virtual void DumpAllDescriptions(CommandInterpreter &interpreter,
                                   Stream &strm) const;

  size_t Apropos(llvm::StringRef keyword,
                 std::vector<const Property *> &matching_properties) const;

  // We sometimes need to introduce a setting to enable experimental features,
  // but then we don't want the setting for these to cause errors when the
  // setting goes away.  Add a sub-topic of the settings using this
  // experimental name, and two things will happen.  One is that settings that
  // don't find the name will not be treated as errors.  Also, if you decide to
  // keep the settings just move them into the containing properties, and we
  // will auto-forward the experimental settings to the real one.
  static llvm::StringRef GetExperimentalSettingsName();

  static bool IsSettingExperimental(llvm::StringRef setting);

  template <typename T>
  T GetPropertyAtIndexAs(uint32_t idx, T default_value,
                         const ExecutionContext *exe_ctx = nullptr) const {
    return m_collection_sp->GetPropertyAtIndexAs<T>(idx, exe_ctx)
        .value_or(default_value);
  }

  template <typename T, typename U = typename std::remove_pointer<T>::type,
            std::enable_if_t<std::is_pointer_v<T>, bool> = true>
  const U *
  GetPropertyAtIndexAs(uint32_t idx,
                       const ExecutionContext *exe_ctx = nullptr) const {
    return m_collection_sp->GetPropertyAtIndexAs<T>(idx, exe_ctx);
  }

  template <typename T>
  bool SetPropertyAtIndex(uint32_t idx, T t,
                          const ExecutionContext *exe_ctx = nullptr) const {
    return m_collection_sp->SetPropertyAtIndex<T>(idx, t, exe_ctx);
  }

protected:
  lldb::OptionValuePropertiesSP m_collection_sp;
};

} // namespace lldb_private

#endif // LLDB_CORE_USERSETTINGSCONTROLLER_H
