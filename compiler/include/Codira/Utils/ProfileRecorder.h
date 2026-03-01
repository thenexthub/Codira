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
 * This file defines the ProfileRecorder class for performance analysis.
 */

#ifndef CODIRA_UTILS_PROFILE_RECORDER_H
#define CODIRA_UTILS_PROFILE_RECORDER_H

#include <string>
#include <functional>

namespace Codira::Utils {

class ProfileRecorder {
public:
    enum class Type {
        INVALID_TYPE = 0x00,
        TIMER        = 0x01,
        MEMORY       = 0x02,
        ALL          = 0x03  // TIMER | MEMORY
    };
    ProfileRecorder(
        const std::string& title, const std::string& subtitle, const std::string& desc = "");
    ~ProfileRecorder();

    static void SetPackageName(const std::string& name);
    static void SetOutputDir(const std::string& path);
    static void Enable(bool en, const Type& type = Type::ALL);

    static void Start(
        const std::string& title, const std::string& subtitle, const std::string& desc = "");
    static void Stop(
        const std::string& title, const std::string& subtitle, const std::string& desc = "");
    /**
     * @brief Record some code info. Avoid introducing other module-specific content by custom function.
     * @param item Indicates the name of a single information item.
     * @param getData Indicates the closure function for obtaining the value of the item.
     */
    static void RecordCodeInfo(const std::string& item, const std::function<int64_t(void)>& getData);
    static void RecordCodeInfo(const std::string& item, int64_t value);

    static std::string GetResult(const Type& type = Type::ALL);

private:
    std::string title_;
    std::string subtitle_;
    std::string desc_;
};
} // namespace Codira::Utils

#endif // CODIRA_UTILS_PROFILE_RECORDER_H
