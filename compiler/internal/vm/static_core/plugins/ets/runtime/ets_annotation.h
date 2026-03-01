/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_ANNOTATION_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_ANNOTATION_H_

#include "libarkfile/file.h"

namespace ark {
class Method;
}  // namespace ark

namespace ark::ets {

class EtsAnnotation {
public:
    static panda_file::File::EntityId FindAsyncAnnotation(const Method *method);
    static panda_file::File::EntityId OptionalParameters(const Method *method);

private:
    static panda_file::File::EntityId FindAnnotation(const Method *method, const std::string_view &sign);
};

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_ETS_ANNOTATION_H_
