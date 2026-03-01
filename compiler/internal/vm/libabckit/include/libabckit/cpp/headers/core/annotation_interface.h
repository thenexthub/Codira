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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_H
#define CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_H

#include "../base_classes.h"
#include "annotation_interface_field.h"
#include "../config.h"

#include <vector>
#include <string>

namespace abckit::core {

/**
 * @brief AnnotationInterface
 */
class AnnotationInterface : public ViewInResource<AbckitCoreAnnotationInterface *, const File *> {
    /// @brief core::Annotation
    friend class core::Annotation;
    /// @brief arkts::AnnotationInterface
    friend class arkts::Module;
    /// @brief core::Module
    friend class core::Module;
    /// @brief core::Namespace
    friend class core::Namespace;
    /// @brief core::AnnotationInterfaceField
    friend class core::AnnotationInterfaceField;
    /// @brief abckit::DefaultHash<AnnotationInterface>
    friend class abckit::DefaultHash<AnnotationInterface>;

protected:
    /// @brief Core API View type
    using CoreViewT = AnnotationInterface;

public:
    /**
     * @brief Construct a new Annotation Interface object
     * @param other
     */
    AnnotationInterface(const AnnotationInterface &other) = default;  // CC-OFF(G.CLS.07): design decision

    /**
     * @brief Constructor
     * @param other
     * @return AnnotationInterface&
     */
    AnnotationInterface &operator=(const AnnotationInterface &other) = default;

    /**
     * @brief Construct a new Annotation Interface object
     * @param other
     */
    AnnotationInterface(AnnotationInterface &&other) = default;  // CC-OFF(G.CLS.07): design decision

    /**
     * @brief Constructor
     * @param other
     * @return AnnotationInterface&
     */
    AnnotationInterface &operator=(AnnotationInterface &&other) = default;

    /**
     * @brief Destroy the Annotation Interface object
     */
    ~AnnotationInterface() override = default;

    /**
     * @brief Returns binary file that the given Annotation Interface is a part of.
     * @return Pointer to the `File`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    const File *GetFile() const;

    /**
     * @brief Returns name for annotation interface.
     * @return `std::string`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Allocates
     */
    std::string GetName() const;

    /**
     * @brief Get vector with Fields
     * @return std::vector<AnnotationInterfaceField>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<AnnotationInterfaceField> GetFields() const;

    /**
     * @brief Returns module for annotation interface.
     * @return `core::Module`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.

     */
    core::Module GetModule() const;

    /**
     * @brief Returns parent namespace for AnnotationInterface.
     * @return `core::Namespace`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Namespace GetParentNamespace() const;

    /**
     * @brief Enumerates fields of Annotation Interface, invoking callback `cb` for each field.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is false.
     */
    bool EnumerateFields(const std::function<bool(core::AnnotationInterfaceField)> &cb) const;

private:
    template <typename AnnotationPayload>
    inline bool GetFieldsInner(AnnotationPayload annPayload)
    {
        auto isNormalExit = GetApiConfig()->cIapi_->annotationInterfaceEnumerateFields(
            GetView(), &annPayload, [](AbckitCoreAnnotationInterfaceField *func, void *data) {
                const auto &payload = *static_cast<AnnotationPayload *>(data);
                payload.vec->push_back(core::AnnotationInterfaceField(func, payload.config, payload.file));
                return true;
            });
        return isNormalExit;
    }

    AnnotationInterface(AbckitCoreAnnotationInterface *anni, const ApiConfig *conf, const File *file)
        : ViewInResource(anni), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;

protected:
    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_H
