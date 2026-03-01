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

#ifndef CODIRA_MODULES_ASTSERIALIZATIONTYPEDEF_H
#define CODIRA_MODULES_ASTSERIALIZATIONTYPEDEF_H

#include <flatbuffers/flatbuffers.h>

#include "flatbuffers/CachedASTFormat_generated.h"
#include "flatbuffers/ModuleFormat_generated.h"

namespace Codira {
using FormattedIndex = uint32_t;
using TStringOffset = flatbuffers::Offset<flatbuffers::String>;
using TDeclDepOffset = flatbuffers::Offset<CachedASTFormat::DeclDep>;
using TEffectMapOffset = flatbuffers::Offset<CachedASTFormat::EffectMap>;
using uoffset_t = flatbuffers::uoffset_t;

struct ExportConfig {
    // Whether save initializer of var decl and func body of function.
    bool exportContent{false};
    // Whether write cacahed codeo astData which used for load cached type info in incremental compilation.
    bool exportForIncr{false};
    bool exportForTest{false};
    // Whether save source files with their absolute paths.
    // Used when compiled with the `--coverage` option.
    bool needAbsPath{false};
    bool compileCoded{false};
};
} // namespace Codira

#endif
