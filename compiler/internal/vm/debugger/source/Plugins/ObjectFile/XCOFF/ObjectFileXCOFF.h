//===-- ObjectFileXCOFF.h --------------------------------------- -*- C++
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

#ifndef LLDB_SOURCE_PLUGINS_OBJECTFILE_XCOFF_OBJECTFILEXCOFF_H
#define LLDB_SOURCE_PLUGINS_OBJECTFILE_XCOFF_OBJECTFILEXCOFF_H

#include "lldb/Symbol/ObjectFile.h"
#include "lldb/Utility/ArchSpec.h"
#include "lldb/Utility/FileSpec.h"
#include "lldb/Utility/UUID.h"
#include "lldb/lldb-private.h"
#include "llvm/Object/XCOFFObjectFile.h"
#include <cstdint>
#include <vector>

/// \class ObjectFileXCOFF
/// Generic XCOFF object file reader.
///
/// This class provides a generic XCOFF (32/64 bit) reader plugin implementing
/// the ObjectFile protocol.
class ObjectFileXCOFF : public lldb_private::ObjectFile {
public:
  // Static Functions
  static void Initialize();

  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() { return "xcoff"; }

  static llvm::StringRef GetPluginDescriptionStatic() {
    return "XCOFF object file reader.";
  }

  static lldb_private::ObjectFile *
  CreateInstance(const lldb::ModuleSP &module_sp,
                 lldb::DataExtractorSP extractor_sp, lldb::offset_t data_offset,
                 const lldb_private::FileSpec *file, lldb::offset_t file_offset,
                 lldb::offset_t length);

  static lldb_private::ObjectFile *CreateMemoryInstance(
      const lldb::ModuleSP &module_sp, lldb::WritableDataBufferSP data_sp,
      const lldb::ProcessSP &process_sp, lldb::addr_t header_addr);

  static size_t GetModuleSpecifications(const lldb_private::FileSpec &file,
                                        lldb::DataBufferSP &data_sp,
                                        lldb::offset_t data_offset,
                                        lldb::offset_t file_offset,
                                        lldb::offset_t length,
                                        lldb_private::ModuleSpecList &specs);

  static bool MagicBytesMatch(lldb::DataBufferSP &data_sp, lldb::addr_t offset,
                              lldb::addr_t length);

  // PluginInterface protocol
  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  // ObjectFile Protocol.
  bool ParseHeader() override;

  lldb::ByteOrder GetByteOrder() const override;

  bool IsExecutable() const override;

  uint32_t GetAddressByteSize() const override;

  lldb_private::AddressClass GetAddressClass(lldb::addr_t file_addr) override;

  void ParseSymtab(lldb_private::Symtab &symtab) override;

  bool IsStripped() override;

  void CreateSections(lldb_private::SectionList &unified_section_list) override;

  void Dump(lldb_private::Stream *s) override;

  lldb_private::ArchSpec GetArchitecture() override;

  lldb_private::UUID GetUUID() override;

  uint32_t GetDependentModules(lldb_private::FileSpecList &files) override;

  ObjectFile::Type CalculateType() override;

  ObjectFile::Strata CalculateStrata() override;

  ObjectFileXCOFF(const lldb::ModuleSP &module_sp,
                  lldb::DataExtractorSP extractor_sp,
                  lldb::offset_t data_offset,
                  const lldb_private::FileSpec *file, lldb::offset_t offset,
                  lldb::offset_t length);

  ObjectFileXCOFF(const lldb::ModuleSP &module_sp,
                  lldb::DataBufferSP header_data_sp,
                  const lldb::ProcessSP &process_sp, lldb::addr_t header_addr);

protected:
  static lldb::WritableDataBufferSP
  MapFileDataWritable(const lldb_private::FileSpec &file, uint64_t Size,
                      uint64_t Offset);

private:
  bool CreateBinary();
  template <typename T>
  void
  CreateSectionsWithBitness(lldb_private::SectionList &unified_section_list);

  struct XCOFF32 {
    using SectionHeader = llvm::object::XCOFFSectionHeader32;
    static constexpr bool Is64Bit = false;
  };
  struct XCOFF64 {
    using SectionHeader = llvm::object::XCOFFSectionHeader64;
    static constexpr bool Is64Bit = true;
  };

  std::unique_ptr<llvm::object::XCOFFObjectFile> m_binary;
};

#endif // LLDB_SOURCE_PLUGINS_OBJECTFILE_XCOFF_OBJECTFILE_H
