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

#include "Checker.h"
#include <string>
#include <thread>
#include <vector>
#include "common/CommonFunc.h"
#include "common/ConfigContext.h"
#include "rules/dataflow_rule_analysis/dataflow_rule_G_CHK_01/DataflowRuleGCHK01Check.h"
#include "rules/dataflow_rule_analysis/dataflow_rule_G_FIO_01/DataflowRuleGFIO01Check.h"
#include "rules/dataflow_rule_analysis/dataflow_rule_G_OTH_01/DataflowRuleGOTH01Check.h"
#include "rules/dataflow_rule_analysis/dataflow_rule_G_SER_03/DataflowRuleGSER03Checker.h"
#include "rules/dataflow_rule_analysis/dataflow_rule_G_VAR_01/DataflowRuleVAR01Check.h"
#include "rules/dataflow_rule_analysis/dataflow_rule_G_CHK_03/DataflowRuleGCHK03Check.h"
#include "rules/dataflow_rule_analysis/dataflow_rule_P_02/DataflowRuleP02Check.h"
#include "rules/structural_rule_analysis/StructuralRuleGCHK02/StructuralRuleGCHK02.h"
#include "rules/structural_rule_analysis/StructuralRuleGCHK04/StructuralRuleGCHK04.h"
#include "rules/structural_rule_analysis/StructuralRuleGCLS01/StructuralRuleGCLS01.h"
#include "rules/structural_rule_analysis/StructuralRuleGCON01/StructuralRuleGCON01.h"
#include "rules/structural_rule_analysis/StructuralRuleGCON02/StructuralRuleGCON02.h"
#include "rules/structural_rule_analysis/StructuralRuleGCON03/StructuralRuleGCON03.h"
#include "rules/structural_rule_analysis/StructuralRuleGDCL01/StructuralRuleGDCL01.h"
#include "rules/structural_rule_analysis/StructuralRuleGDCL02/StructuralRuleGDCL02.h"
#include "rules/structural_rule_analysis/StructuralRuleGENU01/StructuralRuleGENU01.h"
#include "rules/structural_rule_analysis/StructuralRuleGENU02/StructuralRuleGENU02.h"
#include "rules/structural_rule_analysis/StructuralRuleGERR01/StructuralRuleGERR01.h"
#include "rules/structural_rule_analysis/StructuralRuleGERR02/StructuralRuleGERR02.h"
#include "rules/structural_rule_analysis/StructuralRuleGERR03/StructuralRuleGERR03.h"
#include "rules/structural_rule_analysis/StructuralRuleGERR04/StructuralRuleGERR04.h"
#include "rules/structural_rule_analysis/StructuralRuleGEXP01/StructuralRuleGEXP01.h"
#include "rules/structural_rule_analysis/StructuralRuleGEXP02/StructuralRuleGEXP02.h"
#include "rules/structural_rule_analysis/StructuralRuleGEXP03/StructuralRuleGEXP03.h"
#include "rules/structural_rule_analysis/StructuralRuleGEXP04/StructuralRuleGEXP04.h"
#include "rules/structural_rule_analysis/StructuralRuleGEXP05/StructuralRuleGEXP05.h"
#include "rules/structural_rule_analysis/StructuralRuleGEXP06/StructuralRuleGEXP06.h"
#include "rules/structural_rule_analysis/StructuralRuleGEXP07/StructuralRuleGEXP07.h"
#include "rules/structural_rule_analysis/StructuralRuleGFMT01/StructuralRuleGFMT01.h"
#include "rules/structural_rule_analysis/StructuralRuleGFMT13/StructuralRuleGFMT13.h"
#include "rules/structural_rule_analysis/StructuralRuleGFMT15/StructuralRuleGFMT15.h"
#include "rules/structural_rule_analysis/StructuralRuleGFUN01/StructuralRuleGFUN01.h"
#include "rules/structural_rule_analysis/StructuralRuleGFUN02/StructuralRuleGFUN02.h"
#include "rules/structural_rule_analysis/StructuralRuleGFUNC03/StructuralRuleGFUNC03.h"
#include "rules/structural_rule_analysis/StructuralRuleGITF01/StructuralRuleGITF01.h"
#include "rules/structural_rule_analysis/StructuralRuleGITF02/StructuralRuleGITF02.h"
#include "rules/structural_rule_analysis/StructuralRuleGITF03/StructuralRuleGITF03.h"
#include "rules/structural_rule_analysis/StructuralRuleGITF04/StructuralRuleGITF04.h"
#include "rules/structural_rule_analysis/StructuralRuleGNAM01/StructuralRuleGNAM01.h"
#include "rules/structural_rule_analysis/StructuralRuleGNAM02/StructuralRuleGNAM02.h"
#include "rules/structural_rule_analysis/StructuralRuleGNAM03/StructuralRuleGNAM03.h"
#include "rules/structural_rule_analysis/StructuralRuleGNAM04/StructuralRuleGNAM04.h"
#include "rules/structural_rule_analysis/StructuralRuleGNAM05/StructuralRuleGNAM05.h"
#include "rules/structural_rule_analysis/StructuralRuleGNAM06/StructuralRuleGNAM06.h"
#include "rules/structural_rule_analysis/StructuralRuleGOPR01/StructuralRuleGOPR01.h"
#include "rules/structural_rule_analysis/StructuralRuleGOPR02/StructuralRuleGOPR02.h"
#include "rules/structural_rule_analysis/StructuralRuleGOTH02/StructuralRuleGOTH02.h"
#include "rules/structural_rule_analysis/StructuralRuleGOTH03/StructuralRuleGOTH03.h"
#include "rules/structural_rule_analysis/StructuralRuleGOTH04/StructuralRuleGOTH04.h"
#include "rules/structural_rule_analysis/StructuralRuleGPKG01/StructuralRuleGPKG01.h"
#include "rules/structural_rule_analysis/StructuralRuleGSEC01/StructuralRuleGSEC01.h"
#include "rules/structural_rule_analysis/StructuralRuleGSER01/StructuralRuleGSER01.h"
#include "rules/structural_rule_analysis/StructuralRuleGSER02/StructuralRuleGSER02.h"
#include "rules/structural_rule_analysis/StructuralRuleGSYN01/StructuralRuleGSYN01.h"
#include "rules/structural_rule_analysis/StructuralRuleGSYN01/StructuralRuleGSYN0101.h"
#include "rules/structural_rule_analysis/StructuralRuleGTYP03/StructuralRuleGTYP03.h"
#include "rules/structural_rule_analysis/StructuralRuleGVAR02/StructuralRuleGVAR02.h"
#include "rules/structural_rule_analysis/StructuralRuleGVAR03/StructuralRuleGVAR03.h"
#include "rules/structural_rule_analysis/StructuralRuleP01/StructuralRuleP01.h"
#include "rules/structural_rule_analysis/StructuralRuleFFIC7/StructuralRuleFFIC7.h"
#include "rules/structural_rule_analysis/StructuralRuleP03/StructuralRuleP03.h"

