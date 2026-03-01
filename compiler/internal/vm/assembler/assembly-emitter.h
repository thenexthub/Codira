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

#ifndef ASSEMBLER_ASSEMBLY_EMITTER_H
#define ASSEMBLER_ASSEMBLY_EMITTER_H

#include <memory>
#include <type_traits>
#include <vector>
#include <unordered_map>

#include "assembly-ins.h"
#include "assembly-literals.h"
#include "assembly-program.h"
#include "assembly-type.h"
#include "assembly-function.h"
#include "bytecode_emitter.h"
#include "file_item_container.h"
#include "pgo.h"

namespace panda::pandasm {

constexpr size_t DEFAULT_ITEM_THREAD_COUNT = 8;;
struct EmitterConfig {
    uint8_t api = 0;
    std::string subApi = panda_file::DEFAULT_SUB_API_VERSION;
    bool isDebug = false;
    size_t fileThreadCount = DEFAULT_ITEM_THREAD_COUNT;
};

class AsmEmitter {
public:
    struct PandaFileToPandaAsmMaps {
        std::unordered_map<uint32_t, std::string> methods;
        std::unordered_map<uint32_t, std::string> fields;
        std::unordered_map<uint32_t, std::string> classes;
        std::unordered_map<uint32_t, std::string> strings;
        std::unordered_map<uint32_t, std::string> literalarrays;
    };

    struct AsmEntityCollections {
        std::unordered_map<std::string, panda_file::BaseMethodItem *> method_items;
        std::unordered_map<std::string, panda_file::BaseFieldItem *> field_items;
        std::unordered_map<std::string, panda_file::BaseClassItem *> class_items;
        std::unordered_map<std::string, panda_file::StringItem *> string_items;
        std::unordered_map<std::string, panda_file::LiteralArrayItem *> literalarray_items;
    };

    PANDA_PUBLIC_API static bool Emit(panda_file::ItemContainer *items, const Program &program,
                                      PandaFileToPandaAsmMaps *maps = nullptr, bool emit_debug_info = true,
                                      panda::panda_file::pgo::ProfileOptimizer *profile_opt = nullptr);

    PANDA_PUBLIC_API static bool Emit(panda_file::Writer *writer, const Program &program,
                                      std::map<std::string, size_t> *stat = nullptr,
                                      PandaFileToPandaAsmMaps *maps = nullptr, bool debug_info = true,
                                      panda::panda_file::pgo::ProfileOptimizer *profile_opt = nullptr, uint8_t api = 0,
                                      std::string subApi = panda_file::DEFAULT_SUB_API_VERSION);

    PANDA_PUBLIC_API static bool Emit(const std::string &filename, const Program &program,
                                      std::map<std::string, size_t> *stat = nullptr,
                                      PandaFileToPandaAsmMaps *maps = nullptr, bool debug_info = true,
                                      panda::panda_file::pgo::ProfileOptimizer *profile_opt = nullptr, uint8_t api = 0,
                                      std::string subApi = panda_file::DEFAULT_SUB_API_VERSION);

    // The function releases the data in progs in advance for the sake of the peak memory at compiler time.
    static bool EmitPrograms(const std::string &filename, const std::vector<Program *> &progs, bool emit_debug_info,
                             const EmitterConfig &emitterConfig = EmitterConfig {},
                             std::map<std::string, size_t> *stat = nullptr);

    PANDA_PUBLIC_API static std::unique_ptr<const panda_file::File> Emit(
        const Program &program, PandaFileToPandaAsmMaps *maps = nullptr, uint8_t api = 0,
        std::string subApi = panda_file::DEFAULT_SUB_API_VERSION);

    static std::string GetLastError()
    {
        return last_error;
    }

