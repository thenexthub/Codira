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

#ifndef CPP_ABCKIT_STATIC_ISA_IMPL_H
#define CPP_ABCKIT_STATIC_ISA_IMPL_H

#include "static_isa.h"

// NOLINTBEGIN(performance-unnecessary-value-param)
namespace abckit {

inline AbckitIsaApiStaticOpcode StaticIsa::GetOpCode(Instruction inst) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitIsaApiStaticOpcode opc = conf->cStatapi_->iGetOpcode(inst.GetView());
    CheckError(conf);
    return opc;
}

}  // namespace abckit

// NOLINTEND(performance-unnecessary-value-param)

#endif  // CPP_ABCKIT_STATIC_ISA_IMPL_H
