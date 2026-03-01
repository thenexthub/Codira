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

#include <algorithm>
#include <array>
#include <string_view>

#include "irtoc_function_utils.h"

using namespace std::literals::string_view_literals;  // Enables 'sv' suffix

constexpr std::array NOALIAS_IRTOC_FUNC = {
    "CreateObjectByIdEntrypoint"sv,
    "CreateObjectByClassInterpreter"sv,
    "CreateArrayByIdEntrypoint"sv,
    "CreateMultiDimensionalArrayById"sv,
    "AnyLdbyname"sv,
    "AnyLdbyidx"sv,
    "AnyLdbyval"sv,
    "AnyLdByValEntrypoint"sv,
    "AnyCall0"sv,
    "AnyCallRange"sv,
    "AnyCallShort"sv,
    "AnyCallThis0"sv,
    "AnyCallThisRange"sv,
    "AnyCallThisShort"sv,
    "AnyCallNew0"sv,
    "AnyCallNewRange"sv,
    "AnyCallNewShort"sv,
#ifdef PANDA_WITH_ETS
    "LookupGetterByNameShortEntrypoint"sv,
    "LookupGetterByNameLongEntrypoint"sv,
    "LookupGetterByNameObjEntrypoint"sv,
    "LookupSetterByNameShortEntrypoint"sv,
    "LookupSetterByNameLongEntrypoint"sv,
    "LookupSetterByNameObjEntrypoint"sv,
    "LookupFieldByNameEntrypoint"sv,
    "EtsGetTypeofEntrypoint"sv,
    "EtsGetIstrueEntrypoint"sv,
    "LookupMethodByNameEntrypoint"sv,
#endif
};

constexpr std::array PTR_IGN_IRTOC_FUNC = {
    "ResolveStringByIdEntrypoint"sv,
    "ResolveLiteralArrayByIdEntrypoint"sv,
    "ResolveTypeByIdEntrypoint"sv,
    "GetCalleeMethodFromBytecodeId"sv,
    "GetInstructionsByMethod"sv,
    "GetFieldByIdEntrypoint"sv,
    "GetStaticFieldByIdEntrypoint"sv,
    "FindCatchBlockInIFrames"sv,
    "ResolveVirtualMethod"sv,
    "GetMethodClassById"sv,
    "VmCreateString"sv,
    "AllocFrameInterp"sv,
    "AllocFrameStackOverflow"sv,
    "InitializeFrame"sv,
    "memmove"sv,
};

namespace ark::llvmbackend::irtoc_function_utils {
bool IsNoAliasIrtocFunction(const std::string &externalName)
{
    return std::find(NOALIAS_IRTOC_FUNC.begin(), NOALIAS_IRTOC_FUNC.end(), externalName) != NOALIAS_IRTOC_FUNC.end();
}
bool IsPtrIgnIrtocFunction(const std::string &externalName)
{
    return std::find(PTR_IGN_IRTOC_FUNC.begin(), PTR_IGN_IRTOC_FUNC.end(), externalName) != PTR_IGN_IRTOC_FUNC.end();
}
}  // namespace ark::llvmbackend::irtoc_function_utils
