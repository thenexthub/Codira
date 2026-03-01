//===---- NativeRegisterContextAIX.h ----------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_AIX_NATIVEREGISTERCONTEXTAIX_H
#define LLDB_SOURCE_PLUGINS_PROCESS_AIX_NATIVEREGISTERCONTEXTAIX_H

#include "Plugins/Process/Utility/NativeRegisterContextRegisterInfo.h"

namespace lldb_private::process_aix {

class NativeRegisterContextAIX
    : public virtual NativeRegisterContextRegisterInfo {
protected:
  NativeRegisterContextAIX(NativeThreadProtocol &thread)
      : NativeRegisterContextRegisterInfo(thread, nullptr) {}

  lldb::ByteOrder GetByteOrder() const;

  virtual Status ReadRegisterRaw(uint32_t reg_index, RegisterValue &reg_value);

  virtual Status WriteRegisterRaw(uint32_t reg_index,
                                  const RegisterValue &reg_value);

  virtual Status ReadRegisterSet(void *buf, size_t buf_size,
                                 unsigned int regset);

  virtual Status WriteRegisterSet(void *buf, size_t buf_size,
                                  unsigned int regset);

  virtual Status ReadGPR();

  virtual Status WriteGPR();

  virtual Status ReadFPR();

  virtual Status WriteFPR();

  virtual Status ReadVMX();

  virtual Status WriteVMX();

  virtual Status ReadVSX();

  virtual Status WriteVSX();

  virtual void *GetGPRBuffer() = 0;

  virtual size_t GetGPRSize() = 0;

  virtual void *GetFPRBuffer() = 0;

  virtual size_t GetFPRSize() = 0;
};

} // namespace lldb_private::process_aix

#endif // #ifndef LLDB_SOURCE_PLUGINS_PROCESS_AIX_NATIVEREGISTERCONTEXTAIX_H
