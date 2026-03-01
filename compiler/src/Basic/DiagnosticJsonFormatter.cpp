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
 * This file implements the DiagnosticEmitter related classes.
 */

#include "Codira/Basic/DiagnosticJsonFormatter.h"
#include "Codira/Basic/StringConvertor.h"
namespace Codira {

namespace {
static const std::unordered_map<DiagCategory, std::string> CATE_TO_STR{
    {DiagCategory::LEX, "lex"},
    {DiagCategory::PARSE, "parse"},
    {DiagCategory::PARSE_QUERY, "parse_query"},
    {DiagCategory::CONDITIONAL_COMPILATION, "conditional_compilation"},
    {DiagCategory::IMPORT_PACKAGE, "import_package"},
    {DiagCategory::MODULE, "module"},
    {DiagCategory::MACRO_EXPAND, "macro_expand"},
    {DiagCategory::SEMA, "sema"},
    {DiagCategory::CHIR, "chir"},
    {DiagCategory::OTHER, "other"},
};
constexpr std::size_t INDENT_WIDTH = 4;

} // namespace

std::string DiagnosticJsonFormatter::FormatPostionJson(size_t deep, const Position& pos)
{
    std::string diagsJsonStr = Whitespace(deep * INDENT_WIDTH) + "\"File\": \"";
    std::string fileStr = std::to_string(pos.fileID);
    if (diag.HasSourceManager()) {
        if (diag.GetSourceManager().IsSourceFileExist(pos.fileID)) {
            auto source = diag.GetSourceManager().GetSource(pos.fileID);
            if (source.packageName.has_value()) {
                fileStr = "(package " + source.packageName.value() + ")" + FileUtil::GetFileName(source.path);
            } else {
                fileStr = source.path;
            }
        }
    }
    diagsJsonStr += StringConvertor::EscapeToJsonString(fileStr) + "\",\n";
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"Line\": " + std::to_string(pos.line) + ",\n";
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"Column\": " + std::to_string(pos.column);
    return diagsJsonStr;
}

std::string DiagnosticJsonFormatter::FormatRangeJson(size_t deep, const Range& range)
{
    std::string diagsJsonStr = Whitespace(deep * INDENT_WIDTH) + "\"Range\": {\n";
    ++deep;
    // begin
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"Begin\": {\n";
    diagsJsonStr += FormatPostionJson(deep + 1, range.begin) + "\n";
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "},\n";
    // end
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"End\": {\n";
    diagsJsonStr += FormatPostionJson(deep + 1, range.end) + "\n";
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "}\n";
    diagsJsonStr += Whitespace((deep - 1) * INDENT_WIDTH) + "}";
    return diagsJsonStr;
}

std::string DiagnosticJsonFormatter::FormatHintJson(size_t deep, const IntegratedString& hint, bool newLine)
{
    std::string diagsJsonStr = Whitespace(deep * INDENT_WIDTH) + "{\n";
    if (!newLine) {
        diagsJsonStr = "{\n";
    }
    ++deep;
    diagsJsonStr +=
        Whitespace(deep * INDENT_WIDTH) + "\"Content\": \"" + StringConvertor::EscapeToJsonString(hint.str) + "\",\n";
    diagsJsonStr += FormatRangeJson(deep, hint.range) + "\n";
    --deep;
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "}";
    return diagsJsonStr;
}

std::string DiagnosticJsonFormatter::FormatHintsJson(size_t deep, const std::vector<IntegratedString>& hints)
{
    if (hints.empty()) {
        return {"[]"};
    }
    std::string diagsJsonStr{"[\n"};
    for (size_t i = 0; i < hints.size(); ++i) {
        diagsJsonStr += FormatHintJson(deep + 1, hints[i]);
        diagsJsonStr += (i == hints.size() - 1) ? "\n" : ",\n";
    }
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "]";
    return diagsJsonStr;
}

std::string DiagnosticJsonFormatter::FormatSubstitutionJson(size_t deep, const Substitution& substitution)
{
    std::string diagsJsonStr = Whitespace(deep * INDENT_WIDTH) + "{\n";
    ++deep;
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"Content\": \"" +
        StringConvertor::EscapeToJsonString(substitution.str) + "\",\n";
    diagsJsonStr += FormatRangeJson(deep, substitution.range) + "\n";
    --deep;
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "}";
    return diagsJsonStr;
}

std::string DiagnosticJsonFormatter::FormatDiagnosticMainContentJson(size_t deep, const Diagnostic& d)
{
    // Messsage
    std::string diagsJsonStr = Whitespace(deep * INDENT_WIDTH) + "\"Message\": \"" +
        StringConvertor::EscapeToJsonString(d.errorMessage) + "\",\n";
    // Location
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"Location\": {\n";
    diagsJsonStr += FormatPostionJson(deep + 1, d.mainHint.range.begin) + "\n";
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "},\n";
    // main hint
    if (d.mainHint.IsDefault()) {
        diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"MainHint\": null,\n";
    } else {
        diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"MainHint\": ";
        diagsJsonStr += FormatHintJson(deep, d.mainHint, false) + ",\n";
    }
    // other hints
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"OtherHints\": " + FormatHintsJson(deep, d.otherHints);
    return diagsJsonStr;
}

std::string DiagnosticJsonFormatter::FormatDiagnosticDiagHelpJson(size_t deep, const DiagHelp& help, bool newLine)
{
    std::string diagsJsonStr = Whitespace(deep * INDENT_WIDTH) + "{\n";
    if (!newLine) {
        diagsJsonStr = "{\n";
    }
    ++deep;
    // Messsage
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"Message\": \"" +
        StringConvertor::EscapeToJsonString(help.helpMes) + "\",\n";
    // Substitutions
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"Substitutions\": [";
    if (help.substitutions.empty()) {
        diagsJsonStr += "]";
    } else {
        diagsJsonStr += "\n";
        ++deep;
        for (size_t i = 0; i < help.substitutions.size(); ++i) {
            diagsJsonStr += FormatSubstitutionJson(deep, help.substitutions[i]);
            diagsJsonStr += (i == help.substitutions.size() - 1) ? "\n" : ",\n";
        }
        --deep;
        diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "]";
    }
    --deep;
    diagsJsonStr += "\n" + Whitespace(deep * INDENT_WIDTH) + "}";
    return diagsJsonStr;
}

std::string DiagnosticJsonFormatter::FormatDiagnosticSubDiagJson(size_t deep, const SubDiagnostic& subDiag)
{
    std::string diagsJsonStr = Whitespace(deep * INDENT_WIDTH) + "{\n";
    ++deep;
    // Messsage
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"Message\": \"" +
        StringConvertor::EscapeToJsonString(subDiag.subDiagMessage) + "\",\n";
    // MainHint
    if (subDiag.mainHint.IsDefault()) {
        diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"MainHint\": null,\n";
    } else {
        diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"MainHint\": ";
        diagsJsonStr += FormatHintJson(deep, subDiag.mainHint, false) + ",\n";
    }
    // other hints
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"OtherHints\": " + FormatHintsJson(deep, subDiag.otherHints);
    // help
    diagsJsonStr += ",\n";
    if (subDiag.help.IsDefault()) {
        diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"Help\": null";
    } else {
        diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"Help\": ";
        diagsJsonStr += FormatDiagnosticDiagHelpJson(deep, subDiag.help, false);
    }
    --deep;
    diagsJsonStr += "\n" + Whitespace(deep * INDENT_WIDTH) + "}";
    return diagsJsonStr;
}

std::string DiagnosticJsonFormatter::FormatDiagnosticNotesJson(size_t deep, const std::vector<SubDiagnostic>& subDiags)
{
    std::string diagsJsonStr = Whitespace(deep * INDENT_WIDTH) + "\"Notes\": [";
    if (subDiags.empty()) {
        diagsJsonStr += "]";
        return diagsJsonStr;
    }
    diagsJsonStr += "\n";
    for (size_t i = 0; i < subDiags.size(); ++i) {
        diagsJsonStr += FormatDiagnosticSubDiagJson(deep + 1, subDiags[i]);
        diagsJsonStr += (i == subDiags.size() - 1) ? "\n" : ",\n";
    }
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "]";
    return diagsJsonStr;
}

std::string DiagnosticJsonFormatter::FormatDiagnosticHelpsJson(size_t deep, const std::vector<DiagHelp>& helps)
{
    std::string diagsJsonStr = Whitespace(deep * INDENT_WIDTH) + "\"Helps\": [";
    if (helps.empty()) {
        diagsJsonStr += "]";
        return diagsJsonStr;
    }
    diagsJsonStr += "\n";
    ++deep;
    for (size_t i = 0; i < helps.size(); ++i) {
        diagsJsonStr += FormatDiagnosticDiagHelpJson(deep, helps[i]);
        diagsJsonStr += (i == helps.size() - 1) ? "\n" : ",\n";
    }
    diagsJsonStr += Whitespace((deep - 1) * INDENT_WIDTH) + "]";
    return diagsJsonStr;
}

std::string DiagnosticJsonFormatter::FormatDiagnosticToJson(const Diagnostic& d, size_t deep)
{
    std::string diagsJsonStr = Whitespace(deep * INDENT_WIDTH) + "{\n";
    ++deep;
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"DiagKind\": \"";
    if (d.isRefactor && !d.isConvertedToRefactor) {
        if (static_cast<size_t>(d.rKind) < RE_DIAG_KIND_STR_SIZE) {
            diagsJsonStr += StringConvertor::EscapeToJsonString(RE_DIAG_KIND_STR[static_cast<size_t>(d.rKind)]);
        }
    } else {
        if ((static_cast<size_t>(d.kind) < DIAG_KIND_STR_SIZE)) {
            diagsJsonStr += StringConvertor::EscapeToJsonString(DIAG_KIND_STR[static_cast<size_t>(d.kind)]);
        }
    }

    diagsJsonStr += "\",\n" + Whitespace(deep * INDENT_WIDTH) + "\"Severity\": \"";
    CODEC_ASSERT(d.diagSeverity == DiagSeverity::DS_ERROR || d.diagSeverity == DiagSeverity::DS_WARNING);
    if (d.diagSeverity == DiagSeverity::DS_ERROR) {
        diagsJsonStr += "error";
    } else {
        diagsJsonStr += "warning";
    }

    diagsJsonStr += "\",\n" + Whitespace(deep * INDENT_WIDTH) + "\"WarnGroup\": \"";
    if (d.warnGroup == WarnGroup::NONE) {
        diagsJsonStr += "none";
    } else if (d.warnGroup == WarnGroup::UNGROUPED) {
        diagsJsonStr += "ungrouped";
    } else {
        CODEC_ASSERT(static_cast<unsigned>(d.warnGroup) < WARN_GROUP_DESCRS_SIZE);
        diagsJsonStr += StringConvertor::EscapeToJsonString(warnGroupDescrs[static_cast<unsigned>(d.warnGroup)]);
    }

    diagsJsonStr += "\",\n" + Whitespace(deep * INDENT_WIDTH) + "\"DiagCategory\": \"";
    CODEC_ASSERT(CATE_TO_STR.find(d.diagCategory) != CATE_TO_STR.end());
    if (CATE_TO_STR.find(d.diagCategory) != CATE_TO_STR.end()) {
        diagsJsonStr += StringConvertor::EscapeToJsonString(CATE_TO_STR.at(d.diagCategory)) + "\"";
    } else {
        diagsJsonStr += "other\"";
    }
    // Message, Location, MainHint, OtherHints
    diagsJsonStr += ",\n";
    diagsJsonStr += FormatDiagnosticMainContentJson(deep, d);
    // Notes
    diagsJsonStr += ",\n";
    diagsJsonStr += FormatDiagnosticNotesJson(deep, d.subDiags);
    // Helps
    diagsJsonStr += ",\n";
    diagsJsonStr += FormatDiagnosticHelpsJson(deep, d.helps);
    // end
    --deep;
    diagsJsonStr += "\n" + Whitespace(deep * INDENT_WIDTH) + "}";
    return diagsJsonStr;
}

std::string DiagnosticJsonFormatter::AssembleDiagnosticsJsonString(const std::list<std::string>& diagsJsonBuff)
{
    if (diagsJsonBuff.empty()) {
        return "\"Diags\":[]";
    }
    std::string diagsJsonStr(Whitespace(INDENT_WIDTH));
    diagsJsonStr += "\"Diags\":[";
    for (auto it = diagsJsonBuff.begin(); it != diagsJsonBuff.end();) {
        if (diagsJsonStr.back() == '[') {
            diagsJsonStr += "\n";
        }
        diagsJsonStr += *it;
        ++it;
        diagsJsonStr += (it == diagsJsonBuff.end()) ? "\n" : ",\n";
    }
    diagsJsonStr += Whitespace(INDENT_WIDTH) + "]";
    return diagsJsonStr;
}

std::string DiagnosticJsonFormatter::FormatDiagnosticCountToJsonString()
{
    std::string diagsJsonStr(Whitespace(INDENT_WIDTH));
    diagsJsonStr += "\"Num\":{\n";
    size_t deep{2};
    diagsJsonStr += Whitespace(deep * INDENT_WIDTH) + "\"Errors\": " + std::to_string(diag.GetErrorCount()) + ",\n";
    diagsJsonStr +=
        Whitespace(deep * INDENT_WIDTH) + "\"PrintedErrors\": " + std::to_string(diag.GetErrorPrintCount()) + ",\n";
    diagsJsonStr +=
        Whitespace(deep * INDENT_WIDTH) + "\"Warnings\": " + std::to_string(diag.GetWarningCount()) + ",\n";
    diagsJsonStr +=
        Whitespace(deep * INDENT_WIDTH) + "\"PrintedWarnings\": " + std::to_string(diag.GetWarningPrintCount()) + "\n";
    diagsJsonStr += Whitespace(INDENT_WIDTH) + "}";
    return diagsJsonStr;
}

std::string DiagnosticJsonFormatter::AssembleDiagnosticJsonString(
    const std::list<std::string>& diagsJsonBuff, const std::string& numJsonBuff)
{
    if (diagsJsonBuff.empty()) {
        return {};
    }
    std::string diagsJsonStr{"{\n"};
    diagsJsonStr += AssembleDiagnosticsJsonString(diagsJsonBuff);
    if (numJsonBuff.empty()) {
        diagsJsonStr += "\n}\n";
    } else {
        diagsJsonStr += ",\n" + numJsonBuff + "\n}\n";
    }
    return diagsJsonStr;
}

} // namespace Codira
