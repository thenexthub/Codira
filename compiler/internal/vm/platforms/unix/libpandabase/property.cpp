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

#include "platforms/unix/libpandabase/property.h"

#ifdef PANDA_TARGET_MOBILE
#include <sys/system_properties.h>
#endif  // PANDA_TARGET_MOBILE

namespace panda::os::unix::property {

bool GetPropertyBuffer([[maybe_unused]] const char *ark_prop, [[maybe_unused]] std::string &out)
{
#ifdef PANDA_TARGET_MOBILE
    char buf[PROP_VALUE_MAX] = {};
    if (__system_property_get(ark_prop, buf) > 0) {
        out = buf;
        return true;
    }
#endif  // PANDA_TARGET_MOBILE
    return false;
}

}  // namespace panda::os::unix::property
