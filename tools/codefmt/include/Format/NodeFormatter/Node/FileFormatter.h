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

#ifndef CODEFMT_FILEFORMATTER_H
#define CODEFMT_FILEFORMATTER_H

#include "Format/NodeFormatter/NodeFormatter.h"

namespace Codira::Format {
struct PositionHasher {
public:
    size_t operator()(const Codira::Position& pos) const { return static_cast<size_t>(pos.Hash64()); }
};

class FileFormatter : public NodeFormatter {
public:
    FileFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : NodeFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&) override;

private:
    void SetModifierOrAnnoToPosMap(const Codira::AST::File& file,
        std::unordered_map<Codira::Position, int, PositionHasher>& modifierOrAnnoToPosMap);
    void AddFile(Doc& doc, const Codira::AST::File& file, int level);
};
} // namespace Codira::Format
#endif // CODEFMT_FILEFORMATTER_H
