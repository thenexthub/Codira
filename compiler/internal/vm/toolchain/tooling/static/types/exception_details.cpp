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

#include "types/exception_details.h"

#include "libarkbase/utils/json_builder.h"

namespace ark::tooling::inspector {

void ExceptionDetails::Serialize(JsonObjectBuilder &builder) const
{
    builder.AddProperty("exceptionId", exceptionId_);
    builder.AddProperty("text", text_);
    builder.AddProperty("lineNumber", lineNumber_);
    builder.AddProperty("columnNumber", columnNumber_);

    if (scriptId_) {
        builder.AddProperty("scriptId", *scriptId_);
    }
    if (url_) {
        builder.AddProperty("url", *url_);
    }
    if (exception_) {
        builder.AddProperty("exception", *exception_);
    }
    if (executionContextId_) {
        builder.AddProperty("executionContextId", *executionContextId_);
    }
}

}  // namespace ark::tooling::inspector
