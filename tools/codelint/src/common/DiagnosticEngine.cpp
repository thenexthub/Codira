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

#include "common/DiagnosticEngine.h"
#include "common/CommonFunc.h"

#include <fstream>
#include <string>
#ifdef _WIN32
#include <windows.h>
#include "Codira/Basic/StringConvertor.h"
#endif

#include "Codira/Utils/FileUtil.h"

namespace Codira::CodeCheck {
std::atomic<unsigned> CodeCheckDiagnosticEngine::errorCount = 0;
std::atomic<unsigned> CodeCheckDiagnosticEngine::warningCount = 0;

const unsigned FORMAT_LEN = 2;
const int JSON_INDENT = 4;

CodeCheckDiagCategory CodeCheckDiagnostic::GetDiagnoseCategory(CodeCheckDiagKind diagKind)
{
    if (diagKind > CodeCheckDiagKind::mandatory_diag_begin && diagKind <= CodeCheckDiagKind::mandatory_diag_end) {
        return CodeCheckDiagCategory::MANDATORY;
    }
    return CodeCheckDiagCategory::SUGGESTIONS;
}

/**
    String buggyFilePath: 必选, 文件路径
    String bugId：不填，平台自动生成
    String defectLevel：必选，级别
    String defectType：规则
    String analyzerName：工具名
    String description：缺陷描述
    int mainBuggyLine：代码行
    String mainBuggyCode：代码行对应的代码，选填，若不填，平台会自动填充
    List<FixInfo> fixCode：可选，自动修复使用
    String mergeKey：不填，平台自动自动生成
    String extra：选填，扩展字段
    String ruleUrl：??
    String functionName: 所在函数名称
    String engine: ??
    String category: ??
    String subcategory: ??
    String issueType: ??
    String cwe: ??
    List<EventInfo> events: ??
    String language: 选填, 语言
*/
nlohmann::ordered_json CodeCheckDiagnostic::ToJsonValue() const
{
    nlohmann::ordered_json jsonObj;
#ifdef _WIN32
    auto pathName = Codira::StringConvertor::NormalizeStringToUTF8(path);
    jsonObj["file"] = pathName.has_value() ? pathName.value() : path;
#else
    jsonObj["file"] = path;
#endif
    jsonObj["line"] = start.line;
    jsonObj["column"] = start.column;
    jsonObj["endLine"] = end.line;
    jsonObj["endColumn"] = end.column;
    jsonObj["analyzerName"] = "cangjieCodeCheck";
#ifdef _WIN32
    auto message = Codira::StringConvertor::NormalizeStringToUTF8(diagMessage);
    jsonObj["description"] = message.has_value() ? message.value() : diagMessage;
#else
    jsonObj["description"] = diagMessage;
#endif
    jsonObj["defectLevel"] = CODE_CHECK_CLEARANCE_REQUIREMENT_NAMES[static_cast<int>(diagCategory)];
    jsonObj["defectType"] = CODE_CHECK_DIAG_NAMES[static_cast<int>(kind)];
    jsonObj["language"] ="cangjie";
    return jsonObj;
}

std::string CodeCheckDiagnostic::ToCsvValue() const
{
    std::string csvValue;
    (void)csvValue.append(path).append(",");
    (void)csvValue.append(std::to_string(start.line)).append(",");
    (void)csvValue.append(std::to_string(start.column)).append(",");
    (void)csvValue.append(std::to_string(end.line)).append(",");
    (void)csvValue.append(std::to_string(end.column)).append(",");
    (void)csvValue.append("\"").append(diagMessage).append("\"").append(",");
    (void)csvValue.append(CODE_CHECK_DIAG_NAMES[static_cast<int>(kind)]).append(",");
    (void)csvValue.append(CODE_CHECK_CLEARANCE_REQUIREMENT_NAMES[static_cast<int>(diagCategory)]).append("\n");
    return csvValue;
}

CodeCheckDiagnosticEngine::~CodeCheckDiagnosticEngine()
{
#ifndef CODIRA_ENABLE_GCOV
    try {
#endif
        if (!reportFile.empty()) {
            DiagnosticToFile();
        } else {
            for (auto& diag : diagnosticList) {
                DiagnosticPrint(diag);
            }
        }
#ifndef CODIRA_ENABLE_GCOV
    } catch (...) {
        abort();
    }
#endif
}

std::string CodeCheckDiagnosticEngine::ConvertDiagListToJsonString()
{
    nlohmann::ordered_json jsonArray = nlohmann::ordered_json::array();
    for (auto it = diagnosticList.begin(); it != diagnosticList.end(); ++it) {
        jsonArray.emplace_back(it->ToJsonValue());
    }
    std::string res;
#ifndef CODIRA_ENABLE_GCOV
    try {
#endif
        res = jsonArray.dump(JSON_INDENT,  ' ', false, nlohmann::json::error_handler_t::ignore);
#ifndef CODIRA_ENABLE_GCOV
    } catch (const std::exception& e) {
        std::cerr << "Failed to generate json file: " << e.what() << std::endl;
    }
#endif
    return res;
}

std::string CodeCheckDiagnosticEngine::ConvertDiagListToCsvString()
{
    std::string value = "SourceFile,Line,Column,Description,DefectType,DefectLevel\n";
    for (auto &i : diagnosticList) {
        (void)value.append(i.ToCsvValue());
    }
    return value;
}

void CodeCheckDiagnosticEngine::DiagnosticToFile()
{
    if (!reportToFile) {
        Errorln("no set report path");
        return;
    }
    size_t reportFileLength = reportFile.length();
    // 1 is the size of "."
    if (reportFileLength + format.size() + 1 > reportFile.max_size()) {
        Errorln("The report file name is too long: %s", reportFile);
        return;
    }
    std::string targetName = CommonFunc::HasEnding(reportFile, format) ? reportFile : reportFile + '.' + format;
    if (targetName.empty()) {
        Errorln("Create target file %s error!", targetName.c_str());
        return;
    }
    std::ofstream outStream(targetName, std::ios_base::app);
    if (!outStream.is_open()) {
        Errorln("Create target file %s error!", targetName.c_str());
        return;
    }
    auto source = format == "csv" ? ConvertDiagListToCsvString() : ConvertDiagListToJsonString();
    if (source.size() > static_cast<size_t>(std::numeric_limits<long>::max())) {
        // 检查source.size是否在long的范围内，防止后续类型转换后产生溢出
        Errorln("The size of the source is too large to write to the file.");
        return;
    }
    (void)outStream.write(source.data(), static_cast<long>(source.size()));
    outStream.close();
}

void CodeCheckDiagnosticEngine::DiagnosticPrint(const CodeCheckDiagnostic &diag)
{
    switch (diag.clearanceRequirement) {
        case CodeCheckClearanceRequirement::MANDATORY: {
            std::cerr << diag.path << ":" << diag.start.line << ":" << diag.start.column << ":" << " ";
#ifdef _WIN32
            CONSOLE_SCREEN_BUFFER_INFO colorInfo;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &colorInfo);
            auto oldColorInfo = colorInfo.wAttributes;
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
            std::cerr << "error";
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), oldColorInfo);
            std::cerr << ": ";
#else
            std::cerr << RED_ERROR_MARK;
#endif
            std::cerr << diag.diagMessage << std::endl;
            break;
        }
        case CodeCheckClearanceRequirement::SUGGESTIONS: {
            std::cerr << diag.path << ":" << diag.start.line << ":" << diag.start.column << ":" << " ";
#ifdef _WIN32
            CONSOLE_SCREEN_BUFFER_INFO colorInfo;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &colorInfo);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
            std::cerr << "warning";
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorInfo.wAttributes);
            std::cerr << ": ";
