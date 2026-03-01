//===----------------------------------------------------------------------===//
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

#ifndef LLDB_INTERPRETER_INTERFACES_SCRIPTEDFRAMEPROVIDERINTERFACE_H
#define LLDB_INTERPRETER_INTERFACES_SCRIPTEDFRAMEPROVIDERINTERFACE_H

#include "lldb/lldb-private.h"

#include "ScriptedInterface.h"

namespace lldb_private {
class ScriptedFrameProviderInterface : public ScriptedInterface {
public:
  virtual bool AppliesToThread(llvm::StringRef class_name,
                               lldb::ThreadSP thread_sp) {
    return true;
  }

  virtual llvm::Expected<StructuredData::GenericSP>
  CreatePluginObject(llvm::StringRef class_name,
                     lldb::StackFrameListSP input_frames,
                     StructuredData::DictionarySP args_sp) = 0;

  /// Get a description string for the frame provider.
  ///
  /// This is called by the descriptor to fetch a description from the
  /// scripted implementation. Implementations should call a static method
  /// on the scripting class to retrieve the description.
  ///
  /// \param class_name The name of the scripting class implementing the
  /// provider.
  ///
  /// \return A string describing what this frame provider does, or an
  ///         empty string if no description is available.
  virtual std::string GetDescription(llvm::StringRef class_name) { return {}; }

  /// Get the priority of this frame provider.
  ///
  /// This is called by the descriptor to fetch the priority from the
  /// scripted implementation. Implementations should call a static method
  /// on the scripting class to retrieve the priority. Lower numbers indicate
  /// higher priority (like Unix nice values).
  ///
  /// \param class_name The name of the scripting class implementing the
  /// provider.
  ///
  /// \return Priority value where 0 is highest priority, or std::nullopt for
  ///         default priority (UINT32_MAX - lowest priority).
  virtual std::optional<uint32_t> GetPriority(llvm::StringRef class_name) {
    return std::nullopt;
  }

  virtual StructuredData::ObjectSP GetFrameAtIndex(uint32_t index) {
    return {};
  }
};
} // namespace lldb_private

#endif // LLDB_INTERPRETER_INTERFACES_SCRIPTEDFRAMEPROVIDERINTERFACE_H
