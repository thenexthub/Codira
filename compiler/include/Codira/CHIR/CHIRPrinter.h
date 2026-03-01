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
 * This file declares the CHIRPrinter class in CHIR.
 */

#ifndef CODIRA_CHIR_CHIRPRINTER_H
#define CODIRA_CHIR_CHIRPRINTER_H

#include "Codira/CHIR/CHIR.h"

#include <iostream>

namespace Codira::CHIR {
class Type;
class Expression;
class Func;
class If;
class Loop;
class ForIn;
class Value;
class Block;
class BlockGroup;
class Package;

class CHIRPrinter {
public:
    static void PrintCFG(const Func& func, const std::string& path);
    static void PrintPackage(const Package& package, std::ostream& os = std::cout);
    static void PrintPackage(const Package& package, const std::string& fullPath);
    static void PrintCHIRSerializeInfo(ToCHIR::Phase phase, const std::string& path);

public:
    CHIRPrinter(std::ostream& os) : os(os)
    {
    }
    ~CHIRPrinter() = default;
    /*
     * @brief Returns the output stream of the printer.
     *
     */
    std::ostream& GetStream() const
    {
        return os;
    }
private:
    /** @brief The output stream for the printer.*/
    std::ostream& os;
};
} // namespace Codira::CHIR
#endif // CODIRA_CHIR_CHIRPRINTER_H
