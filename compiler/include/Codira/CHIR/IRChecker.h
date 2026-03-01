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

/*
    type check
    block group block check
    block terminal check
*/

#ifndef CODIRA_CHIR_IRCHECKER_H
#define CODIRA_CHIR_IRCHECKER_H

#include <iostream>

#include "Codira/CHIR/CHIR.h"
#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/Option/Option.h"

namespace Codira::CHIR {
bool IRCheck(const class Package& root, const Codira::GlobalOptions& opts, CHIRBuilder& builder,
    const ToCHIR::Phase& phase, std::ostream& out = std::cerr);
} // namespace Codira::CHIR

#endif
