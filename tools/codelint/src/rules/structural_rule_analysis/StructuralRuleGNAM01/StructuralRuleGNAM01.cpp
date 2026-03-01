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

#include "StructuralRuleGNAM01.h"

namespace {
std::string RemoveBackticks(const std::string& name)
{
    const size_t lenOfBackticks = 2;
    if (name.size() > lenOfBackticks && name.front() == '`' && name.back() == '`') {
        return name.substr(1, name.size() - lenOfBackticks);
    }
    return name;
}

std::string GetFullPackageName(const Codira::AST::PackageSpec& pkg)
{
    if (pkg.prefixPaths.empty()) {
        return pkg.packageName;
    }
    auto prefix = Codira::Utils::JoinStrings(pkg.prefixPaths, ".");
    return prefix + "." + pkg.packageName;
}
} // namespace

namespace Codira::CodeCheck {
using namespace Codira;
using namespace AST;
using namespace Meta;

const std::string REGEX = "^[a-z]+[a-z0-9_]*(\\.[a-z][a-z0-9_]*)*$";

/*
 * This method is used to check whether the package name complies with the regular expression.
 */
void StructuralRuleGNAM01::FileDeclHandler(const File &file)
{
    if (!file.package) {
        return;
    }
    const auto& package = *file.package;
    std::regex reg = std::regex(REGEX);
    if (!(std::regex_match(package.packageName.Val(), reg))) {
        Diagnose(package.packageName.Begin(), package.packageName.Begin(),
            CodeCheckDiagKind::G_NAM_01_Package_Information, GetFullPackageName(package));
    }
    // Root packages can have any valid package name.
    if (package.prefixPaths.empty()) {
        return;
    }
    auto filePath = FileUtil::GetDirPath(file.filePath);
    auto lastSlashPos = filePath.rfind(PATH_SEPARATOR);
    auto curDir = lastSlashPos != std::string::npos ? filePath.substr(lastSlashPos + 1) : "";
    if (RemoveBackticks(package.packageName) != curDir) {
        Diagnose(package.packageName.Begin(), package.packageName.End(),
            CodeCheckDiagKind::G_NAM_01_Package_name_should_match_path, GetFullPackageName(package));
    }
}

/*
 * Obtain the file node and obtain the package name.
 */
void StructuralRuleGNAM01::FilePackageCheckingFunction(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const File &file) {
                FileDeclHandler(file);
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGNAM01::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    FilePackageCheckingFunction(node);
}
} // namespace Codira::CodeCheck
