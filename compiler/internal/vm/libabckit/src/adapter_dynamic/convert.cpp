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

#include "libabckit/src/adapter_dynamic/convert.h"

#include "libabckit/c/metadata_core.h"
#include "libabckit/src/logger.h"

#include "abc2program/abc2program_driver.h"

namespace libabckit::convert {

std::string_view ToString(AbckitTarget target)
{
    switch (target) {
        case ABCKIT_TARGET_UNKNOWN:
            return "unknown";
        case ABCKIT_TARGET_ARK_TS_V1:
            return "ArkTsV1";
        case ABCKIT_TARGET_ARK_TS_V2:
            return "ArkTsV2";
        case ABCKIT_TARGET_TS:
            return "TypeScript";
        case ABCKIT_TARGET_JS:
            return "JavaScript";
        case ABCKIT_TARGET_NATIVE:
            return "native";
    }
    LIBABCKIT_UNREACHABLE;
}

panda::panda_file::SourceLang ToSourceLang(AbckitTarget target)
{
    using SourceLang = panda::panda_file::SourceLang;
    switch (target) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return SourceLang::ARKTS;
        case ABCKIT_TARGET_TS:
            return SourceLang::TYPESCRIPT;
        case ABCKIT_TARGET_JS:
            return SourceLang::JAVASCRIPT;
        case ABCKIT_TARGET_NATIVE:
            LIBABCKIT_UNIMPLEMENTED;
        case ABCKIT_TARGET_UNKNOWN:
            LIBABCKIT_UNREACHABLE;
        default:
            LIBABCKIT_UNIMPLEMENTED;
    }
}

AbckitTarget ToTargetDynamic(panda::panda_file::SourceLang lang)
{
    using SourceLang = panda::panda_file::SourceLang;
    switch (lang) {
        case SourceLang::ECMASCRIPT:
        case SourceLang::JAVASCRIPT:
        case SourceLang::TYPESCRIPT:
            return ABCKIT_TARGET_JS;
        case SourceLang::ARKTS:
            return ABCKIT_TARGET_ARK_TS_V1;
        case SourceLang::PANDA_ASSEMBLY:
            LIBABCKIT_LOG(ERROR) << "Unexpected source lang: PandaAssembly";
            LIBABCKIT_UNREACHABLE;
        default:
            LIBABCKIT_UNIMPLEMENTED;
    }
}

}  // namespace libabckit::convert
