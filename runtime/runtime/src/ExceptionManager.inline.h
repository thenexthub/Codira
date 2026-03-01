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


#ifndef MRT_EXCEPTION_MANAGER_INLINE_H
#define MRT_EXCEPTION_MANAGER_INLINE_H

#include "Base/LogFile.h"
#include "Exception/Exception.h"
#include "ExceptionManager.h"
#include "Mutator/Mutator.h"
#include "Mutator/MutatorManager.h"
#include "schedule.h"

namespace MapleRuntime {
inline ExceptionRef ExceptionManager::GetPendingException()
{
    ExceptionWrapper& mExceptionWrapper = Mutator::GetMutator()->GetExceptionWrapper();
    return mExceptionWrapper.GetExceptionRef();
}

inline void ExceptionManager::ClearPendingException()
{
    ExceptionWrapper& mExceptionWrapper = Mutator::GetMutator()->GetExceptionWrapper();
    mExceptionWrapper.Reset();
    mExceptionWrapper.SetExceptionRef(nullptr);
}

inline void ExceptionManager::ThrowPendingException() { MRT_ThrowPendingException(); }

inline bool ExceptionManager::HasPendingException()
{
    ExceptionWrapper& mExceptionWrapper = Mutator::GetMutator()->GetExceptionWrapper();
    return (mExceptionWrapper.GetExceptionRef() != nullptr);
}

inline bool ExceptionManager::HasFatalException()
{
    ExceptionWrapper& eWrapper = Mutator::GetMutator()->GetExceptionWrapper();
    return eWrapper.IsFatalException();
}

inline void ExceptionManager::CheckAndThrowPendingException(const CString& msg)
{
    CHECK_DETAIL(ExceptionManager::HasPendingException(), "%s, but there is no pending exception.", msg.Str());
    ExceptionManager::ThrowPendingException();
}

inline void ExceptionManager::CheckAndDumpException()
{
    if (ExceptionManager::HasPendingException()) {
        ExceptionManager::DumpException();
    }
}

inline void* ExceptionManager::GetExceptionWrapper()
{
    ExceptionWrapper& mExceptionWrapper = Mutator::GetMutator()->GetExceptionWrapper();
    return reinterpret_cast<void*>(&mExceptionWrapper);
}

inline uint32_t ExceptionManager::GetExceptionTypeID()
{
    ExceptionWrapper& mExceptionWrapper = Mutator::GetMutator()->GetExceptionWrapper();
    return mExceptionWrapper.GetTypeIndex();
}

inline void ExceptionManager::EndCatch() {}
} // namespace MapleRuntime
#endif // MRT_EXCEPTION_MANAGER_INLINE_H
