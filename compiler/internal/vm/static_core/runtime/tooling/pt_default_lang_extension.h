/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_TOOLING_INSPECTOR_PT_DEFAULT_LANG_EXTENSION_H
#define PANDA_RUNTIME_TOOLING_INSPECTOR_PT_DEFAULT_LANG_EXTENSION_H

#include "runtime/include/tooling/pt_lang_extension.h"

namespace ark::tooling {
class PtStaticDefaultExtension : public PtLangExt {
public:
    explicit PtStaticDefaultExtension(panda_file::SourceLang lang) : lang_(lang) {}
    ~PtStaticDefaultExtension() override = default;

    NO_COPY_SEMANTIC(PtStaticDefaultExtension);
    NO_MOVE_SEMANTIC(PtStaticDefaultExtension);

    std::string GetClassName(const ObjectHeader *object) override;
    std::optional<std::string> GetAsString(const ObjectHeader *object) override;
    std::optional<size_t> GetLengthIfArray(const ObjectHeader *object) override;
    void EnumerateProperties(const ObjectHeader *object, const PropertyHandler &handler) override;
    void EnumerateGlobals(const PropertyHandler &handler) override;

private:
    panda_file::SourceLang lang_;
};

class PtDynamicDefaultExtension : public PtLangExt {
public:
    PtDynamicDefaultExtension() = default;
    ~PtDynamicDefaultExtension() override = default;

    NO_COPY_SEMANTIC(PtDynamicDefaultExtension);
    NO_MOVE_SEMANTIC(PtDynamicDefaultExtension);

    std::string GetClassName(const ObjectHeader *object) override;
    std::optional<std::string> GetAsString(const ObjectHeader *object) override;
    std::optional<size_t> GetLengthIfArray(const ObjectHeader *object) override;
    void EnumerateProperties(const ObjectHeader *object, const PropertyHandler &handler) override;
    void EnumerateGlobals([[maybe_unused]] const PropertyHandler &handler) override {}
};
}  // namespace ark::tooling

#endif  // PANDA_RUNTIME_TOOLING_INSPECTOR_PT_DEFAULT_LANG_EXTENSION_H
