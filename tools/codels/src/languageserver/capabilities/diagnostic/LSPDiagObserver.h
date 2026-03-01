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

#ifndef LSPSERVER_DIAGNOSTIC_H
#define LSPSERVER_DIAGNOSTIC_H

#include <iostream>
#include <functional>
#include "Codira/Basic/DiagnosticEngine.h"
#include "../../common/Callbacks.h"
#include "../../common/PositionResolver.h"

namespace ark {
enum class NeedDiagnostics {
    YES,  /// Diagnostics must be generated for this snapshot.
    NO,   /// Diagnostics must not be generated for this snapshot.
    AUTO, /// Diagnostics must be generated for this snapshot or a subsequent one
};

enum class DiagLSPSeverity {
    ERROR_DIAG = 1,
    WARNING = 2,
    INFO = 3,
    HINT = 4,
    DEFAULT_DIAG = 0
};

class LSPDiagObserver : public Codira::DiagnosticHandler {
public:
    explicit LSPDiagObserver(Callbacks *c, Codira::DiagnosticEngine &engine);
    void HandleDiagnose(Codira::Diagnostic& diagnostic) override;
    ~LSPDiagObserver() override;
private:
    Callbacks *callback = nullptr;
    void FormatDiags(DiagnosticToken &diagToken, SubDiagnostic &subDiag, std::set<char> endPunctuation);
    void AddNoteInfo(Codira::Diagnostic &diagnostic, std::vector<DiagnosticRelatedInformation> &relatedInformation);
    void DealMacroDiags(Codira::Diagnostic &diagnostic, const DiagnosticToken &token);
    void CollectQuickFix(Codira::Diagnostic &diagnostic, DiagnosticToken &diagToken);
};
} // namespace ark

#endif // LSPSERVER_DIAGNOSTIC_H
