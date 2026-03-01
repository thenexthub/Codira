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

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_H

#include <functional>
#include <vector>

#include "Codira/AST/ASTContext.h"
#include "Codira/AST/Node.h"
#include "Codira/AST/Walker.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "common/CODELintCompilerInstance.h"
#include "Codira/Utils/Utils.h"
#include "common/ConfigContext.h"
#include "common/DiagnosticEngine.h"
#include "rules/Rule.h"

namespace Codira::CodeCheck {
class StructuralRule : public Rule {
public:
    explicit StructuralRule(CodeCheckDiagnosticEngine *diagEngine) : Rule(diagEngine) {};
    void DoAnalysis(CODELintCompilerInstance *instance) override;

protected:
    virtual void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) = 0;
    template <typename... Args> void Diagnose(const Position pos, CodeCheckDiagKind kind, const Args... args)
    {
        diagEngine->Diagnose(pos, kind, args...);
    }
    template <typename... Args> void Diagnose(const Position start, const Position end, CodeCheckDiagKind kind,
        const Args... args)
    {
        diagEngine->Diagnose(start, end, kind, args...);
    }
    template <typename... Args>
    void Diagnose(const std::string filePath, int line, int column, CodeCheckDiagKind kind, const Args... args)
    {
        Codira::Position pos;
        pos.fileID = static_cast<unsigned int>(diagEngine->GetSourceManager().GetFileID(filePath));
        pos.line = line;
        pos.column = column;
        diagEngine->Diagnose(pos, kind, args...);
    }
    template <typename... Args>
    void Diagnose(const std::string filePath, int line, int column, int endLine, int endColumn, CodeCheckDiagKind kind,
        const Args... args)
    {
        Codira::Position start, end;
        start.fileID = static_cast<unsigned int>(diagEngine->GetSourceManager().GetFileID(filePath));
        start.line = line;
        start.column = column;
        end.fileID = static_cast<unsigned int>(diagEngine->GetSourceManager().GetFileID(filePath));
        end.line = endLine;
        end.column = endColumn;
        diagEngine->Diagnose(start, end, kind, args...);
    }
};
} // namespace Codira::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_H
