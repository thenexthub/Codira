/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef LIBABCKIT_SRC_ADAPTER_DYNAMIC_HELPERS_DYNAMIC_H
#define LIBABCKIT_SRC_ADAPTER_DYNAMIC_HELPERS_DYNAMIC_H

#include <string>

#include "libabckit/c/abckit.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "assembler/assembly-emitter.h"
#include "assembler/assembly-literals.h"

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#define STD_FILESYSTEM_EXPERIMENTAL
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

namespace libabckit {

struct ModuleUpdateData {
    bool isRegularImportsChange = false;
    bool isLocalExportsChange = false;
    std::vector<panda::pandasm::LiteralArray::Literal> newLiterals {};
    std::unordered_map<uint32_t, uint32_t> regularImportsIdxMap {};
    std::unordered_map<uint32_t, uint32_t> localExportsIdxMap {};
};

struct ModuleIterateData {
    AbckitCoreModule *m = nullptr;
    AbckitModulePayloadDyn *payload = nullptr;
    panda::pandasm::LiteralArray *moduleLitArr = nullptr;
    std::pair<bool, bool> isRegularImport {};  // {isImport, isRegular}
    AbckitDynamicExportKind kind = ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT;
    libabckit::ModuleUpdateData updateData {};
};

const panda::panda_file::File *EmitDynamicProgram(AbckitFile *file, panda::pandasm::Program *program,
                                                  panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps *mapsp,
                                                  bool getFile, std::string_view path = {});
bool IterateModuleSections(
    ModuleIterateData &data, const std::function<size_t(ModuleIterateData *)> &requestIdxSectionModifier,
    const std::function<std::pair<size_t, size_t>(ModuleIterateData *, size_t, size_t)> &sectionModifier,
    const std::function<bool(ModuleIterateData *, std::pair<size_t, size_t>)> &postAction);
void DumpModuleArray(const panda::pandasm::LiteralArray *moduleLitArr, std::string_view name);

fs::path Relative(const fs::path &src, const fs::path &base);

bool IsServiceRecord(const std::string &name);
bool IsModuleDescriptorRecord(const panda::pandasm::Record &rec);
bool IsAnnotationInterfaceRecord(const panda::pandasm::Record &rec);
bool IsExternalRecord(const panda::pandasm::Record &rec);

bool IsMain(const std::string &funcName);
bool IsFunction(const std::string &funcName);
bool IsCtor(const std::string &funcName);
bool IsStatic(const std::string &funcName);
bool IsAnonymousName(const std::string &funcName);

panda::pandasm::Function *GetDynFunction(AbckitCoreFunction *function);
panda::pandasm::Function *GetDynFunction(AbckitCoreClass *klass);
AbckitModulePayloadDyn *GetDynModulePayload(AbckitCoreModule *mod);
AbckitDynamicImportDescriptorPayload *GetDynImportDescriptorPayload(AbckitCoreImportDescriptor *id);
AbckitDynamicExportDescriptorPayload *GetDynExportDescriptorPayload(AbckitCoreExportDescriptor *ed);

std::string GetClassNameFromCtor(const std::string &ctorName, const AbckitLiteralArray *scopeNames);

AbckitLiteral *FindOrCreateLiteralBoolDynamicImpl(AbckitFile *file, bool value);
AbckitLiteral *FindOrCreateLiteralU8DynamicImpl(AbckitFile *file, uint8_t value);
AbckitLiteral *FindOrCreateLiteralU16DynamicImpl(AbckitFile *file, uint16_t value);
AbckitLiteral *FindOrCreateLiteralMethodAffiliateDynamicImpl(AbckitFile *file, uint16_t value);
AbckitLiteral *FindOrCreateLiteralU32DynamicImpl(AbckitFile *file, uint32_t value);
AbckitLiteral *FindOrCreateLiteralU64DynamicImpl(AbckitFile *file, uint64_t value);
AbckitLiteral *FindOrCreateLiteralFloatDynamicImpl(AbckitFile *file, float value);
AbckitLiteral *FindOrCreateLiteralDoubleDynamicImpl(AbckitFile *file, double value);
AbckitLiteral *FindOrCreateLiteralLiteralArrayDynamicImpl(AbckitFile *file, const std::string &value);
AbckitLiteral *FindOrCreateLiteralStringDynamicImpl(AbckitFile *file, const std::string &value);
AbckitLiteral *FindOrCreateLiteralMethodDynamicImpl(AbckitFile *file, const std::string &value);
AbckitLiteral *FindOrCreateLiteralNullValueDynamicImpl(AbckitFile *file);
AbckitLiteral *FindOrCreateLiteralDynamic(AbckitFile *file, const panda::pandasm::LiteralArray::Literal &value);

AbckitValue *FindOrCreateValueU1DynamicImpl(AbckitFile *file, bool value);
AbckitValue *FindOrCreateValueIntDynamicImpl(AbckitFile *file, int value);
AbckitValue *FindOrCreateValueDoubleDynamicImpl(AbckitFile *file, double value);
AbckitValue *FindOrCreateValueStringDynamicImpl(AbckitFile *file, const std::string &value);
AbckitValue *FindOrCreateLiteralArrayValueDynamicImpl(AbckitFile *file, const std::string &value);
AbckitValue *FindOrCreateValueDynamic(AbckitFile *file, const panda::pandasm::Value &value);

}  // namespace libabckit

#endif
