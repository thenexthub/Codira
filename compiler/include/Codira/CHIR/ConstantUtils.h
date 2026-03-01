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

/**
 * @file
 *
 * This file declares some utility constants.
 */

#ifndef CODIRA_CHIR_CONSTANTUTILS_H
#define CODIRA_CHIR_CONSTANTUTILS_H

#include <string>

namespace Codira::CHIR {
inline const std::string FUNC_MANGLE_NAME_MALLOC_CSTRING = "_CNat4LibC13mallocCStringHRNat6StringE";
inline const std::string FUNC_MANGLE_NAME_CSTRING_SIZE = "_CNatXk4sizeHv";
inline const std::string GLOBAL_VALUE_PREFIX = "@"; // identifier prefix
constexpr size_t CLASS_REF_DIM{2};
} // namespace Codira::CHIR
#endif // CODIRA_CHIR_CONSTANTUTILS_H