#else
            std::cerr << YELLOW_WARNING_MARK;
#endif
            std::cerr << diag.diagMessage << std::endl;
            break;
        }
    }
}

void CodeCheckDiagnosticEngine::PushToDiagnosticList(CodeCheckDiagnostic &diag)
{
    lock.lock();
    if (diag.start.IsZero()) {
        diagnosticList.emplace_front(diag);
        lock.unlock();
        return;
    }

    auto it = diagnosticList.begin();
    for (; it != diagnosticList.end(); ++it) {
        if (it->start.fileID < diag.start.fileID) {
            continue;
        } else if (it->start.fileID > diag.start.fileID) {
            (void)diagnosticList.insert(it, diag);
            break;
        } else if (it->start.line < diag.start.line) {
            continue;
        } else if (it->start.line > diag.start.line) {
            (void)diagnosticList.insert(it, diag);
            break;
        } else if (it->start.column < diag.start.column) {
            continue;
        } else if (it->start.column > diag.start.column) {
            (void)diagnosticList.insert(it, diag);
            break;
        }
        if ((it->start.fileID == diag.start.fileID) && (it->start.line == diag.start.line) &&
            (it->start.column == diag.start.column) && (it->diagMessage == diag.diagMessage)) {
            break;
        }
        (void)diagnosticList.insert(it, diag);
        break;
    }
    if (it == diagnosticList.end()) {
        diagnosticList.emplace_back(diag);
    }
    lock.unlock();
}

void CodeCheckDiagnosticEngine::ConvertToDiagMessage(CodeCheckDiagnostic &diagnostic) noexcept
{
    std::string formatStr = CODE_CHECK_DIAG_MESSAGES[static_cast<int>(diagnostic.kind)];
    uint8_t index = 0;
    auto formatPos = formatStr.find('%');
    auto &formatArgs = diagnostic.args;
    while (formatPos != std::string::npos) {
        if (index >= formatArgs.size()) {
            break;
        }
        std::string argStr;
        switch (formatStr[formatPos + 1]) {
            case 'd':
                if (auto val = std::get_if<int>(&formatArgs[index].arg)) {
                    argStr = std::to_string(*val);
                } else {
                    Errorln("The num ", index, "format parameter does not match");
                }
                break;
            case 's':
                if (auto val = std::get_if<std::string>(&formatArgs[index].arg)) {
                    argStr = *val;
                } else {
                    Errorln("The num ", index, "format parameter does not match");
                }
                break;
            case 'c':
                if (auto val = std::get_if<char>(&formatArgs[index].arg); val) {
                    argStr = std::string(1, *val);
                } else {
                    Errorln("The num ", index, "format parameter does not match");
                }
                break;
            case 'p':
                if (auto val = std::get_if<Position>(&formatArgs[index].arg); val) {
                    argStr = sm->GetSource((*val).fileID).path + ":" + std::to_string((*val).line) + ":" +
                        std::to_string((*val).column);
                } else {
                    Errorln("The num ", index, "format parameter does not match");
                }
                break;
            default:
                Errorln("%", formatStr[formatPos + 1], " is illegal format");
                break;
        }
        if (!argStr.empty()) {
            (void)formatStr.replace(formatPos, FORMAT_LEN, argStr);
            formatPos += argStr.size();
        }
        ++index;
        formatPos = formatStr.find('%', formatPos);
    }
    diagnostic.diagMessage = formatStr;
}
} // namespace Codira::CodeCheck
