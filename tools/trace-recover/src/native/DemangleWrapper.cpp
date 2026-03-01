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

#include <string>

#include "CodiraDemangle.h"

std::string Demangle(const std::string &name)
{
    auto demangle = Codira::Demangle(name);
    auto fullName = demangle.GetFullName();
    auto pkgName = demangle.GetPkgName();
    auto demangleName = pkgName + (pkgName.size() > 0 ? "." : "") + fullName;
    return demangleName;
}

extern "C" {
#ifdef USE_BOUNDSCHECK_LIBRARY
// we declare here manually to get rid of header dependency
int memcpy_s(char *dest, size_t destMax, const char *src, size_t count);
#endif

const char *Demangle(char *name, size_t len)
{
    std::string mangle;
    mangle.assign(name, name + len);
    auto demangle = Demangle(mangle);
    char *res = static_cast<char*>(malloc(demangle.size() + 1));
    if (res == nullptr) {
        return "";
    }
    memcpy_s(res, demangle.size() + 1, demangle.c_str(), demangle.size());
    res[demangle.size()] = 0;
    return res;
}
}
