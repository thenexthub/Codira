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

#ifndef LIBABCKIT_SRC_ADAPTER_STATIC_ABCKIT_STATIC_H
#define LIBABCKIT_SRC_ADAPTER_STATIC_ABCKIT_STATIC_H

#include "libabckit/c/ir_core.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/src/metadata_inspect_impl.h"

namespace libabckit {

AbckitFile *OpenAbcStatic(const char *path, size_t len);
void WriteAbcStatic(AbckitFile *file, const char *path, size_t len);
void DestroyGraphStatic(AbckitGraph *graph);
void DestroyGraphStaticSync(AbckitGraph *graph);
void CloseFileStatic(AbckitFile *file);

std::unique_ptr<AbckitCoreEnumField> EnumCreateField(AbckitFile *file, AbckitCoreEnum *owner,
                                                     ark::pandasm::Field &recordField);

std::unique_ptr<AbckitCoreClassField> ClassCreateField(AbckitFile *file, AbckitCoreClass *owner,
                                                       ark::pandasm::Field &recordField);

std::unique_ptr<AbckitCoreModuleField> ModuleCreateField(AbckitFile *file, AbckitCoreModule *owner,
                                                         ark::pandasm::Field &recordField);
}  // namespace libabckit

#endif
