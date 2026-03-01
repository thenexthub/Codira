/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_MEMORY_COALESCING_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_MEMORY_COALESCING_H

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"
#include "compiler_options.h"

namespace ark::compiler {
class MemoryCoalescing : public Optimization {
    using Optimization::Optimization;

public:
    struct CoalescedPair {
        Inst *first;
        Inst *second;
    };

    explicit MemoryCoalescing(Graph *graph, bool aligned = true) : Optimization(graph), alignedOnly_(aligned) {}

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerMemoryCoalescing();
    }

    const char *GetPassName() const override
    {
        return "MemoryCoalescing";
    }

    /// Types of memory accesses that can be coalesced
    static bool AcceptedType(DataType::Type type)
    {
        switch (type) {
            case DataType::UINT32:
            case DataType::INT32:
            case DataType::UINT64:
            case DataType::INT64:
            case DataType::FLOAT32:
            case DataType::FLOAT64:
            case DataType::ANY:
                return true;
            case DataType::REFERENCE:
                return g_options.IsCompilerMemoryCoalescingObjects();
            default:
                return false;
        }
    }

    NO_MOVE_SEMANTIC(MemoryCoalescing);
    NO_COPY_SEMANTIC(MemoryCoalescing);
    ~MemoryCoalescing() override = default;

    static void RemoveAddI(Inst *inst);

private:
    void ReplacePairs(ArenaVector<CoalescedPair> const &pairs);
    void ReplacePair(Inst *first, Inst *second, Inst *insertAfter);

    Inst *ReplaceLoadArray(Inst *first, Inst *second, Inst *insertAfter);
    Inst *ReplaceLoadArrayI(Inst *first, Inst *second, Inst *insertAfter);
    Inst *ReplaceLoadObject(Inst *first, Inst *second, Inst *insertAfter);
    Inst *ReplaceStoreArray(Inst *first, Inst *second, Inst *insertAfter);
    Inst *ReplaceStoreArrayI(Inst *first, Inst *second, Inst *insertAfter);
    Inst *ReplaceStoreObject(Inst *first, Inst *second, Inst *insertAfter);

    void CheckForObjectCandidates(Inst *inst, Inst *obj, uint8_t fieldSize, size_t fieldOffset);

private:
    bool alignedOnly_;
};
}  // namespace ark::compiler

#endif  //  COMPILER_OPTIMIZER_OPTIMIZATIONS_MEMORY_COALESCING_H
