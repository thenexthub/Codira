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
 * This file implements the CHIRPrinter class in CHIR.
 */

#include "Codira/CHIR/CHIRPrinter.h"

#include <fstream>

#include "Codira/CHIR/CHIR.h"
#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Type/Type.h"
#include "Codira/CHIR/Value.h"
#include "Codira/CHIR/Visitor/Visitor.h"

using namespace Codira::CHIR;

static void ReplaceAll(std::string& str, const std::string& o, const std::string& n)
{
    std::string::size_type pos = 0;
    while ((pos = str.find(o, pos)) != std::string::npos) {
        str.replace(pos, o.length(), n);
        pos += n.length();
    }
}

void CHIRPrinter::PrintCFG(const Func& func, const std::string& path)
{
    std::fstream fout;
    std::string id = func.GetIdentifierWithoutPrefix();
    ReplaceAll(id, "$", "_");
    ReplaceAll(id, "<", "_");
    ReplaceAll(id, ">", "_");
    std::string filePath = path + id + ".dot";
    fout.open(filePath, std::ios::out);
    if (!fout.is_open()) {
        std::cerr << "open file: " << filePath << " failed!" << std::endl;
        return;
    }
    fout << "digraph " << id << "{" << std::endl;
    fout << "graph [fontname=\"Courier, monospace\"];" << std::endl;
    fout << "node [fontname=\"Courier, monospace\"];" << std::endl;
    fout << "edge [fontname=\"Courier, monospace\"];" << std::endl;
    Visitor::Visit(func, [&fout](Block& block) {
        fout << block.GetIdentifierWithoutPrefix();
        fout << " [shape=none, ";
        fout << "label=<<table border='0' cellborder='1' cellspacing='0'>";
        fout << "<tr><td bgcolor='gray' align='center' colspan='1'>";
        fout << "Block " << block.GetIdentifier() << "</td></tr>";

        for (auto expr : block.GetExpressions()) {
            std::string info = "";
            if (LocalVar* res = expr->GetResult(); res != nullptr) {
                info += res->GetIdentifier() + ": " + res->GetType()->ToString() + " = ";
            }
            info += expr->ToString();
            ReplaceAll(info, "&", "&amp;");
            ReplaceAll(info, "<", "&lt;");
            ReplaceAll(info, ">", "&gt;");
            fout << "<tr><td align='left'>" << info << "</td></tr>";
        }
        fout << "</table>>];" << std::endl;

        for (auto suc : block.GetSuccessors()) {
            fout << block.GetIdentifierWithoutPrefix() << " -> " << suc->GetIdentifierWithoutPrefix() << ";"
                 << std::endl;
        }
        return VisitResult::CONTINUE;
    });
    fout << "}" << std::endl;
    fout.close();
}

void CHIRPrinter::PrintPackage(const Package& package, std::ostream& os)
{
    os << package.ToString() << std::endl;
}

void CHIRPrinter::PrintPackage(const Package& package, const std::string& fullPath)
{
    std::fstream fout;
    fout.open(fullPath, std::ios::out | std::ios::app);
    if (!fout.is_open()) {
        std::cerr << "open file: " << fullPath << " failed!" << std::endl;
        return;
    }
    PrintPackage(package, fout);
    fout.close();
}

void CHIRPrinter::PrintCHIRSerializeInfo(ToCHIR::Phase phase, const std::string& path)
{
    if (path.empty()) {
        Errorln("path empty");
        return;
    }
    auto realDirPath = FileUtil::GetAbsPath(FileUtil::GetDirPath(path));
    if (!realDirPath.has_value()) {
        Errorln("realDirPath false");
        return;
    }
    auto fileNameWithExt = FileUtil::GetFileName(path);
    auto ret = FileUtil::JoinPath(realDirPath.value(), fileNameWithExt);
    std::fstream fout;
    fout.open(ret, std::ofstream::out);
    if (!fout.is_open()) {
        std::cerr << "open file: " << ret << " failed!" << std::endl;
        return;
    }
    std::string phaseStr{};
    switch (phase) {
        case ToCHIR::Phase::RAW:
            phaseStr = "raw";
            break;
        case ToCHIR::Phase::OPT:
            phaseStr = "opt";
            break;
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
        case ToCHIR::Phase::PLUGIN:
            phaseStr = "plugin";
            break;
        case ToCHIR::Phase::ANALYSIS_FOR_CODELINT:
            phaseStr = "analysis for codelint";
            break;
#endif
    }
    fout << "ToCHIRPhase: " << phaseStr << std::endl;
    fout.close();
}
