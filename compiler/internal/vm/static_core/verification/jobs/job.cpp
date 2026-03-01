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

#include "job_fill_gen.h"
#include "runtime/include/thread_scopes.h"
#include "verification/absint/absint.h"
#include "verification/cflow/cflow_check.h"
#include "verification/config/debug_breakpoint/breakpoint.h"
#include "verification/jobs/job.h"
#include "verification/type/type_system.h"
#include "verification/public_internal.h"

namespace ark::verifier {
bool Job::UpdateTypes(TypeSystem *types) const
{
    bool result = true;
    Job::ErrorHandler handler;

    auto hasType = [&](Class const *klass) {
        types->MentionClass(klass);
        return true;
    };
    ForAllCachedTypes([&](Type type) {
        if (type.IsClass()) {
            result = result && hasType(type.GetClass());
        }
    });
    ForAllCachedMethods([&](Method const *method) { types->GetMethodSignature(method); });
    ForAllCachedFields([&](Field const *field) {
        result = result && hasType(field->GetClass());
        if (field->GetType().IsReference()) {
            ScopedChangeThreadStatus st(ManagedThread::GetCurrent(), ThreadStatus::RUNNING);
            result = result && hasType(field->ResolveTypeClass(&handler));
        }
    });
    return result;
}

bool Job::Verify(TypeSystem *types) const
{
    auto verifContext = PrepareVerificationContext(types, this);
    auto result = VerifyMethod(verifContext);
    return result != VerificationStatus::ERROR;
}

bool Job::DoChecks(TypeSystem *types)
{
    const auto &check = Options().Check();

    if (check[MethodOption::CheckType::RESOLVE_ID]) {
        if (!ResolveIdentifiers()) {
            LOG(WARNING, VERIFIER) << "Failed to resolve identifiers for method " << method_->GetFullName(true);
            return false;
        }
    }

    if (check[MethodOption::CheckType::CFLOW]) {
        auto cflowInfo = CheckCflow(method_);
        if (!cflowInfo) {
            LOG(WARNING, VERIFIER) << "Failed to check control flow for method " << method_->GetFullName(true);
            return false;
        }
        cflowInfo_ = std::move(cflowInfo);
    }

    DBG_MANAGED_BRK(&service_->debugCtx, method_->GetUniqId(), 0xFFFF);

    if (check[MethodOption::CheckType::TYPING]) {
        if (!UpdateTypes(types)) {
            LOG(WARNING, VERIFIER) << "Cannot update types from cached classes for method "
                                   << method_->GetFullName(true);
            return false;
        }
    }

    if (check[MethodOption::CheckType::ABSINT]) {
        if (!Verify(types)) {
            LOG(WARNING, VERIFIER) << "Abstract interpretation failed for method " << method_->GetFullName(true);
            return false;
        }
    }

    // Clean up temporary types
    types->ResetTypeSpans();

    return true;
}

void Job::ErrorHandler::OnError([[maybe_unused]] ClassLinker::Error error, PandaString const &message)
{
    LOG(ERROR, VERIFIER) << "Cannot link class: " << message;
}

}  // namespace ark::verifier
