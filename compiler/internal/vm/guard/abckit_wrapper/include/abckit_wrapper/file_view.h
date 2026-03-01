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

#ifndef ABCKIT_WRAPPER_FILE_VIEW_H
#define ABCKIT_WRAPPER_FILE_VIEW_H

#include "module.h"
#include "object.h"

namespace abckit_wrapper {
/**
 * @brief FileView
 * abc file view
 */
class FileView {
public:
    /**
     * @brief FileView Constructor
     */
    explicit FileView() = default;

    /**
     * @brief Init fileView with abc file path
     * @param path abc file path
     * @return AbckitWrapperErrorCode
     */
    AbckitWrapperErrorCode Init(const std::string_view &path);

    /**
     * @brief  Get module by name
     * @param moduleName module name
     * @return module ptr
     */
    std::optional<Module *> GetModule(const std::string &moduleName) const;

    /**
     * @brief Get object by name
     * @param moduleName module name
     * @return object ptr
     */
    std::optional<Object *> GetObject(const std::string &moduleName) const;

    /**
     * Get object ptr by full name
     * @tparam T object type
     * @param objectName object full name
     * @return object ptr
     */
    template <typename T>
    std::optional<T *> Get(const std::string &objectName) const
    {
        const auto object = objectPool_.Get(objectName);
        if (!object.has_value()) {
            return std::nullopt;
        }

        const auto module = object.value()->owningModule_;
        if (!module.has_value()) {
            return std::nullopt;
        }

        return module.value()->Get<T>(objectName);
    }

    /**
     * @brief All modules accept visit
     * @param visitor ModuleVisitor
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool ModulesAccept(ModuleVisitor &visitor);

    /**
     * @brief Clear object's visit data
     */
    void ClearObjectsData();

    void Save(const std::string_view &path) const;

private:
    /**
     * abckit::File ptr
     */
    std::unique_ptr<abckit::File> file_ = nullptr;
    /**
     * store all the modules of the file
     */
    std::unordered_map<std::string, Module *> moduleTable_;
    /**
     * store all objects unique ptr of the file
     */
    ObjectPool objectPool_;
};
}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_FILE_VIEW_H
