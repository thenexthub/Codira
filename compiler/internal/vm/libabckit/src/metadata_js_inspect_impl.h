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

#ifndef LIBABCKIT_SRC_METADATA_JS_INSPECT_IMPL_H
#define LIBABCKIT_SRC_METADATA_JS_INSPECT_IMPL_H

#include "libabckit/c/metadata_core.h"

namespace libabckit {

// ========================================
// Module
// ========================================

bool JsModuleEnumerateImports(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreImportDescriptor *i, void *data));
bool JsModuleEnumerateExports(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreExportDescriptor *e, void *data));
bool JsModuleEnumerateClasses(AbckitCoreModule *m, void *data, bool cb(AbckitCoreClass *klass, void *data));
bool JsModuleEnumerateTopLevelFunctions(AbckitCoreModule *m, void *data,
                                        bool (*cb)(AbckitCoreFunction *function, void *data));
bool JsModuleEnumerateAnonymousFunctions(AbckitCoreModule *m, void *data,
                                         bool (*cb)(AbckitCoreFunction *function, void *data));
bool JsModuleEnumerateAnnotationInterfaces(AbckitCoreModule *m, void *data,
                                           bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data));

// ========================================
// Class
// ========================================

bool JsClassEnumerateMethods(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreFunction *method, void *data));

// ========================================
// Function
// ========================================

bool JsFunctionEnumerateNestedFunctions(AbckitCoreFunction *function, void *data,
                                        bool (*cb)(AbckitCoreFunction *nestedFunc, void *data));
bool JsFunctionEnumerateNestedClasses(AbckitCoreFunction *function, void *data,
                                      bool (*cb)(AbckitCoreClass *nestedClass, void *data));

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_METADATA_JS_INSPECT_IMPL_H
