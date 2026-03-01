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

#ifndef CODIRA_USERBASE_H
#define CODIRA_USERBASE_H

#include <string>

namespace Codira {
class UserBase {
public:
    virtual std::string GetResult() const;
    void OutputResult() const noexcept;

    void WriteJson(const std::string& context, const std::string& suffix) const;

    virtual ~UserBase() = default;

    void Enable(bool en);
    bool IsEnable() const;
    void SetPackageName(const std::string& name);
    void SetOutputDir(const std::string& path);

protected:
    bool enable{false};
    std::string packageName;
    std::string outputDir;

private:
    virtual std::string GetSuffix() const = 0;
    virtual std::string GetJson() const = 0;
};
} // namespace Codira

#endif // CODIRA_USERBASE_H