    static void SetLastError(const std::string &message)
    {
        last_error = message;
    }

private:
    static bool FillFields(panda_file::ItemContainer *items, const Program &program,
                           const panda::pandasm::Record &record, const AsmEmitter::AsmEntityCollections &entities);
    static bool CheckDuplicateField(panda_file::ValueItem &value_item, panda_file::FieldItem &field_item,
                                    std::string &field_name);
    static void MakeStringItems(panda_file::ItemContainer *items, const Program &program,
                                AsmEntityCollections &entities);
    static void MakeLiteralItems(panda_file::ItemContainer *items, const Program &program,
                                 AsmEmitter::AsmEntityCollections &entities);
    static void MakeArrayTypeItems(panda_file::ItemContainer *items, const Program &program,
                                   AsmEntityCollections &entities);
    static bool HandleRecordAsForeign(
        panda_file::ItemContainer *items, const Program &program, AsmEntityCollections &entities,
        const std::unordered_map<panda_file::Type::TypeId, panda_file::PrimitiveTypeItem *> &primitive_types,
        const std::string &name, const Record &rec);
    static bool HandleBaseRecord(panda_file::ItemContainer *items, const Program &program, const std::string &name,
                                 const Record &base_rec, panda_file::ClassItem *record);
    static bool HandleInterfaces(panda_file::ItemContainer *items, const Program &program, const std::string &name,
                                 const Record &rec, panda_file::ClassItem *record);
    static bool HandleFields(
        panda_file::ItemContainer *items, const Program &program, AsmEmitter::AsmEntityCollections &entities,
        const std::unordered_map<panda_file::Type::TypeId, panda_file::PrimitiveTypeItem *> &primitive_types,
        const std::string &name, const Record &rec, panda_file::ClassItem *record);
    static bool HandleRecord(
        panda_file::ItemContainer *items, const Program &program, AsmEntityCollections &entities,
        const std::unordered_map<panda_file::Type::TypeId, panda_file::PrimitiveTypeItem *> &primitive_types,
        const std::string &name, const Record &rec);
    static bool MakeRecordItems(
        panda_file::ItemContainer *items, const Program &program, AsmEntityCollections &entities,
        const std::unordered_map<panda_file::Type::TypeId, panda_file::PrimitiveTypeItem *> &primitive_types);
    static panda_file::StringItem *GetMethodName(panda_file::ItemContainer *items, const Function &func,
                                                 const std::string &name);
    static bool HandleAreaForInner(panda_file::ItemContainer *items, const Program &program,
                                   panda_file::ClassItem **area, panda_file::ForeignClassItem **foreign_area,
                                   const std::string &name, const std::string &record_owner_name);
    static bool HandleRecordOnwer(panda_file::ItemContainer *items, const Program &program,
                                  panda_file::ClassItem **area, panda_file::ForeignClassItem **foreign_area,
                                  const std::string &name, const std::string &record_owner_name);
    static bool HandleFunctionParams(
        panda_file::ItemContainer *items, const Program &program, size_t idx, const std::string &name,
        const Function &func,
        const std::unordered_map<panda_file::Type::TypeId, panda_file::PrimitiveTypeItem *> &primitive_types,
        std::vector<panda_file::MethodParamItem> &params);
    static bool HandleFunctionLocalVariables(panda_file::ItemContainer *items, const Function &func,
                                             const std::string &name);
    static bool CreateMethodItem(panda_file::ItemContainer *items, AsmEmitter::AsmEntityCollections &entities,
                                 const Function &func, panda_file::TypeItem *type_item, panda_file::ClassItem *area,
                                 panda_file::ForeignClassItem *foreign_area, uint32_t access_flags,
                                 panda_file::StringItem *method_name, const std::string &mangled_name,
                                 const std::string &name, std::vector<panda_file::MethodParamItem> &params);
    static bool MakeFunctionItems(
        panda_file::ItemContainer *items, const Program &program, AsmEntityCollections &entities,
        const std::unordered_map<panda_file::Type::TypeId, panda_file::PrimitiveTypeItem *> &primitive_types,
        bool emit_debug_info);
    static bool MakeRecordAnnotations(panda_file::ItemContainer *items, const Program &program,
                                      const AsmEntityCollections &entities);
    static void SetCodeAndDebugInfo(panda_file::ItemContainer *items, panda_file::MethodItem *method,
                                    const Function &func, bool emit_debug_info);
    static bool AddMethodAndParamsAnnotations(panda_file::ItemContainer *items, const Program &program,
                                              const AsmEmitter::AsmEntityCollections &entities,
                                              panda_file::MethodItem *method, const Function &func);
    static bool MakeFunctionDebugInfoAndAnnotations(panda_file::ItemContainer *items, const Program &program,
                                                    const AsmEntityCollections &entities, bool emit_debug_info);
    static void FillMap(PandaFileToPandaAsmMaps *maps, AsmEntityCollections &entities);
    static void EmitDebugInfo(panda_file::ItemContainer *items, const Program &program,
                              const std::vector<uint8_t> *bytes, const panda_file::MethodItem *method,
                              const Function &func, const std::string &name, bool emit_debug_info);
    static bool EmitFunctions(panda_file::ItemContainer *items, const Program &program,
                              const AsmEntityCollections &entities, bool emit_debug_info);

    static panda_file::TypeItem *GetTypeItem(
        panda_file::ItemContainer *items,
        const std::unordered_map<panda_file::Type::TypeId, panda_file::PrimitiveTypeItem *> &primitive_types,
        const Type &type, const Program &program);

    static bool MakeItemsForSingleProgram(panda_file::ItemContainer *items, const Program &program,
        bool emit_debug_info, AsmEntityCollections &entities,
        std::unordered_map<panda_file::Type::TypeId, panda_file::PrimitiveTypeItem *> primitive_types);

