//===-- NativeProcessELF.h ------------------------------------ -*- C++ -*-===//
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

#ifndef liblldb_NativeProcessELF_H_
#define liblldb_NativeProcessELF_H_

#include "Plugins/Process/Utility/AuxVector.h"
#include "lldb/Host/common/NativeProcessProtocol.h"
#include "llvm/BinaryFormat/ELF.h"
#include <optional>

namespace lldb_private {

/// \class NativeProcessELF
/// Abstract class that extends \a NativeProcessProtocol with ELF specific
/// logic. Meant to be subclassed by ELF based NativeProcess* implementations.
class NativeProcessELF : public NativeProcessProtocol {
  using NativeProcessProtocol::NativeProcessProtocol;

public:
  std::optional<uint64_t> GetAuxValue(enum AuxVector::EntryType type);

protected:
  template <typename T> struct ELFLinkMap {
    T l_addr;
    T l_name;
    T l_ld;
    T l_next;
    T l_prev;
  };

  lldb::addr_t GetSharedLibraryInfoAddress() override;

  template <typename ELF_EHDR, typename ELF_PHDR, typename ELF_DYN>
  lldb::addr_t GetELFImageInfoAddress();

  llvm::Expected<std::vector<SVR4LibraryInfo>>
  GetLoadedSVR4Libraries() override;

  template <typename T>
  llvm::Expected<SVR4LibraryInfo>
  ReadSVR4LibraryInfo(lldb::addr_t link_map_addr);

  void NotifyDidExec() override;

  std::unique_ptr<AuxVector> m_aux_vector;
  std::optional<lldb::addr_t> m_shared_library_info_addr;
};

// Explicitly declare the two 32/64 bit templates that NativeProcessELF.cpp will
// define. This allows us to keep the template definition here and usable
// elsewhere.
extern template lldb::addr_t NativeProcessELF::GetELFImageInfoAddress<
    llvm::ELF::Elf32_Ehdr, llvm::ELF::Elf32_Phdr, llvm::ELF::Elf32_Dyn>();
extern template lldb::addr_t NativeProcessELF::GetELFImageInfoAddress<
    llvm::ELF::Elf64_Ehdr, llvm::ELF::Elf64_Phdr, llvm::ELF::Elf64_Dyn>();

} // namespace lldb_private

#endif