using namespace Codira;
using namespace Codira::CodeCheck;
using Json = nlohmann::json;

namespace {
#ifdef _WIN32
const std::string LIB_EXTENSION = ".dll";
#else
const std::string LIB_EXTENSION = ".so";
#endif
const std::string CACHED_EXTENSION = "_Cache";
const std::string CODELINT_RULE_LIST = "/config/codelint_rule_list.json";
static const std::vector<std::string> g_fileExtension{
    ".codeo", ".bc", ".cachedast", ".iast", LIB_EXTENSION, ".o", ".bchir", ".cbc", ".pdba"};
static const std::map<std::string, std::string> ruleConfig = {{"G.OTH.01", "/config/dataflow_rule_G_OTH_01.json"},
    {"G.CHK.02", "/config/structural_rule_G_CHK_02.json"}, {"G.CHK.04", "/config/structural_rule_G_CHK_04.json"},
    {"G.ERR.02", "/config/structural_rule_G_ERR_02.json"}, {"G.OTH.02", "/config/structural_rule_G_OTH_02.json"},
    {"G.OTH.04", "/config/structural_rule_G_OTH_04.json"}, {"G.SEC.01", "/config/structural_rule_G_SEC_01.json"},
    {"G.SER.01", "/config/structural_rule_G_SER_01.json"}, {"G.SYN.01", "/config/structural_rule_G_SYN_01.json"}};
} // namespace

