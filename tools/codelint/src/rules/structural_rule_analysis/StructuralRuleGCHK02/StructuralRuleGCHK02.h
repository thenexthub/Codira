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

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CHK_02_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CHK_02_H

#include "nlohmann/json.hpp"
#include "Codira/AST/Match.h"
#include "Codira/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
/**
 * G.CHK.02 禁止直接使用外部数据记录日志
 */
class StructuralRuleGCHK02 : public StructuralRule {
public:
    explicit StructuralRuleGCHK02(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGCHK02() override = default;
protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override;

private:
    struct LogInfo {
        std::string name;
        Position start;
        Position end;
        std::string scopeName;
        bool isPurified;
        friend bool operator < (const LogInfo &a, const LogInfo &b)
        {
            if (a.start.fileID == b.start.fileID) {
                if (a.start == b.start) {
                    return a.name < b.name;
                }
                return a.start < b.start;
            }
            return a.start.fileID < b.start.fileID;
        }
    };

    struct TaintVarInfo {
        std::string name;
        Position start;
        Position end;
        friend bool operator < (const TaintVarInfo &a, const TaintVarInfo &b)
        {
            if (a.start == b.start) {
                return a.name < b.name;
            }
            return a.start < b.start;
        }
    };

    nlohmann::json jsonInfo;
    std::set<LogInfo> externalDataLogSet;
    std::set<TaintVarInfo> TaintVarSet;
    std::vector<std::string> logFunc = { "trace", "debug", "info", "warn", "error", "log" };
    bool isExternalInterfaceFlag = false;
    static bool IsLogObject(const Codira::AST::MemberAccess &memberAccess);
    bool IsTaintVar(Ptr<const Codira::AST::FuncDecl> funcDecl, const std::string &parentName, const std::string &key);
    void TaintLoggerFinder(Ptr<Codira::AST::Node> node);
    void TaintVarFinder(Ptr<Codira::AST::Node> node);
    void LoggerFinderHelper(Ptr<const Codira::AST::MemberAccess> memberAccess, const Codira::AST::CallExpr &callExpr);
    void ObjectFilter(Ptr<Codira::AST::Expr> expr, const Position start, const Position end,
        const std::string &scopeName, const std::string refName = "");
    bool ExternalDateCheckerHelper(Codira::AST::Decl &decl);
    void AddLogInfoToSet(const Position start, const Position end, const std::string &scopeName,
        const std::string refName);
    void EditLogInfo(const LogInfo &logInfo, bool isPurified);
    void FuncDeclBodyChecker(Ptr<Codira::AST::FuncDecl> pFuncDecl, const std::string &refName, const Position start,
        const Position end, const std::string &scopeName);
    void IsExternalInterface(Ptr<Codira::AST::Expr> expr);
    void UpdateLogInfoByAssignExpr(const Codira::AST::AssignExpr &assignExpr);
    void UpdateLogInfoByMatchExpr(const Codira::AST::MatchExpr &matchExpr, Ptr<const Codira::AST::CallExpr> callExpr);
    void UpdateLogInfoByMatchExprHelper(const Codira::AST::MatchExpr &matchExpr, Ptr<Codira::AST::Expr> expr,
        const LogInfo &logInfo);
    static bool IsRegex(Ptr<const Codira::AST::CallExpr> callExpr);
    void Purifier(Ptr<Codira::AST::Node> node);
};
} // namespace Codira::CodeCheck
#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CHK_02_H
