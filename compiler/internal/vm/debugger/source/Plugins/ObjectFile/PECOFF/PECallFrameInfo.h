//===-- PECallFrameInfo.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_OBJECTFILE_PECOFF_PECALLFRAMEINFO_H
#define LLDB_SOURCE_PLUGINS_OBJECTFILE_PECOFF_PECALLFRAMEINFO_H

#include "lldb/Core/AddressRange.h"
#include "lldb/Symbol/CallFrameInfo.h"
#include "lldb/Symbol/UnwindPlan.h"
#include "lldb/Utility/DataExtractor.h"

class ObjectFilePECOFF;

namespace llvm {
namespace Win64EH {

struct RuntimeFunction;

}
} // namespace llvm

class PECallFrameInfo : public virtual lldb_private::CallFrameInfo {
public:
  explicit PECallFrameInfo(ObjectFilePECOFF &object_file,
                           uint32_t exception_dir_rva,
                           uint32_t exception_dir_size);

  bool GetAddressRange(lldb_private::Address addr,
                       lldb_private::AddressRange &range) override;

  std::unique_ptr<lldb_private::UnwindPlan>
  GetUnwindPlan(const lldb_private::Address &addr) override {
    return GetUnwindPlan({lldb_private::AddressRange(addr, 1)}, addr);
  }

  std::unique_ptr<lldb_private::UnwindPlan>
  GetUnwindPlan(llvm::ArrayRef<lldb_private::AddressRange> ranges,
                const lldb_private::Address &addr) override;

private:
  const llvm::Win64EH::RuntimeFunction *FindRuntimeFunctionIntersectsWithRange(
      const lldb_private::AddressRange &range) const;

  ObjectFilePECOFF &m_object_file;
  lldb_private::DataExtractor m_exception_dir;
};

#endif // LLDB_SOURCE_PLUGINS_OBJECTFILE_PECOFF_PECALLFRAMEINFO_H
