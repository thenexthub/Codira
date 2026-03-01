
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

#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/ani/verify/types/internal_ref.h"
#include "plugins/ets/runtime/ani/verify/types/vref.h"

namespace ark::ets::ani::verify {

VRef *InternalRef::CastToVRef(InternalRef *iref)
{
    ASSERT(((common::ToUintPtr(iref) & TYPE_MASK) != TYPE_VREF) || ManagedCodeAccessor::IsUndefined(iref->ref_));
    return reinterpret_cast<VRef *>((common::ToUintPtr(iref) | TYPE_VREF));
}

InternalRef *InternalRef::CastFromVRef(VRef *vref)
{
    ASSERT((common::ToUintPtr(vref) & TYPE_MASK) == TYPE_VREF);
    return reinterpret_cast<InternalRef *>((common::ToUintPtr(vref) & ~TYPE_MASK));
}

bool InternalRef::IsUndefinedStackRef(VRef *vref)
{
    return reinterpret_cast<EtsReference *>(vref) == EtsReference::GetUndefined();
}

bool InternalRef::IsStackVRef(VRef *vref)
{
    if (IsUndefinedStackRef(vref)) {
        return true;
    }
    return (common::ToUintPtr(vref) & TYPE_MASK) != TYPE_VREF;
}

}  // namespace ark::ets::ani::verify