    static std::string GetMethodSignatureFromProgram(const std::string &name, const Program &program);

    static panda_file::LiteralItem *CreateLiteralItem(panda_file::ItemContainer *container, const Value *value,
                                                      std::vector<panda_file::LiteralItem> *out,
                                                      const AsmEmitter::AsmEntityCollections &entities);

    template <class PrimType>
    static panda_file::ScalarValueItem *CreateScalarPrimValueItem(panda_file::ItemContainer *container,
                                                                  const Value *value,
                                                                  std::vector<panda_file::ScalarValueItem> *out)
    {
        static_assert(std::is_arithmetic<PrimType>::value);
        auto v = value->GetAsScalar()->GetValue<PrimType>();
        if (out != nullptr) {
            out->emplace_back(v, container);
            return &out->back();
        }

        if constexpr (std::is_same<PrimType, uint32_t>::value) {
            return container->GetOrCreateIntegerValueItem(v);
        } else if constexpr (std::is_same<PrimType, uint64_t>::value) {
            return container->GetOrCreateLongValueItem(v);
        } else if constexpr (std::is_same<PrimType, float>::value) {
            return container->GetOrCreateFloatValueItem(v);
        } else if constexpr (std::is_same<PrimType, double>::value) {
            return container->GetOrCreateDoubleValueItem(v);
        } else {
            UNREACHABLE();
            return nullptr;
        }
    }

    static panda_file::ScalarValueItem *CreateScalarStringValueItem(panda_file::ItemContainer *container,
                                                                    const Value *value,
                                                                    std::vector<panda_file::ScalarValueItem> *out);
    static panda_file::ScalarValueItem *CreateScalarRecordValueItem(
        panda_file::ItemContainer *container, const Value *value, std::vector<panda_file::ScalarValueItem> *out,
        const std::unordered_map<std::string, panda_file::BaseClassItem *> &classes);
    static panda_file::ScalarValueItem *CreateScalarMethodValueItem(
        panda_file::ItemContainer *container, const Value *value, std::vector<panda_file::ScalarValueItem> *out,
        const Program &program, const std::unordered_map<std::string, panda_file::BaseMethodItem *> &methods);
    static panda_file::ScalarValueItem *CreateScalarLiteralArrayItem(
        panda_file::ItemContainer *container, const Value *value, std::vector<panda_file::ScalarValueItem> *out,
        const Program &program, const std::unordered_map<std::string, panda_file::LiteralArrayItem *> &literalarrays);
    static panda_file::ScalarValueItem *CreateScalarEnumValueItem(
        panda_file::ItemContainer *container, const Value *value, std::vector<panda_file::ScalarValueItem> *out,
        const std::unordered_map<std::string, panda_file::BaseFieldItem *> &fields);
    static panda_file::ScalarValueItem *CreateScalarAnnotationValueItem(
        panda_file::ItemContainer *container, const Value *value, std::vector<panda_file::ScalarValueItem> *out,
        const Program &program, const AsmEmitter::AsmEntityCollections &entities);

    static panda_file::ScalarValueItem *CreateScalarValueItem(panda_file::ItemContainer *container, const Value *value,
                                                              std::vector<panda_file::ScalarValueItem> *out,
                                                              const Program &program,
                                                              const AsmEmitter::AsmEntityCollections &entities);

    static panda_file::ValueItem *CreateValueItem(panda_file::ItemContainer *container, const Value *value,
                                                  const Program &program,
                                                  const AsmEmitter::AsmEntityCollections &entities);

    static panda_file::AnnotationItem *CreateAnnotationItem(panda_file::ItemContainer *container,
                                                            const AnnotationData &annotation, const Program &program,
                                                            const AsmEmitter::AsmEntityCollections &entities);

    static panda_file::MethodHandleItem *CreateMethodHandleItem(
        panda_file::ItemContainer *container, const MethodHandle &mh,
        const std::unordered_map<std::string, panda_file::BaseFieldItem *> &fields,
        const std::unordered_map<std::string, panda_file::BaseMethodItem *> &methods);

    template <class T>
    static bool AddAnnotations(T *item, panda_file::ItemContainer *container, const AnnotationMetadata &metadata,
                               const Program &program, const AsmEmitter::AsmEntityCollections &entities);

    // TODO(mgonopolsky): Refactor to introduce a single error-processing mechanism for parser and emitter
    PANDA_PUBLIC_API static std::string last_error;
};

PANDA_PUBLIC_API std::string GetOwnerName(std::string name);
PANDA_PUBLIC_API std::string GetItemName(std::string name);

}  // namespace panda::pandasm

#endif  // ASSEMBLER_ASSEMBLY_EMITTER_H

