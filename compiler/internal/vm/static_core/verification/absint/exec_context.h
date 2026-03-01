/**
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

#ifndef PANDA_VERIFIER_ABSINT_EXEC_CONTEXT_HPP_
#define PANDA_VERIFIER_ABSINT_EXEC_CONTEXT_HPP_

#include "reg_context.h"

#include "util/addr_map.h"
#include "util/hash.h"

#include "include/mem/panda_containers.h"

namespace ark::verifier {

enum class EntryPointType : size_t { METHOD_BODY, EXCEPTION_HANDLER, LAST = EXCEPTION_HANDLER };

class ExecContext {
public:
    enum class Status { OK, ALL_DONE, NO_ENTRY_POINTS_WITH_CONTEXT };

    bool HasContext(const uint8_t *addr) const
    {
        return regContextOnCheckPoint_.count(addr) > 0;
    }

    bool IsCheckPoint(const uint8_t *addr) const
    {
        return checkPoint_.HasMark(addr);
    }

    void AddEntryPoint(const uint8_t *addr, EntryPointType type)
    {
        entryPoint_.insert({addr, type});
    }

    template <typename Reporter>
    void StoreCurrentRegContextForAddr(const uint8_t *addr, Reporter reporter)
    {
        if (HasContext(addr)) {
            StoreCurrentRegContextForAddrIfHasContext(addr, reporter);
        } else if (IsCheckPoint(addr)) {
            regContextOnCheckPoint_[addr] = currentRegContext_;
        }
    }

    template <typename Reporter>
    void StoreCurrentRegContextForAddrIfHasContext(const uint8_t *addr, Reporter reporter)
    {
        RegContext &ctx = regContextOnCheckPoint_[addr];
        auto lub = RcUnion(&ctx, &currentRegContext_, typeSystem_);
        if (lub.HasInconsistentRegs()) {
            for (int regIdx : lub.InconsistentRegsNums()) {
                if (!reporter(regIdx, currentRegContext_[regIdx], ctx[regIdx])) {
                    break;
                }
            }
        }

        ctx.UnionWith(&currentRegContext_, typeSystem_);

        if (ctx.HasInconsistentRegs()) {
            ctx.RemoveInconsistentRegs();
        }
    }

    void StoreCurrentRegContextForAddr(const uint8_t *addr)
    {
        if (HasContext(addr)) {
            RegContext &ctx = regContextOnCheckPoint_[addr];
            ctx.UnionWith(&currentRegContext_, typeSystem_);
            ctx.RemoveInconsistentRegs();
        } else if (IsCheckPoint(addr)) {
            regContextOnCheckPoint_[addr] = currentRegContext_;
        }
    }

    template <typename Reporter>
    void ProcessJump(const uint8_t *jmpInsnPtr, const uint8_t *targetPtr, Reporter reporter, EntryPointType codeType)
    {
        if (!processedJumps_.HasMark(jmpInsnPtr)) {
            processedJumps_.Mark(jmpInsnPtr);
            AddEntryPoint(targetPtr, codeType);
            StoreCurrentRegContextForAddr(targetPtr, reporter);
        } else {
            RegContext &targetCtx = regContextOnCheckPoint_[targetPtr];
            bool typeUpdated = targetCtx.UnionWith(&currentRegContext_, typeSystem_);
            if (typeUpdated) {
                AddEntryPoint(targetPtr, codeType);
            }
        }
    }

    void ProcessJump(const uint8_t *jmpInsnPtr, const uint8_t *targetPtr, EntryPointType codeType)
    {
        if (!processedJumps_.HasMark(jmpInsnPtr)) {
            processedJumps_.Mark(jmpInsnPtr);
            AddEntryPoint(targetPtr, codeType);
            StoreCurrentRegContextForAddr(targetPtr);
        } else {
            RegContext &targetCtx = regContextOnCheckPoint_[targetPtr];
            bool typeUpdated = targetCtx.UnionWith(&currentRegContext_, typeSystem_);
            if (typeUpdated) {
                AddEntryPoint(targetPtr, codeType);
            }
        }
    }

    const RegContext &RegContextOnTarget(const uint8_t *addr) const
    {
        auto ctx = regContextOnCheckPoint_.find(addr);
        ASSERT(ctx != regContextOnCheckPoint_.cend());
        return ctx->second;
    }

    Status GetEntryPointForChecking(const uint8_t **entry, EntryPointType *entryType)
    {
        for (auto [addr, type] : entryPoint_) {
            if (HasContext(addr)) {
                *entry = addr;
                *entryType = type;
                currentRegContext_ = RegContextOnTarget(addr);
                entryPoint_.erase({addr, type});
                return Status::OK;
            }
        }
        if (entryPoint_.empty()) {
            return Status::ALL_DONE;
        }
        return Status::NO_ENTRY_POINTS_WITH_CONTEXT;
    }

    RegContext &CurrentRegContext()
    {
        return currentRegContext_;
    }

    const RegContext &CurrentRegContext() const
    {
        return currentRegContext_;
    }

    void SetCheckPoint(const uint8_t *addr)
    {
        checkPoint_.Mark(addr);
    }

    template <typename Fetcher>
    void SetCheckPoints(Fetcher fetcher)
    {
        while (auto tgt = fetcher()) {
            SetCheckPoint(*tgt);
        }
    }

    template <typename Handler>
    void ForContextsOnCheckPointsInRange(const uint8_t *from, const uint8_t *to, Handler handler)
    {
        checkPoint_.EnumerateMarksInScope<const uint8_t *>(from, to, [&handler, this](const uint8_t *ptr) {
            if (HasContext(ptr)) {
                return handler(ptr, regContextOnCheckPoint_[ptr]);
            }
            return true;
        });
    }

    ExecContext(const uint8_t *pcStartPtr, const uint8_t *pcEndPtr, TypeSystem *typeSystem)
        : checkPoint_ {pcStartPtr, pcEndPtr}, processedJumps_ {pcStartPtr, pcEndPtr}, typeSystem_ {typeSystem}
    {
    }

    DEFAULT_MOVE_SEMANTIC(ExecContext);
    DEFAULT_COPY_SEMANTIC(ExecContext);
    ~ExecContext() = default;

private:
    AddrMap checkPoint_;
    AddrMap processedJumps_;
    // Use an ordered set to make iteration over elements reproducible.
    PandaSet<std::pair<const uint8_t *, EntryPointType>> entryPoint_;
    PandaUnorderedMap<const uint8_t *, RegContext> regContextOnCheckPoint_;
    TypeSystem *typeSystem_;
    RegContext currentRegContext_;
};
}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_ABSINT_EXEC_CONTEXT_HPP_