Checker::Checker(CodeCheckDiagnosticEngine* diagEngine) : diagEngine(diagEngine), rulesMap(RulesMap())
{
    ConfigContext& configContext = ConfigContext::GetInstance();
    fileDir = configContext.GetSrcFileDir()[0];
    modulesDir = configContext.GetModulesDir();
    RegisterRule<DataflowRuleGCHK01Check>("G.CHK.01", CompileStage::CHIR);
    RegisterRule<DataflowRuleGFIO01Check>("G.FIO.01", CompileStage::CHIR);
    RegisterRule<DataflowRuleGOTH01Check>("G.OTH.01", CompileStage::CHIR);
    RegisterRule<DataflowRuleGSER03Checker>("G.SER.03", CompileStage::CHIR);
    RegisterRule<DataflowRuleP02Check>("P.02", CompileStage::CHIR);
    RegisterRule<DataflowRuleVAR01Check>("G.VAR.01", CompileStage::CHIR);
    RegisterRule<DataflowRuleGCHK03Check>("G.CHK.03", CompileStage::CHIR);
    RegisterRule<StructuralRuleGENU01>("G.ENU.01", CompileStage::SEMA);
    RegisterRule<StructuralRuleGEXP01>("G.EXP.01", CompileStage::SEMA);
    RegisterRule<StructuralRuleGEXP02>("G.EXP.02", CompileStage::SEMA);
    RegisterRule<StructuralRuleGENU02>("G.ENU.02", CompileStage::SEMA);
    RegisterRule<StructuralRuleGEXP03>("G.EXP.03", CompileStage::SEMA);
    RegisterRule<StructuralRuleGDCL01>("G.DCL.01", CompileStage::SEMA);
    RegisterRule<StructuralRuleGDCL02>("G.DCL.02", CompileStage::PARSE);
    RegisterRule<StructuralRuleGEXP04>("G.EXP.04", CompileStage::SEMA);
    RegisterRule<StructuralRuleGFMT01>("G.FMT.01", CompileStage::SEMA);
    RegisterRule<StructuralRuleGFMT13>("G.FMT.13", CompileStage::SEMA);
    RegisterRule<StructuralRuleGFMT15>("G.FMT.15", CompileStage::SEMA);
    RegisterRule<StructuralRuleGOTH02>("G.OTH.02", CompileStage::SEMA);
    RegisterRule<StructuralRuleGOTH03>("G.OTH.03", CompileStage::SEMA);
    RegisterRule<StructuralRuleGOTH04>("G.OTH.04", CompileStage::SEMA);
    RegisterRule<StructuralRuleGCON01>("G.CON.01", CompileStage::SEMA);
    RegisterRule<StructuralRuleGCON03>("G.CON.03", CompileStage::SEMA);
    RegisterRule<StructuralRuleGFUN01>("G.FUN.01", CompileStage::SEMA);
    RegisterRule<StructuralRuleGFUN02>("G.FUN.02", CompileStage::SEMA);
    RegisterRule<StructuralRuleGFUNC03>("G.FUN.03", CompileStage::SEMA);
    RegisterRule<StructuralRuleGNAM01>("G.NAM.01", CompileStage::SEMA);
    RegisterRule<StructuralRuleGNAM02>("G.NAM.02", CompileStage::SEMA);
    RegisterRule<StructuralRuleGNAM03>("G.NAM.03", CompileStage::SEMA);
    RegisterRule<StructuralRuleGNAM04>("G.NAM.04", CompileStage::SEMA);
    RegisterRule<StructuralRuleGNAM05>("G.NAM.05", CompileStage::SEMA);
    RegisterRule<StructuralRuleGNAM06>("G.NAM.06", CompileStage::SEMA);
    RegisterRule<StructuralRuleGSEC01>("G.SEC.01", CompileStage::SEMA);
    RegisterRule<StructuralRuleGSER01>("G.SER.01", CompileStage::SEMA);
    RegisterRule<StructuralRuleGSER02>("G.SER.02", CompileStage::SEMA);
    RegisterRule<StructuralRuleGCHK02>("G.CHK.02", CompileStage::SEMA);
    RegisterRule<StructuralRuleGCHK04>("G.CHK.04", CompileStage::SEMA);
    RegisterRule<StructuralRuleGERR01>("G.ERR.01", CompileStage::SEMA);
    RegisterRule<StructuralRuleGERR02>("G.ERR.02", CompileStage::SEMA);
    RegisterRule<StructuralRuleGERR04>("G.ERR.04", CompileStage::SEMA);
    RegisterRule<StructuralRuleGCON02>("G.CON.02", CompileStage::SEMA);
    RegisterRule<StructuralRuleP01>("P.01", CompileStage::SEMA);
    RegisterRule<StructuralRuleP03>("P.03", CompileStage::SEMA);
    RegisterRule<StructuralRuleGVAR02>("G.VAR.02", CompileStage::SEMA);
    RegisterRule<StructuralRuleGVAR03>("G.VAR.03", CompileStage::SEMA);
    RegisterRule<StructuralRuleGPKG01>("G.PKG.01", CompileStage::SEMA);
    RegisterRule<StructuralRuleGERR03>("G.ERR.03", CompileStage::SEMA);
    RegisterRule<StructuralRuleGEXP05>("G.EXP.05", CompileStage::SEMA);
    RegisterRule<StructuralRuleGEXP06>("G.EXP.06", CompileStage::SEMA);
    RegisterRule<StructuralRuleGEXP07>("G.EXP.07", CompileStage::SEMA);
    RegisterRule<StructuralRuleGOPR01>("G.OPR.01", CompileStage::SEMA);
    RegisterRule<StructuralRuleGOPR02>("G.OPR.02", CompileStage::SEMA);
    RegisterRule<StructuralRuleGITF01>("G.ITF.01", CompileStage::SEMA);
    RegisterRule<StructuralRuleGITF02>("G.ITF.02", CompileStage::SEMA);
    RegisterRule<StructuralRuleGCLS01>("G.CLS.01", CompileStage::SEMA);
    RegisterRule<StructuralRuleGITF03>("G.ITF.03", CompileStage::SEMA);
    RegisterRule<StructuralRuleGITF04>("G.ITF.04", CompileStage::SEMA);
    RegisterRule<StructuralRuleGSYN01>("G.SYN.01", CompileStage::PARSE);
    RegisterRule<StructuralRuleGSYN0101>("G.SYN.01", CompileStage::SEMA);
    RegisterRule<StructuralRuleGTYP03>("G.TYP.03", CompileStage::SEMA);
    RegisterRule<StructuralRuleFFIC7>("FFI.C.7", CompileStage::SEMA);
}

