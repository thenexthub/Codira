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

#ifndef LIBABCKIT_CONVERT_H
#define LIBABCKIT_CONVERT_H

#include "libabckit/c/metadata_core.h"

#include <cstdint>
#include <string_view>

namespace panda::panda_file {
enum class SourceLang : uint8_t;
}  // namespace panda::panda_file

namespace libabckit::convert {

std::string_view ToString(AbckitTarget target);

AbckitTarget ToTargetDynamic(panda::panda_file::SourceLang lang);
panda::panda_file::SourceLang ToSourceLang(AbckitTarget target);

}  // namespace libabckit::convert

#endif  // LIBABCKIT_CONVERT_H
