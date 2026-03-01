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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_VN_H
#define COMPILER_OPTIMIZER_ANALYSIS_VN_H

#include "libarkbase/utils/hash.h"
#include "optimizer/pass.h"
#include <unordered_map>
#include <array>
#include "optimizer/ir/analysis.h"

namespace ark::compiler {
class Inst;
class VnObject;
class Graph;

class VnObject {
public:
    using HalfObjType = uint16_t;
    using ObjType = uint32_t;
    using DoubleObjType = uint64_t;

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

    void Set(Inst *inst);
    void Add(HalfObjType obj1, HalfObjType obj2);
    void Add(ObjType obj);
    void Add(DoubleObjType obj);
    bool Compare(VnObject *obj);
    uint32_t GetSize() const
    {
        return sizeObjs_;
    }
    ObjType GetElement(uint32_t index) const
    {
        ASSERT(index < sizeObjs_);
        return objs_[index];
    }
    ObjType *GetArray()
    {
        return objs_.data();
    }

private:
    uint8_t sizeObjs_ {0};
    // opcode, type, 2 inputs, 3 advanced property
    static constexpr uint8_t MAX_ARRAY_SIZE = 7;
    std::array<ObjType, MAX_ARRAY_SIZE> objs_;
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
        return GetHash32(reinterpret_cast<const uint8_t *>(obj->GetArray()),
                         obj->GetSize() * sizeof(VnObject::ObjType));
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
class PANDA_PUBLIC_API ValNum : public Optimization {
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
        return g_options.IsCompilerVn();
    }

    void InvalidateAnalyses() override;

    void FindEqualVnOrCreateNew(Inst *inst);

private:
    using InstVector = ArenaVector<Inst *>;

    // Sets vn for the inst
    void SetInstValNum(Inst *inst);

    bool TryToApplyCse(Inst *inst, InstVector *equivInsts);

    // !NOTE add own allocator
    ArenaUnorderedMap<VnObject *, InstVector, VnObjHash, VnObjEqual> mapInsts_;
    SaveStateBridgesBuilder ssb_;
    uint32_t currVn_ {0};
    bool cseIsApplied_ {false};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_VN_H
