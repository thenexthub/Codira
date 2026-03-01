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
 * This file implements the symbol table printing function.
 */

#include "PrintSymbolTable.h"

#include "Codira/AST/Match.h"
#include "Codira/AST/Symbol.h"
#include "Codira/Basic/Print.h"

using namespace Codira;
using namespace AST;

namespace {
constexpr unsigned int ONE_INDENT = 1;
constexpr unsigned int TWO_INDENT = 2;
constexpr unsigned int THREE_INDENT = 3;
constexpr unsigned int FOUR_INDENT = 4;
constexpr unsigned int FIVE_INDENT = 5;

template <typename... Args> inline void PrintIndentNoSplitNoEndl(const unsigned int indent, const Args... args)
{
    PrintIndentOnly(indent);
    PrintNoSplit(args...);
}

template <typename... Args> inline void PrintIndentNoSplit(const unsigned int indent, const Args... args)
{
    PrintIndentNoSplitNoEndl(indent, args...);
    Println();
}

void PrintPackage(const Symbol& symbol)
{
    PrintIndent(TWO_INDENT, "{");
    PrintIndentNoSplit(THREE_INDENT, "\"name\": \"", symbol.name, "\",");
    PrintIndent(THREE_INDENT, "\"files\": [");
    Ptr<Node> node = symbol.node;
    CODEC_NULLPTR_CHECK(node);
    CODEC_ASSERT(node->astKind == ASTKind::PACKAGE);
    Ptr<Package> package = StaticAs<ASTKind::PACKAGE>(node);
    auto iter = package->files.cbegin();
    PrintIndentNoSplitNoEndl(FOUR_INDENT, "\"", (*iter)->filePath, "\"");
    ++iter;
    std::for_each(iter, package->files.cend(), [](auto& file) {
        Println(",");
        PrintIndentNoSplitNoEndl(FOUR_INDENT, "\"", file->filePath, "\"");
    });
    Println();
    PrintIndent(THREE_INDENT, "]");
    PrintIndentNoSplitNoEndl(TWO_INDENT, "}");
}

void PrintPackages(const std::vector<std::reference_wrapper<const Symbol>>& packages)
{
    if (packages.empty()) {
        return;
    }
    std::unordered_set<std::string> packageNames;
    auto iter = packages.cbegin();
    packageNames.emplace((*iter).get().name);
    PrintPackage(*iter);
    ++iter;
    std::for_each(iter, packages.cend(), [&packageNames](const Symbol& sym) {
        if (packageNames.emplace(sym.name).second) {
            Println(",");
            PrintPackage(sym);
        }
    });
    Println();
}

void PrintPosition(unsigned int indent, const Position& pos, const std::string& name, bool hasComma)
{
    PrintIndentNoSplit(indent, "\"", name, "\": {");
    PrintIndentNoSplit(indent + ONE_INDENT, "\"line\": ", pos.line, ",");
    PrintIndent(indent + ONE_INDENT, "\"column\":", pos.column);
    PrintIndentNoSplit(indent, "}", hasComma ? "," : "");
}

void PrintPackageSpecPart(const PackageSpec& packageSpec)
{
    PrintIndentNoSplit(FIVE_INDENT, "\"packageName\": \"", packageSpec.packageName.Val(), "\",");
    PrintPosition(FIVE_INDENT, packageSpec.macroPos, "macroPos", true);
    PrintPosition(FIVE_INDENT, packageSpec.packagePos, "packagePos", true);
    PrintPosition(FIVE_INDENT, packageSpec.packageName.Begin(), "packageNamePos", true);
}

void PrintImportSpecPart(const ImportSpec& importSpec)
{
    PrintPosition(FIVE_INDENT, importSpec.importPos, "importPos", true);
    PrintIndentNoSplit(FIVE_INDENT, "\"packageName\": \"", GetImportedItemFullName(importSpec.content), "\",");
    PrintPosition(FIVE_INDENT, importSpec.content.begin, "packageNamePos", true);
    if (importSpec.IsImportAlias()) {
        PrintPosition(FIVE_INDENT, importSpec.content.asPos, "asPos", true);
        PrintIndentNoSplit(FIVE_INDENT, "\"asIdentifier\": \"", importSpec.content.aliasName.Val(), "\",");
        PrintPosition(FIVE_INDENT, importSpec.content.aliasName.Begin(), "asIdentifierPos", true);
    }
}

void PrintDeclPart(const Decl& decl)
{
    PrintIndentNoSplit(FIVE_INDENT, "\"identifier\": \"", decl.identifier.Val(), "\",");
    PrintPosition(FIVE_INDENT, decl.identifier.Begin(), "identifierPos", true);
}

void PrintNode(const Node& node)
{
    if (node.TestAttr(Attribute::COMPILER_ADD)) {
        return;
    }
    PrintIndent(FOUR_INDENT, "{");
    PrintIndentNoSplit(FIVE_INDENT, "\"astKind\": \"", ASTKIND_TO_STRING_MAP[node.astKind], "\",");
    PrintIndentNoSplit(FIVE_INDENT, "\"name\": \"", node.symbol ? node.symbol->name : "", "\",");
    if (auto packageSpec = DynamicCast<const PackageSpec*>(&node); packageSpec) {
        PrintPackageSpecPart(*packageSpec);
    } else if (auto importSpec = DynamicCast<const ImportSpec*>(&node); importSpec) {
        PrintImportSpecPart(*importSpec);
    } else if (auto decl = DynamicCast<const Decl*>(&node); decl) {
        PrintDeclPart(*decl);
    }
    PrintPosition(FIVE_INDENT, node.begin, "begin", true);
    PrintPosition(FIVE_INDENT, node.end, "end", false);
    PrintIndentNoSplitNoEndl(FOUR_INDENT, "}");
}

void PrintFile(const File& file, const std::vector<std::reference_wrapper<const Node>>& nodes)
{
    PrintIndent(TWO_INDENT, "{");
    PrintIndentNoSplit(THREE_INDENT, "\"path\": \"", file.filePath, "\",");
    PrintIndentNoSplit(THREE_INDENT, "\"symbols\": [");
    if (!nodes.empty()) {
        auto iter = nodes.cbegin();
        PrintNode(*iter);
        ++iter;
        std::for_each(iter, nodes.cend(), [](const Node& node) {
            Println(",");
            PrintNode(node);
        });
        Println();
    }
    PrintIndentNoSplit(THREE_INDENT, "]");
    PrintIndentNoSplitNoEndl(TWO_INDENT, "}");
}

void PrintFiles(
    const std::unordered_map<Ptr<const File>, std::vector<std::reference_wrapper<const Node>>>& file2nodesMap)
{
    if (file2nodesMap.empty()) {
        return;
    }
    auto iter = file2nodesMap.cbegin();
    PrintFile(*(*iter).first, (*iter).second);
    ++iter;
    std::for_each(iter, file2nodesMap.cend(),
        [](const std::pair<Ptr<const File>, std::vector<std::reference_wrapper<const Node>>>& pair) {
            Println(",");
            PrintFile(*pair.first, pair.second);
        });
    Println();
}

void HandleFileNode(std::unordered_map<Ptr<const File>, std::vector<std::reference_wrapper<const Node>>>& file2nodesMap,
    const Symbol& symbol, const Node& node)
{
    if (node.astKind != ASTKind::FILE) {
        return;
    }
    if (&symbol == node.symbol) {
        return;
    }
    const File& file = static_cast<const File&>(node);
    std::vector<std::reference_wrapper<const Node>>& nodes =
        file2nodesMap.emplace(std::piecewise_construct, std::make_tuple(&file), std::make_tuple()).first->second;
    if (file.package) {
        nodes.push_back(*file.package);
    }
    for (auto& import : file.imports) {
        if (import->importPos.line != 0) {
            nodes.push_back(*import);
        }
    }
}

void AddSymbolToInfo(std::vector<std::reference_wrapper<const Symbol>>& packages,
    std::unordered_map<Ptr<const File>, std::vector<std::reference_wrapper<const Node>>>& file2nodesMap,
    std::optional<std::reference_wrapper<File>>& fileOpt, const Symbol& symbol)
{
    CODEC_NULLPTR_CHECK(symbol.node);
    Node& node = *symbol.node;
    if (node.TestAttr(Attribute::COMPILER_ADD)) {
        return;
    }
    if (node.astKind == ASTKind::PACKAGE) {
        packages.push_back(symbol);
    } else {
        if (node.astKind == ASTKind::FILE) {
            fileOpt = static_cast<File&>(node);
        }
        CODEC_ASSERT(fileOpt);
        file2nodesMap.emplace(std::piecewise_construct, std::make_tuple(&fileOpt->get()), std::make_tuple())
            .first->second.push_back(node);
        HandleFileNode(file2nodesMap, symbol, node);
    }
}
} // namespace

void Codira::PrintSymbolTable(CompilerInstance& ci)
{
    std::vector<std::reference_wrapper<const Symbol>> packages;
    std::unordered_map<Ptr<const File>, std::vector<std::reference_wrapper<const Node>>> file2nodesMap;
    for (auto srcPkg : ci.GetSourcePackages()) {
        ASTContext* ctx = ci.GetASTContextByPackage(srcPkg);
        CODEC_NULLPTR_CHECK(ctx);
        std::optional<std::reference_wrapper<File>> fileOpt;
        for (auto& sym : std::as_const(ctx->symbolTable)) {
            AddSymbolToInfo(packages, file2nodesMap, fileOpt, *sym);
        }
    }
    Println("{");
    PrintIndent(ONE_INDENT, "\"packages\": [");
    PrintPackages(packages);
    PrintIndent(ONE_INDENT, "],");
    PrintIndent(ONE_INDENT, "\"files\": [");
    PrintFiles(file2nodesMap);
    PrintIndent(ONE_INDENT, "]");
    Println("}");
}
