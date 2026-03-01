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

#ifndef CONST_VALUE_H
#define CONST_VALUE_H

namespace ark {
    static constexpr size_t INSTRUCTION_WIDTH_LIMIT = 80;
    static constexpr size_t INSTRUCTION_FORMAT_WIDTH = 20;
    static constexpr size_t INSTRUCTION_VALUE_WIDTH = 2;
    static constexpr size_t INSTRUCTION_OFFSET_WIDTH = 4;
    static constexpr std::string_view SCOPE_NAME_RECORD = "_ESScopeNamesRecord";
    static constexpr std::string_view TYPE_SUMMARY_FIELD_NAME = "typeSummaryOffset";
    static constexpr std::string_view SCOPE_NAMES = "scopeNames";
    static constexpr std::string_view MODULE_REQUEST_PAHSE_IDX = "moduleRequestPhaseIdx";
}  // namespace ark

#endif  // CONST_VALUE_H
