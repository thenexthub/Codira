//===-- RegisterTypeBuilderClang.h ------------------------------*- C++ -*-===//
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

#ifndef LLDB_PLUGINS_REGISTERTYPEBUILDER_REGISTERTYPEBUILDERCLANG_H
#define LLDB_PLUGINS_REGISTERTYPEBUILDER_REGISTERTYPEBUILDERCLANG_H

#include "lldb/Target/RegisterTypeBuilder.h"
#include "lldb/Target/Target.h"

namespace lldb_private {
class RegisterTypeBuilderClang : public RegisterTypeBuilder {
public:
  RegisterTypeBuilderClang(Target &target);

  static void Initialize();
  static void Terminate();
  static llvm::StringRef GetPluginNameStatic() {
    return "register-types-clang";
  }
  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }
  static llvm::StringRef GetPluginDescriptionStatic() {
    return "Create register types using TypeSystemClang";
  }
  static lldb::RegisterTypeBuilderSP CreateInstance(Target &target);

  CompilerType GetRegisterType(const std::string &name,
                               const lldb_private::RegisterFlags &flags,
                               uint32_t byte_size) override;

private:
  Target &m_target;
};
} // namespace lldb_private

#endif // LLDB_PLUGINS_REGISTERTYPEBUILDER_REGISTERTYPEBUILDERCLANG_H
