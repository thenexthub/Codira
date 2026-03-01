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

#ifndef JSVM_DFX_H
#define JSVM_DFX_H

#include <cassert>
#include <unordered_set>
#include <vector>

#include "jsvm_log.h"
#include "jsvm_version.h"
#include "platform/platform.h"
#include "jsvm_types.h"

// v8 header
#include "v8.h"

#define JSVM_FATAL(message)                                                                      \
    do {                                                                                         \
        /* Make sure that this struct does not end up in inline code, but      */                \
        /* rather in a read-only data section when modifying this code.        */                \
        jsvm::OnFatalError(STRINGIFY(__FILE__) ":" STRINGIFY(__LINE__) " ", STRINGIFY(message)); \
    } while (0)

namespace jsvm {
[[noreturn]] inline void OnFatalError(const char* location, const char* message)
{
    LOG(Fatal) << "JSVM Fatal Error Position : " << (location ? location : "Unkown");
    LOG(Fatal) << "JSVM Fatal Error Message : " << (message ? message : "Unkown");
    platform::OS::Abort();
}

class DebugSealHandleScope {
public:
    explicit inline DebugSealHandleScope(v8::Isolate* isolate = nullptr)
#ifdef DEBUG
        : sealHandleScope(isolate != nullptr ? isolate : v8::Isolate::GetCurrent())
#endif
    {}

private:
#ifdef DEBUG
    v8::SealHandleScope sealHandleScope;
#endif
};

/* A ScopeLifecycleTracker can record the depths of different scopes and */
/* the JSVM_Value type variables within each scope.                      */
class ScopeLifecycleTracker {
public:
    uint32_t GetCurrentScopeDepth() const
    {
        return scopeDepthToVal.size();
    }

    void IncHandleScopeDepth()
    {
        scopeDepthToVal.push_back(std::vector<JSVM_Value>());
    }

    void DecHandleScopeDepth()
    {
        scopeDepthToVal.pop_back();
    }

    void ReleaseJSVMVals()
    {
        if (scopeDepthToVal.size() == 0) {
            JSVM_FATAL("Unpaired HandleScope detected after scope check is enabled!");
        }
        for (auto item : scopeDepthToVal[scopeDepthToVal.size() - 1]) {
            addedVal.erase(item);
        }
    }

    void AddJSVMVal(JSVM_Value val, bool isEscape = false)
    {
        if (scopeDepthToVal.size() == 0) {
            JSVM_FATAL("Unpaired HandleScope detected after scope check is enabled!");
        }
        addedVal.insert(val);
        if (!isEscape) {
            // Add JSVM value to current depth
            scopeDepthToVal[scopeDepthToVal.size() - 1].push_back(val);
        } else {
            // Add JSVM value to parent depth
            constexpr size_t depth = 2;
            if (scopeDepthToVal.size() < depth) {
                JSVM_FATAL("Not in any scope!");
            }
            scopeDepthToVal[scopeDepthToVal.size() - depth].push_back(val);
        }
    }

    bool CheckJSVMVal(JSVM_Value val)
    {
        auto it = addedVal.find(val);
        if (it != addedVal.end()) {
            return true;
        } else {
            return false;
        }
    }

private:
    std::unordered_set<JSVM_Value> addedVal;
    std::vector<std::vector<JSVM_Value>> scopeDepthToVal;
};

} // namespace jsvm

constexpr uint16_t FUNCTIONNAME_MAX = 1024;

struct JsvmStepParam {
    uintptr_t *fp;
    uintptr_t *sp;
    uintptr_t *pc;
    bool *isJsvmFrame;

    JsvmStepParam(uintptr_t *fp, uintptr_t *sp, uintptr_t *pc, bool *isJsvmFrame)
        : fp(fp), sp(sp), pc(pc), isJsvmFrame(isJsvmFrame) {}
};
struct JsvmFunction {
    char functionName[FUNCTIONNAME_MAX];
};

typedef bool (*ReadMemFunc)(void *ctx, uintptr_t addr, uintptr_t *val);

extern "C" int step_jsvm(void *ctx, ReadMemFunc readMem, JsvmStepParam *frame);

extern "C" int create_jsvm_extractor(uintptr_t *extractorPptr, uint32_t pid);

extern "C" int destroy_jsvm_extractor(uintptr_t extractorPtr);

extern "C" int jsvm_parse_js_frame_info(uintptr_t pc,
                                        uintptr_t jsvmExtractorPtr,
                                        JsvmFunction *jsvmFunction);


#define UNREACHABLE(...) JSVM_FATAL("Unreachable code reached" __VA_OPT__(": ") __VA_ARGS__)

#ifdef __GNUC__
#define LIKELY(expr) __builtin_expect(!!(expr), 1)
#define UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#else
#define LIKELY(expr) expr
#define UNLIKELY(expr) expr
#endif

#define CHECK(expr)              \
    do {                         \
        if (UNLIKELY(!(expr))) { \
            JSVM_FATAL(expr);    \
        }                        \
    } while (0)

#define CHECK_EQ(a, b) CHECK((a) == (b))
#define CHECK_NE(a, b) CHECK((a) != (b))
#define CHECK_LE(a, b) CHECK((a) <= (b))
#define CHECK_GE(a, b) CHECK((a) >= (b))
#define CHECK_LT(a, b) CHECK((a) < (b))
#define CHECK_GT(a, b) CHECK((a) > (b))
#define CHECK_NULL(val) CHECK((val) == nullptr)
#define CHECK_NOT_NULL(val) CHECK((val) != nullptr)
#define CHECK_IMPLIES(a, b) CHECK(!(a) || (b))

#ifdef DEBUG
#define DCHECK(expr) CHECK(expr)
#define DCHECK_EQ(a, b) CHECK_EQ(a, b)
#define DCHECK_NE(a, b) CHECK_NE(a, b)
#define DCHECK_LE(a, b) CHECK_LE(a, b)
#define DCHECK_GE(a, b) CHECK_GE(a, b)
#define DCHECK_LT(a, b) CHECK_LT(a, b)
#define DCHECK_GT(a, b) CHECK_GT(a, b)
#define DCHECK_NULL(val) CHECK_NULL(val)
#define DCHECK_NOT_NULL(val) CHECK_NOT_NULL(val)
#define DCHECK_IMPLIES(a, b) CHECK_IMPLIES(a, b)
#else
#define DCHECK(expr)
#define DCHECK_EQ(a, b)
#define DCHECK_NE(a, b)
#define DCHECK_LE(a, b)
#define DCHECK_GE(a, b)
#define DCHECK_LT(a, b)
#define DCHECK_GT(a, b)
#define DCHECK_NULL(val)
#define DCHECK_NOT_NULL(val)
#define DCHECK_IMPLIES(a, b)
#endif

#endif