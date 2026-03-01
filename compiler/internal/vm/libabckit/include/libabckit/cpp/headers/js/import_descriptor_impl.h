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

#ifndef CPP_ABCKIT_JS_IMPORT_DESCRIPTOR_IMPL_H
#define CPP_ABCKIT_JS_IMPORT_DESCRIPTOR_IMPL_H

#include "./import_descriptor.h"
#include "../core/import_descriptor.h"

namespace abckit::js {

inline AbckitJsImportDescriptor *ImportDescriptor::TargetCast() const
{
    auto ret = GetApiConfig()->cJsIapi_->coreImportDescriptorToJsImportDescriptor(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline ImportDescriptor::ImportDescriptor(const core::ImportDescriptor &other)
    : core::ImportDescriptor(other), targetChecker_(this)
{
}

}  // namespace abckit::js

#endif  // CPP_ABCKIT_JS_IMPORT_DESCRIPTOR_IMPL_H
