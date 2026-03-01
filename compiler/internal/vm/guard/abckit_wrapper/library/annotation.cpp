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
#include "abckit_wrapper/annotation.h"
#include "logger.h"

std::string abckit_wrapper::Annotation::GetName() const
{
    return this->GetObjectName();
}

bool abckit_wrapper::Annotation::SetName(const std::string &name)
{
    return this->SetObjectName<abckit::arkts::Annotation>(name);
}

bool abckit_wrapper::Annotation::IsExternal() const
{
    return this->impl_.IsExternal();
}

bool abckit_wrapper::Annotation::Accept(AnnotationVisitor &visitor)
{
    return visitor.Visit(this);
}

AbckitWrapperErrorCode abckit_wrapper::AnnotationTarget::InitAnnotations()
{
    for (const auto &coreAnnotation : this->GetAnnotations()) {
        auto annotation = std::make_unique<Annotation>(coreAnnotation);
        // init annotation's owner
        this->InitAnnotation(annotation.get());
        const auto rc = annotation->Init();
        if (ABCKIT_WRAPPER_ERROR(rc)) {
            LOG_E << "Failed to init Annotation:" << coreAnnotation.GetName() << rc;
            return rc;
        }
        LOG_I << "Found Annotation:" << annotation->GetFullyQualifiedName();
        this->annotations_.emplace(annotation->GetFullyQualifiedName(), std::move(annotation));
    }

    return ERR_SUCCESS;
}

bool abckit_wrapper::AnnotationTarget::AnnotationsAccept(AnnotationVisitor &visitor)
{
    for (const auto &[_, annotation] : this->annotations_) {
        if (!annotation->Accept(visitor)) {
            return false;
        }
    }

    return true;
}

std::vector<std::string> abckit_wrapper::AnnotationTarget::GetAnnotationNames() const
{
    std::vector<std::string> annotationNames;
    annotationNames.reserve(this->annotations_.size());
    for (const auto &[_, annotation] : this->annotations_) {
        annotationNames.emplace_back(annotation->GetName());
    }
    return annotationNames;
}
