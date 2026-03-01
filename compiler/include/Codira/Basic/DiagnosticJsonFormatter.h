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
 * This file declares the Diagnostic Json Formatter class, which format diagnostic info to json string.
 */

#ifndef CODIRA_DIAGNOSTICODESONFORMATTER_H
#define CODIRA_DIAGNOSTICODESONFORMATTER_H


#include <vector>
#include <string>
#include <list>
#include "Codira/Basic/DiagnosticEngine.h"

namespace Codira {
/// Print diagnostic as json. Used when --diagnostic-format=json enabled.
class DiagnosticJsonFormatter {
public:
    explicit DiagnosticJsonFormatter(DiagnosticEngine& diag) : diag(diag)
    {
    }
    static std::string AssembleDiagnosticJsonString(
        const std::list<std::string>& diagsJsonBuff, const std::string& numJsonBuff);
    std::string FormatDiagnosticToJson(const Diagnostic& d, size_t deep = DIAG_JSON_DEEP);
    std::string FormatDiagnosticCountToJsonString();

    DiagnosticJsonFormatter(const DiagnosticJsonFormatter&) = delete;
    DiagnosticJsonFormatter& operator=(const DiagnosticJsonFormatter&) = delete;
    DiagnosticJsonFormatter(const DiagnosticJsonFormatter&&) = delete;
    DiagnosticJsonFormatter& operator=(const DiagnosticJsonFormatter&&) = delete;
private:
    static constexpr size_t DIAG_JSON_DEEP = 3;
    DiagnosticEngine& diag;
    inline static std::string Whitespace(size_t num)
    {
        return std::string(num, ' ');
    }
    static std::string AssembleDiagnosticsJsonString(const std::list<std::string>& diagsJsonBuff);
    std::string FormatDiagnosticMainContentJson(size_t deep, const Diagnostic& d);
    std::string FormatDiagnosticNotesJson(size_t deep, const std::vector<SubDiagnostic>& subDiags);
    std::string FormatDiagnosticHelpsJson(size_t deep, const std::vector<DiagHelp>& helps);
    std::string FormatDiagnosticSubDiagJson(size_t deep, const SubDiagnostic& subDiag);
    std::string FormatDiagnosticDiagHelpJson(size_t deep, const DiagHelp& help, bool newLine = true);
    std::string FormatHintJson(size_t deep, const IntegratedString& hint, bool newLine = true);
    std::string FormatSubstitutionJson(size_t deep, const Substitution& substitution);
    std::string FormatHintsJson(size_t deep, const std::vector<IntegratedString>& hints);
    std::string FormatRangeJson(size_t deep, const Range& range);
    std::string FormatPostionJson(size_t deep, const Position& pos);
};

} // namespace Codira
#endif // CODIRA_DIAGNOSTICODESONFORMATTER_H
