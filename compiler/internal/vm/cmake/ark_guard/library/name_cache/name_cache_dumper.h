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

#ifndef GUARD_NAME_CACHE_NAME_CACHE_PRINTER_H
#define GUARD_NAME_CACHE_NAME_CACHE_PRINTER_H

#include "abckit_wrapper/file_view.h"

#include "error.h"
#include "name_cache_constants.h"

namespace ark::guard {

/**
 * @brief NameCacheDumper
 */
class NameCacheDumper : public abckit_wrapper::ModuleVisitor, public abckit_wrapper::ChildVisitor {
public:
    /**
     * @brief constructor, initializes fileView, printNameCache and defaultNameCache
     * @param fileView fileView
     * @param printNameCache print name cache path
     * @param defaultNameCache default name cache path
     */
    NameCacheDumper(abckit_wrapper::FileView &fileView, std::string printNameCache, std::string defaultNameCache)
        : fileView_(fileView),
          printNameCache_(std::move(printNameCache)),
          defaultNameCache_(std::move(defaultNameCache))
    {
    }

    /**
     * @brief dump name cache
     * @return ErrorCode ERR_SUCCESS for success otherwise for failed
     */
    ErrorCode Dump();

private:
    bool Visit(abckit_wrapper::Module *module) override;

    bool VisitNamespace(abckit_wrapper::Namespace *ns) override;

    bool VisitMethod(abckit_wrapper::Method *method) override;

    bool VisitField(abckit_wrapper::Field *field) override;

    bool VisitClass(abckit_wrapper::Class *clazz) override;

    bool VisitAnnotationInterface(abckit_wrapper::AnnotationInterface *ai) override;

    std::string BuildJson();

    abckit_wrapper::FileView &fileView_;

    std::string printNameCache_;

    std::string defaultNameCache_;

    ObjectNameCacheTable moduleCaches_;
};
}  // namespace ark::guard

#endif  // GUARD_NAME_CACHE_NAME_CACHE_PRINTER_H
