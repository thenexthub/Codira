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
 * Implements the generator for llvm reflection info.
 */

#include "CODENative/CODENativeReflectionInfo.h"

#include "CODENative/CODENativeMetadata.h"

using namespace Codira;
using namespace CodeGen;

void CODENativeReflectionInfo::Gen() const
{
    // Generate metadata for packages, classes, interfaces, structs, enums, global functions, and global variables.
    CGMetadata(cgMod, subCHIRPkg)
        .Needs(MetadataKind::PKG_METADATA)
        ->Needs(MetadataKind::CLASS_METADATA)
        ->Needs(MetadataKind::STRUCT_METADATA)
        ->Needs(MetadataKind::ENUM_METADATA)
        ->Needs(MetadataKind::GF_METADATA)
        ->Needs(MetadataKind::GV_METADATA)
        ->Gen();
}
