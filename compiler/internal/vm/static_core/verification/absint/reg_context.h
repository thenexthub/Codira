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

#ifndef PANDA_VERIFIER_ABSINT_REG_CONTEXT_HPP_
#define PANDA_VERIFIER_ABSINT_REG_CONTEXT_HPP_

#include "value/abstract_typed_value.h"

#include "util/index.h"
#include "util/lazy.h"
#include "util/str.h"
#include "util/shifted_vector.h"

#include "libarkbase/macros.h"

#include <algorithm>

namespace ark::verifier {
/*
Design decisions:
1. regs - unordered map, for speed (compared to map) and space efficiency (compared to vector)
   after implementing sparse vectors - rebase on them (taking into consideration immutability, see immer)
*/

// NOTE(vdyadov): correct handling of values origins during LUB operation

class RegContext {
public:
    explicit RegContext() = default;
    explicit RegContext(size_t size) : regs_(size) {}
    ~RegContext() = default;

    DEFAULT_MOVE_SEMANTIC(RegContext);
    DEFAULT_COPY_SEMANTIC(RegContext);

    bool UnionWith(RegContext const *rhs, TypeSystem *tsys)
    {
        bool updated = false;
        auto lhsIt = regs_.begin();
        auto rhsIt = rhs->regs_.begin();
        while (lhsIt != regs_.end() && rhsIt != rhs->regs_.end()) {
            if (!(*lhsIt).IsNone() && !(*rhsIt).IsNone()) {
                auto before = *lhsIt;
                auto result = AtvJoin(&*lhsIt, &*rhsIt, tsys);
                updated |= (result.GetAbstractType() != before.GetAbstractType());
                *lhsIt = result;
            } else {
                updated |= !(*lhsIt).IsNone();
                *lhsIt = AbstractTypedValue {};
            }
            ++lhsIt;
            ++rhsIt;
        }
        while (lhsIt != regs_.end()) {
            updated = true;
            *lhsIt = AbstractTypedValue {};
            ++lhsIt;
        }
        return updated;
    }
    void ChangeValuesOfSameOrigin(int idx, const AbstractTypedValue &atv)
    {
        if (!regs_.InValidRange(idx)) {
            regs_[idx] = atv;
            return;
        }
        auto oldAtv = regs_[idx];
        if (oldAtv.IsNone()) {
            regs_[idx] = atv;
            return;
        }
        const auto &oldOrigin = oldAtv.GetOrigin();
        if (!oldOrigin.IsValid()) {
            regs_[idx] = atv;
            return;
        }
        auto it = regs_.begin();
        while (it != regs_.end()) {
            if (!(*it).IsNone()) {
                const auto &origin = (*it).GetOrigin();
                if (origin.IsValid() && origin == oldOrigin) {
                    *it = atv;
                }
            }
            ++it;
        }
    }
    AbstractTypedValue &operator[](int idx)
    {
        if (!regs_.InValidRange(idx)) {
            regs_.ExtendToInclude(idx);
        }
        return regs_[idx];
    }
    AbstractTypedValue operator[](int idx) const
    {
        ASSERT(IsRegDefined(idx) && regs_.InValidRange(idx));
        return regs_[idx];
    }
    size_t Size() const
    {
        size_t size = 0;
        EnumerateAllRegs([&size](auto, auto) {
            ++size;
            return true;
        });
        return size;
    }
    template <typename Callback>
    void EnumerateAllRegs(Callback cb) const
    {
        for (int idx = regs_.BeginIndex(); idx < regs_.EndIndex(); ++idx) {
            if (!regs_[idx].IsNone()) {
                const auto &atv = regs_[idx];
                if (!cb(idx, atv)) {
                    return;
                }
            }
        }
    }
    template <typename Callback>
    void EnumerateAllRegs(Callback cb)
    {
        for (int idx = regs_.BeginIndex(); idx < regs_.EndIndex(); ++idx) {
            if (!regs_[idx].IsNone()) {
                auto &atv = regs_[idx];
                if (!cb(idx, atv)) {
                    return;
                }
            }
        }
    }
    bool HasInconsistentRegs() const
    {
        bool result = false;

        EnumerateAllRegs([&](int, const auto &av) {
            if (!av.IsConsistent()) {
                result = true;
                return false;
            }
            return true;
        });
        return result;
    }
    auto InconsistentRegsNums() const
    {
        PandaVector<int> result;
        EnumerateAllRegs([&](int num, const auto &av) {
            if (!av.IsConsistent()) {
                result.push_back(num);
            }
            return true;
        });
        return result;
    }
    bool IsRegDefined(int num) const
    {
        return regs_.InValidRange(num) && !regs_[num].IsNone();
    }
    bool IsValid(int num) const
    {
        return regs_.InValidRange(num);
    }
    bool WasConflictOnReg(int num) const
    {
        return conflictingRegs_.count(num) > 0;
    }
    void Clear()
    {
        regs_.clear();
        conflictingRegs_.clear();
    }
    void RemoveInconsistentRegs()
    {
        EnumerateAllRegs([this](int num, auto &atv) {
            if (!atv.IsConsistent()) {
                conflictingRegs_.insert(num);
                atv = AbstractTypedValue {};
            } else {
                conflictingRegs_.erase(num);
            }
            return true;
        });
    }
    PandaString DumpRegs(TypeSystem const *tsys) const
    {
        PandaString logString {};
        bool comma = false;
        EnumerateAllRegs([&comma, &logString, &tsys](int num, const auto &absTypeVal) {
            PandaString result {num == -1 ? "acc" : "v" + NumToStr(num)};
            result += " : ";
            result += absTypeVal.ToString(tsys);
            if (comma) {
                logString += ", ";
            }
            logString += result;
            comma = true;
            return true;
        });
        return logString;
    }

private:
    ShiftedVector<1, AbstractTypedValue> regs_;

    // NOTE(vdyadov): After introducing sparse bit-vectors, change ConflictingRegs_ type.
    PandaUnorderedSet<int> conflictingRegs_;

    friend RegContext RcUnion(RegContext const *lhs, RegContext const *rhs, TypeSystem * /* tsys */);
};

inline RegContext RcUnion(RegContext const *lhs, RegContext const *rhs, TypeSystem *tsys)
{
    RegContext result(std::max(lhs->regs_.size(), rhs->regs_.size()));
    auto resultIt = result.regs_.begin();
    for (auto lhsIt = lhs->regs_.begin(), rhsIt = rhs->regs_.begin();
         lhsIt != lhs->regs_.end() && rhsIt != rhs->regs_.end(); ++lhsIt, ++rhsIt, ++resultIt) {
        if (!(*lhsIt).IsNone() && !(*rhsIt).IsNone()) {
            *resultIt = AtvJoin(&*lhsIt, &*rhsIt, tsys);
        }
    }
    return result;
}

}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_ABSINT_REG_CONTEXT_HPP_
