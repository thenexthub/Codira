//===-- ObjectFileJSON.h -------------------------------------- -*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_OBJECTFILE_JSON_OBJECTFILEJSON_H
#define LLDB_SOURCE_PLUGINS_OBJECTFILE_JSON_OBJECTFILEJSON_H

#include "lldb/Symbol/ObjectFile.h"
#include "lldb/Utility/ArchSpec.h"
#include "llvm/Support/JSON.h"

namespace lldb_private {

class ObjectFileJSON : public ObjectFile {
public:
  static void Initialize();
  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() { return "JSON"; }

  static const char *GetPluginDescriptionStatic() {
    return "JSON object file reader.";
  }

  static ObjectFile *CreateInstance(const lldb::ModuleSP &module_sp,
                                    lldb::DataExtractorSP extractor_sp,
                                    lldb::offset_t data_offset,
                                    const FileSpec *file,
                                    lldb::offset_t file_offset,
                                    lldb::offset_t length);

  static ObjectFile *CreateMemoryInstance(const lldb::ModuleSP &module_sp,
                                          lldb::WritableDataBufferSP data_sp,
                                          const lldb::ProcessSP &process_sp,
                                          lldb::addr_t header_addr);

  static size_t GetModuleSpecifications(const FileSpec &file,
                                        lldb::DataBufferSP &data_sp,
                                        lldb::offset_t data_offset,
                                        lldb::offset_t file_offset,
                                        lldb::offset_t length,
                                        ModuleSpecList &specs);

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  // LLVM RTTI support
  static char ID;
  bool isA(const void *ClassID) const override {
    return ClassID == &ID || ObjectFile::isA(ClassID);
  }
  static bool classof(const ObjectFile *obj) { return obj->isA(&ID); }

  bool ParseHeader() override;

  lldb::ByteOrder GetByteOrder() const override {
    return m_arch.GetByteOrder();
  }

  bool IsExecutable() const override { return false; }

  uint32_t GetAddressByteSize() const override {
    return m_arch.GetAddressByteSize();
  }

  AddressClass GetAddressClass(lldb::addr_t file_addr) override {
    return AddressClass::eInvalid;
  }

  void ParseSymtab(lldb_private::Symtab &symtab) override;

  bool IsStripped() override { return false; }

  void CreateSections(SectionList &unified_section_list) override;

  void Dump(Stream *s) override {}

  ArchSpec GetArchitecture() override { return m_arch; }

  UUID GetUUID() override { return m_uuid; }

  uint32_t GetDependentModules(FileSpecList &files) override { return 0; }

  Type CalculateType() override { return m_type; }

  Strata CalculateStrata() override { return eStrataUser; }

  bool SetLoadAddress(Target &target, lldb::addr_t value,
                      bool value_is_offset) override;

  static bool MagicBytesMatch(lldb::DataBufferSP data_sp, lldb::addr_t offset,
                              lldb::addr_t length);

  struct Header {
    std::string triple;
    std::string uuid;
    std::optional<ObjectFile::Type> type;
  };

  struct Body {
    std::vector<JSONSection> sections;
    std::vector<JSONSymbol> symbols;
  };

private:
  ArchSpec m_arch;
  UUID m_uuid;
  ObjectFile::Type m_type;
  std::optional<uint64_t> m_size;
  std::vector<JSONSymbol> m_symbols;
  std::vector<JSONSection> m_sections;

  ObjectFileJSON(const lldb::ModuleSP &module_sp,
                 lldb::DataExtractorSP extractor_sp, lldb::offset_t data_offset,
                 const FileSpec *file, lldb::offset_t offset,
                 lldb::offset_t length, ArchSpec arch, UUID uuid, Type type,
                 std::vector<JSONSymbol> symbols,
                 std::vector<JSONSection> sections);
};

bool fromJSON(const llvm::json::Value &value, ObjectFileJSON::Header &header,
              llvm::json::Path path);

bool fromJSON(const llvm::json::Value &value, ObjectFileJSON::Body &body,
              llvm::json::Path path);

} // namespace lldb_private
#endif // LLDB_SOURCE_PLUGINS_OBJECTFILE_JSON_OBJECTFILEJSON_H