void Checker::CreateRuleThreads(Json jsonInfo, const std::unique_ptr<CODELintCompilerInstance>& compilerInstance,
    CompileStage stage, int& readJsonCode)
{
    std::vector<std::thread> threads;
    if (!jsonInfo.contains("RuleList")) {
        readJsonCode = JSON_READ_ERR;
        return;
    }

    std::set<std::string> checkedLst;
    for (const auto& item : jsonInfo["RuleList"]) {
        std::string upperStr = item.get<std::string>();
        (void)std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), toupper);
        auto res = checkedLst.insert(upperStr);
        if (!res.second) {
            continue;
        }
        auto rules = rulesMap[stage];
        if (rules.count(upperStr) == 0) {
            continue;
        }
        if (ruleConfig.count(upperStr) == 0) {
            threads.emplace_back(&DoAnalysis, rules[upperStr].get(), compilerInstance.get());
            continue;
        }
        Json ruleJsonInfo;
        if (CommonFunc::ReadJsonFileToJsonInfo(ruleConfig.find(upperStr)->second,
            ConfigContext::GetInstance(), ruleJsonInfo) == ERR) {
            readJsonCode = JSON_READ_ERR;
        } else {
            threads.emplace_back(&DoAnalysis, rules[upperStr].get(), compilerInstance.get());
        }
    }
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

