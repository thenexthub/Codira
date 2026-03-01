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

#ifndef LIBPANDABASE_OS_PROPERTY_H
#define LIBPANDABASE_OS_PROPERTY_H

#include <string>

#if defined(PANDA_TARGET_UNIX)
#include "unix/libpandabase/property.h"
#endif  // PANDA_TARGET_UNIX

namespace panda::os::property {

#if defined(PANDA_TARGET_UNIX)
const auto ARK_DFX_PROP = panda::os::unix::property::ARK_DFX_PROP;
const auto ARK_TRACE_PROP = panda::os::unix::property::ARK_TRACE_PROP;

const auto GetPropertyBuffer = panda::os::unix::property::GetPropertyBuffer;
#else
bool GetPropertyBuffer(const char *ark_prop, std::string &out);
#endif  // PANDA_TARGET_UNIX
}  // namespace panda::os::property

#endif  // LIBPANDABASE_OS_PROPERTY_H
