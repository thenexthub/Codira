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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_VN_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_VN_H

#include <unordered_map>
#include <array>
#include "utils/hash.h"
#include "optimizer/pass.h"

namespace panda::compiler {
class Inst;
class VnObject;
class Graph;

class VnObject {
public:
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    explicit VnObject()
    {
        for (uint8_t i = 0; i < MAX_ARRAY_SIZE; i++) {
            objs_[i] = 0;
        }
    }
    NO_MOVE_SEMANTIC(VnObject);
    NO_COPY_SEMANTIC(VnObject);
    ~VnObject() = default;

    void Add(Inst *inst);
    void Add(uint32_t obj);
    void Add(uint64_t obj);
    bool Compare(VnObject *obj);
    uint32_t GetSize() const
    {
        return size_objs_;
    }
    uint64_t GetElement(uint32_t index) const
    {
        ASSERT(index < size_objs_);
        return objs_[index];
    }
    uint32_t *GetArray()
    {
        return objs_.data();
    }

private:
    uint8_t size_objs_ {0};
    // opcode, type, 2 inputs, 2 advanced property
    static constexpr uint8_t MAX_ARRAY_SIZE = 6;
    std::array<uint32_t, MAX_ARRAY_SIZE> objs_;
};

struct VnObjEqual {
    bool operator()(VnObject *obj1, VnObject *obj2) const
    {
        return obj1->Compare(obj2);
    }
};

struct VnObjHash {
    uint32_t operator()(VnObject *obj) const
    {
        return GetHash32(reinterpret_cast<const uint8_t *>(obj->GetArray()), obj->GetSize());
    }
};

/*
 * Optimization assigns numbers(named vn) to all instructions.
 * Equivalent instructions have equivalent vn.
 * If instruction A dominates B and they have equivalent vn, users B are moved to A and DCE removes B at the end.
 * The instruction with the property NO_CSE has unique vn and they can't be removed.
 * The optimization creates VnObject for instruction without NO_CSE.
 * The VnObject is key for the instruction.
 * The unordered_map is used for searching equivalent instruction by the key(VnObject).
 */
class ValNum : public Optimization {
public:
    explicit ValNum(Graph *graph);
    NO_MOVE_SEMANTIC(ValNum);
    NO_COPY_SEMANTIC(ValNum);
    ~ValNum() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "GVN";
    }

    bool IsEnable() const override
    {
        return options.IsCompilerVn();
    }

    void InvalidateAnalyses() override;

    void FindEqualVnOrCreateNew(Inst *inst);

private:
    using InstVector = ArenaVector<Inst *>;

    // Sets vn for the inst
    void SetInstValNum(Inst *inst);

    bool TryToApplyCse(Inst *inst, InstVector *equiv_insts);

    // !TODO add own allocator
    ArenaUnorderedMap<VnObject *, InstVector, VnObjHash, VnObjEqual> map_insts_;

    uint32_t curr_vn_ {0};
    bool cse_is_appied_ {false};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_VN_H