static void RemoveExtensionFileAndDir(std::set<std::string>& macroPath)
{
    for (auto& filePath : macroPath) {
        auto pos = filePath.rfind(PATH_SEPARATOR, filePath.length());
        auto dir = filePath.substr(0, pos + 1);
        auto pkgName = filePath.substr(pos + 1, filePath.length());
        auto dirPath = dir + "." + pkgName + CACHED_EXTENSION;
        auto macroLib = dir + PATH_SEPARATOR + "lib-macro_" + pkgName + LIB_EXTENSION;
        if (FileUtil::FileExist(macroLib)) {
            (void)FileUtil::Remove(macroLib);
        }
        for (auto& extension : g_fileExtension) {
            std::string file = filePath + extension;
            std::string fileInDir = dirPath + PATH_SEPARATOR + pkgName + extension;
            if (FileUtil::FileExist(file)) {
                (void)FileUtil::Remove(file);
            }
            if (FileUtil::FileExist(fileInDir)) {
                (void)FileUtil::Remove(fileInDir);
            }
        }
        (void)FileUtil::Remove(dirPath);
    }
}

int Checker::CheckCode()
{
    DiagnosticEngine diag;
#ifdef NDEBUG
    diag.SetIsEmitter(false);
    diag.SetIsDumpErrCnt(false);
#endif
    Json jsonInfo;
    int readJsonCode = OK;
    bool isSuccess = true;
    if (CommonFunc::ReadJsonFileToJsonInfo(CODELINT_RULE_LIST, ConfigContext::GetInstance(), jsonInfo) == ERR) {
        CODELintCompilerInvocation::GetInstance().DesInitRuntime();
        return ERR;
    }
    auto compilerInstance = CODELintCompilerInvocation::GetInstance().PrePareCompilerInstance(diag);
    diagEngine->SetSourceManager(&compilerInstance->GetSourceManager());

#ifndef CODIRA_ENABLE_GCOV
    try {
#endif
        isSuccess = compilerInstance->Compile(CompileStage::PARSE);
        CreateRuleThreads(jsonInfo, compilerInstance, CompileStage::PARSE, readJsonCode);

        isSuccess = isSuccess && compilerInstance->Compile(CompileStage::SEMA);
        CreateRuleThreads(jsonInfo, compilerInstance, CompileStage::SEMA, readJsonCode);

        isSuccess = isSuccess && compilerInstance->Compile(CompileStage::CHIR);
        if (compilerInstance->diag.GetErrorCount() == 0) {
            CreateRuleThreads(jsonInfo, compilerInstance, CompileStage::CHIR, readJsonCode);
        }
#ifndef CODIRA_ENABLE_GCOV
    } catch (const std::out_of_range& e) {
        Errorln("Out of range exception caught: ", e.what());
    } catch (const std::exception& e) {
        Errorln("Exception caught: ", e.what());
    } catch (...) {
        Errorln("Unknown exception caught.");
    }
#endif

    CODELintCompilerInvocation::GetInstance().DesInitRuntime();
    RemoveExtensionFileAndDir(compilerInstance->macroPath);
    return readJsonCode;
}
