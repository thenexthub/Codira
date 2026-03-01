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

#include "dfx_option.h"

namespace ark::os::dfx_option {

/* static */
bool DfxOptionHandler::IsInOptionList(const std::string &s)
{
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define D(e, v, str)                               \
    /* CC-OFFNXT(G.PRE.10) function scope macro */ \
    if (s == (str)) {                              \
        /* CC-OFFNXT(G.PRE.05) function gen */     \
        return true;                               \
    }
    DFX_OPTION_LIST(D)
#undef D
    return false;
}

/* static */
DfxOptionHandler::DfxOption DfxOptionHandler::DfxOptionFromString(const std::string &s)
{
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define D(e, v, str)                               \
    /* CC-OFFNXT(G.PRE.10) function scope macro */ \
    if (s == (str)) {                              \
        /* CC-OFFNXT(G.PRE.05) function gen */     \
        return DfxOptionHandler::DfxOption::e;     \
    }
    DFX_OPTION_LIST(D)
#undef D
    UNREACHABLE();
}

/* static */
std::string DfxOptionHandler::StringFromDfxOption(DfxOptionHandler::DfxOption dfxOption)
{
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define D(e, v, str)                                   \
    /* CC-OFFNXT(G.PRE.10) function scope macro */     \
    if (dfxOption == DfxOptionHandler::DfxOption::e) { \
        /* CC-OFFNXT(G.PRE.05) function gen */         \
        return (str);                                  \
    }
    DFX_OPTION_LIST(D)
#undef D
    UNREACHABLE();
}

}  // namespace ark::os::dfx_option
