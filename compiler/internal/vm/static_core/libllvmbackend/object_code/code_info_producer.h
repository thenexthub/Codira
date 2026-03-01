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

#ifndef LIBLLVMBACKEND_OBJECT_CODE_CODE_INFO_PRODUCER_H
#define LIBLLVMBACKEND_OBJECT_CODE_CODE_INFO_PRODUCER_H

#include "compiler/optimizer/ir/runtime_interface.h"
#include "libarkbase/utils/bit_vector.h"

#include "created_object_file.h"

#include <llvm/Object/FaultMapParser.h>
#include <llvm/Object/StackMapParser.h>

namespace ark {
class Method;
}  // namespace ark

namespace ark::compiler {
class CodeInfoBuilder;
}  // namespace ark::compiler

namespace ark::llvmbackend {

class LLVMArkInterface;

class CodeInfoProducer {
private:
    enum LocationHeader {
        LOCATION_CC,
        LOCATION_FLAGS,
        LOCATION_DEOPT_COUNT,
        LOCATION_INLINE_DEPTH,
        LOCATION_INLINE_TABLE
    };
    enum InlineRecord { INLINE_METHOD_ID, INLINE_BPC, INLINE_NEED_REGMAP, INLINE_VREG_COUNT, INLINE_VREGS };
    enum VRegRecord { VREG_IDX, VREG_TYPE, VREG_VALUE, VREG_RECORD_SIZE };

public:
    using LLVMStackMap = llvm::StackMapParser<llvm::support::little>;
    using Function = LLVMStackMap::FunctionAccessor;
    using Record = LLVMStackMap::RecordAccessor;
    using Location = LLVMStackMap::LocationAccessor;
    using StackMapSymbol = ark::llvmbackend::CreatedObjectFile::StackMapSymbol;
    using CodeInfoBuilder = ark::compiler::CodeInfoBuilder;

    explicit CodeInfoProducer(Arch arch, LLVMArkInterface *compilation);

    void SetStackMap(const uint8_t *section, uintptr_t size);
    void SetFaultMaps(const uint8_t *section, uintptr_t size);

    void AddSymbol(Method *method, StackMapSymbol symbol);
    void AddFaultMapSymbol(Method *method, uint32_t symbol);

    void Produce(Method *method, ark::compiler::CodeInfoBuilder *builder) const;

private:
    static void DumpStackMap(const std::unique_ptr<const LLVMStackMap> &stackmap, std::ostream &stream);
    static void DumpStackMapFunction(const Function &function, std::ostream &stream, const std::string &prefix = "");
    static void DumpStackMapRecord(const Record &record, std::ostream &stream, const std::string &prefix = "");
    static void DumpStackMapLocation(const Location &location, std::ostream &stream, const std::string &prefix = "");

    uint16_t GetDwarfBase(Arch arch) const;

    size_t GetArkFrameSlot(const Location &location, uint64_t stackSize, size_t slotSize) const;

    unsigned CollectRoots(const Record &record, uint64_t stackSize, ArenaBitVector *stack) const;

    void BuildSingleRegMap(CodeInfoBuilder *builder, const Record &record, int32_t methodIdIndex, int32_t vregsCount,
                           uint64_t stackSize) const;
    void BuildRegMap(CodeInfoBuilder *builder, const Record &record, uint64_t stackSize) const;

    void ConvertStackMaps(Method *method, CodeInfoBuilder *builder) const;
    void EncodeNullChecks(Method *method, CodeInfoBuilder *builder) const;

private:
    const Arch arch_;
    LLVMArkInterface *compilation_;
    std::unique_ptr<const LLVMStackMap> stackmap_;
    std::unordered_map<Method *, StackMapSymbol> symbols_;
    std::unordered_map<Method *, uint32_t> faultMapSymbols_;
    std::unique_ptr<llvm::FaultMapParser> faultMapParser_;
};
}  // namespace ark::llvmbackend
#endif  // LIBLLVMBACKEND_OBJECT_CODE_CODE_INFO_PRODUCER_H
