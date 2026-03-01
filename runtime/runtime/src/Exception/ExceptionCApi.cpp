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


#include "ExceptionCApi.h"

#include "Base/LogFile.h"
#include "Common/Runtime.h"
#include "Exception.h"
#include "ExceptionManager.h"
#include "Loader/ILoader.h"
#include "Mutator/Mutator.h"
#include "Mutator/MutatorManager.h"
#include "schedule.h"

namespace MapleRuntime {
// If there is an unhandled exception, the exception is thrown.
// This function is used at the end of the C2NStub function.
extern "C" void MRT_ThrowPendingException(void* sigContext)
{
    Mutator* mutator = Mutator::GetMutator();
    if (mutator == nullptr) {
        return;
    }
    ExceptionWrapper& mExceptionWrapper = mutator->GetExceptionWrapper();
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    DLOG(EXCEPTION, "throw pending exception");
    DLOG(EXCEPTION, "Struct ExceptionWrapper");
    DLOG(EXCEPTION, "  ExceptionRef\t:\t%p", mExceptionWrapper.GetExceptionRef());
    DLOG(EXCEPTION, "  isCaught\t:\t%d", mExceptionWrapper.IsCaught());
    DLOG(EXCEPTION, "  typeIndex\t:\t%lu", mExceptionWrapper.GetTypeIndex());
    DLOG(EXCEPTION, "  throwingOOME\t:\t%d", mExceptionWrapper.IsThrowingOOME());
    DLOG(EXCEPTION, "  fatalException\t:\t%d", mExceptionWrapper.IsFatalException());
    DLOG(EXCEPTION, "  ExceptionType\t:\t%s", mExceptionWrapper.GetExceptionTypeName());
#endif
    if (mExceptionWrapper.GetExceptionRef() != nullptr) {
        if (!mExceptionWrapper.IsThrowingSOFE()) {
            LOG(RTLOG_INFO, "Warning : An exception occurred in the Codira function, return value may be invalid!");
        }
        mExceptionWrapper.Reset();
        ExceptionHandling eh(mExceptionWrapper, sigContext);
    }
}
} // namespace MapleRuntime
