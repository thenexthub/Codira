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

#include "obfuscator.h"

#include "element_obfuscator.h"
#include "element_preprocessor.h"
#include "logger.h"
#include "member_linker.h"
#include "member_obfuscator.h"
#include "member_preprocessor.h"
#include "name_cache/name_cache_dumper.h"
#include "name_cache/name_cache_keeper.h"
#include "name_cache/name_cache_parser.h"
#include "name_marker.h"
#include "remove_log_obfuscator.h"
#include "name_mapping/name_mapping_manager.h"
#include "seeds_dumper.h"
#include "util/assert_util.h"

bool ark::guard::Obfuscator::Execute(abckit_wrapper::FileView &fileView)
{
    // member link
    MemberLinker memberLinker(fileView);
    GUARD_ASSERT(memberLinker.Link() != ErrorCode::ERR_SUCCESS, ErrorCode::GENERIC_ERROR, "member link failed");
    LOG_I << "member link success";

    // name mark: file name, path and class_specification keep
    NameMarker nameMarker(config_);
    nameMarker.Execute(fileView);
    LOG_I << "name mark success";

    if (config_.HasKeepConfiguration()) {
        // dump print seeds
        SeedsDumper seedsDumper(config_.GetPrintSeedsOption());
        seedsDumper.Dump(fileView);
        LOG_I << "seeds dump success";
    }

    // name mark: name cache read and set
    NameCacheParser parser(config_.GetApplyNameCache());
    parser.Parse();
    LOG_I << "name cache parse success";

    NameCacheKeeper keeper(fileView);
    keeper.Process(parser.GetNameCache());
    LOG_I << "name cache keep success";

    // obfuscate preparation
    NameMappingManager memberNameMapping;
    MemberPreProcessor memberPreProcessor(memberNameMapping);
    GUARD_ASSERT(!memberPreProcessor.Process(fileView), ErrorCode::GENERIC_ERROR,
                     "member obfuscate prepare failed");
    LOG_I << "member preprocess success";

    NameMappingManager elementNameMapping;
    ElementPreProcessor elementPreProcessor(elementNameMapping);
    GUARD_ASSERT(!elementPreProcessor.Process(fileView), ErrorCode::GENERIC_ERROR,
                     "element obfuscate prepare failed");
    LOG_I << "element preprocess success";

    // remove log obfuscator
    if (config_.IsRemoveLogObfEnabled()) {
        RemoveLogObfuscator removeLogObfuscator;
        GUARD_ASSERT(!removeLogObfuscator.Execute(fileView), ErrorCode::GENERIC_ERROR,
                         "remove log obfuscate execute failed");
        LOG_I << "remove log obfuscate success";
    }

    // obfuscate
    MemberObfuscator memberObfuscator(memberNameMapping);
    GUARD_ASSERT(!memberObfuscator.Execute(fileView), ErrorCode::GENERIC_ERROR, "member obfuscate execute failed");
    LOG_I << "member obfuscate execute success";

    ElementObfuscator elementObfuscator(elementNameMapping);
    GUARD_ASSERT(!elementObfuscator.Execute(fileView), ErrorCode::GENERIC_ERROR,
                     "element obfuscate execute failed");
    LOG_I << "element obfuscate execute success";

    // dump name cache
    NameCacheDumper nameCacheDumper(fileView, config_.GetPrintNameCache(), config_.GetDefaultNameCachePath());
    nameCacheDumper.Dump();
    LOG_I << "name cache dump success";

    return true;
}
