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
 * This file declares class for java code generation.
 */
#ifndef CODIRA_SEMA_JAVA_ABSTRACT_GENERATOR
#define CODIRA_SEMA_JAVA_ABSTRACT_GENERATOR

#include <fstream>
#include <functional>

#include "Codira/AST/Types.h"
#include "Codira/Utils/FileUtil.h"

namespace Codira::Interop {
using namespace Codira::AST;

class AbstractSourceCodeGenerator {
public:
    explicit AbstractSourceCodeGenerator(const std::string& outputFilePath);
    AbstractSourceCodeGenerator(const std::string& outputFolderPath, const std::string& outputFileName);
    virtual ~AbstractSourceCodeGenerator() = default;

    void Generate();

protected:
    std::string res;
    static const std::string TAB;
    static const std::string TAB2;

    template <typename Container, typename Element>
    static std::string Join(const Container& container, const std::string& delimiter,
        const std::function<std::string(Element)>& transformer)
    {
        using E = std::decay_t<Element>;
        using ValueType = std::decay_t<decltype(*std::begin(container))>;
        static_assert(
            std::is_same_v<ValueType, E>, "Transformer argument must have the same type as container's element.");

        std::string result = "";
        bool isFirst = true;

        for (const auto& elem : container) {
            if (!isFirst) {
                result += delimiter;
            }
            isFirst = false;
            result += transformer(elem);
        }

        return result;
    }

    virtual void ConstructResult() = 0;
    void AddWithIndent(const std::string& indent, const std::string& s);

private:
    const std::string outputFilePath;
    bool WriteToOutputFile();
};
} // namespace Codira::Interop

#endif // CODIRA_SEMA_JAVA_ABSTRACT_GENERATOR
