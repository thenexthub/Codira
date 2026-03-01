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

#ifndef GUARD_CONFIGURATION_KEEP_OPTION_PARSER_H
#define GUARD_CONFIGURATION_KEEP_OPTION_PARSER_H

#include <optional>
#include <unordered_set>

#include "class_specification.h"
#include "word_reader.h"

namespace ark::guard {

/**
 * @brief KeepOptionParser
 * This parser is used to parse the keep option configuration.
 */
class KeepOptionParser {
public:
    /**
     * @brief constructor, initializes the keepOptionStr
     * @param keepOptionStr keep option configuration string
     */
    explicit KeepOptionParser(const std::string &keepOptionStr);

    /**
     * @brief parse keep option string and return a ClassSpecification object.
     * Current support keep option:
     *  -keep class_specification
     *  -keepclassmembers class_specification
     *  -keepclasswithmembers class_specification
     * @return ClassSpecification object
     */
    std::optional<ClassSpecification> Parse();

private:
    ClassSpecification ParseClassSpecificationArguments(bool keepClassName);

    void GetClassAnnotationNameAndAccessFlags(std::string &annotationName, uint32_t &setAccessFlags,
                                              uint32_t &unSetAccessFlags);

    void GetClassTypeDeclarations(uint32_t &setTypeDeclarations, uint32_t &unSetTypeDeclarations) const;

    void GetExtensionInfo(uint32_t &extensionType, std::string &extensionAnnotationName,
                          std::string &extensionClassName);

    void ParseMemberSpecificationArguments(ClassSpecification &specification);

    void GetMemberAnnotationNameAndAccessFlags(std::string &annotationName, uint32_t &setAccessFlags,
                                               uint32_t &unSetAccessFlags);

    bool HandleWildcardMemberSpecification(const std::string &annotationName, uint32_t setAccessFlags,
                                           uint32_t unSetAccessFlags, ClassSpecification &specification);

    bool HandleFieldSpecification(const std::string &annotationName, uint32_t setAccessFlags, uint32_t unSetAccessFlags,
                                  const std::string &memberName, ClassSpecification &specification);

    bool HandleMethodSpecification(const std::string &annotationName, uint32_t setAccessFlags,
                                   uint32_t unSetAccessFlags, const std::string &memberName,
                                   ClassSpecification &specification);

    std::string ParseCommaSeparatedList(bool skipCurrentWord);

    std::string GetMethodArguments();

    std::string GetMemberType(const std::unordered_set<std::string_view> &delimiters);

    WordReader wordReader_;

    std::string nextWord_;
};
}  // namespace ark::guard

#endif  // GUARD_CONFIGURATION_KEEP_OPTION_PARSER_H
