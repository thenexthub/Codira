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

#ifndef PANDA_ASSEMBLER_EXTENSIONS_H
#define PANDA_ASSEMBLER_EXTENSIONS_H

#include <memory>
#include <optional>

#include "meta.h"
#include "libarkfile/file_items.h"
#include "libarkbase/macros.h"

namespace ark::pandasm::extensions {

// Workaround for ts2abc. Should be removed by our colleagues.
using Language = ark::panda_file::SourceLang;

class MetadataExtension {
public:
    static PANDA_PUBLIC_API std::unique_ptr<RecordMetadata> CreateRecordMetadata(ark::panda_file::SourceLang lang);

    static PANDA_PUBLIC_API std::unique_ptr<FieldMetadata> CreateFieldMetadata(ark::panda_file::SourceLang lang);

    static PANDA_PUBLIC_API std::unique_ptr<FunctionMetadata> CreateFunctionMetadata(ark::panda_file::SourceLang lang);

    static PANDA_PUBLIC_API std::unique_ptr<ParamMetadata> CreateParamMetadata(ark::panda_file::SourceLang lang);
};

}  // namespace ark::pandasm::extensions

#endif  // PANDA_ASSEMBLER_EXTENSIONS_H
