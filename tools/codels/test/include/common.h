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

#include <string>
#include "TestUtils.h"

enum TestType {
    DocumentHighlight,
    GoToDefinition,
    References,
    SemanticHighlight,
    Rename,
    TypeHierarchy,
    Hover,
    Diagnostic,
    Completion,
    DocumentSymbol,
    SignatureHelp,
    CallHierarchy,
    CrossLanguageDefinition,
    OverrideMethods,
    CodeAction,
    ApplyEdit,
    FileReference
};

struct TextDocumentEditInfo {
    ark::Location location;
    std::string message;
};

struct TestParam {
    std::string testFile;
    std::string baseFile;
    std::string preId;
    std::string id;
    std::string method;
};

struct TypeHierarchyInfo {
    ark::Location location;
    std::string name;
    int kind;
};

struct HoverInfo {
    ark::Range range;
    std::string message;
};

struct DiagnosticInfo {
    ark::DiagnosticToken diagInfo;
    std::string uri;
};
struct CallHierarchyResult {
    bool isOutgoingCall{true};
    ark::lsp::SymbolID symbolId = ark::lsp::INVALID_SYMBOL_ID;
    std::vector<ark::CallHierarchyIncomingCall> incomingCall;
    std::vector<ark::CallHierarchyOutgoingCall> outgoingCall;
};

struct TypeHierarchyResult {
    ark::lsp::SymbolID symbolId = ark::lsp::INVALID_SYMBOL_ID;
    std::vector<ark::TypeHierarchyItem> subOrSuperTypes;
};

namespace test::common {
    void ShowDiff(const nlohmann::json &inBase, const nlohmann::json &result,
                  const TestParam &param, const std::string &baseUrl = "");

    void ChangeMessageUrlForBaseFile(const std::string &testFilePath, nlohmann::json &resultBase, std::string &rootUri,
                                     bool &isMultiModule);

    void ChangeApplyEditUrlForBaseFile(const std::string &testFilePath, nlohmann::json &resultBase, std::string &rootUri,
        bool &isMultiModule);

    void HandleCodedExpLines(nlohmann::json &expLines);

    std::string GetRealPath(const std::string &fileName);

    std::string GetPwd();

    void SetUpConfig(const std::string &featureName);

    void StartLspServer(bool useDb = false);

    std::string GetRootPath(const std::string &work);

    bool CreateBuildScript(const std::string &execScriptPath, const std::string &testFile);

    void BuildDynamicBinary(const std::string &buildScriptpath);

    bool CheckUseDB(const std::string &message);

    bool CreateMsg(const std::string &path, const std::string &testFile, std::string &rootUri, bool &isMultiModule,
                   const std::string &symbolId = "");

    bool CheckResult(nlohmann::json exp, nlohmann::json &result, TestType testType, std::string &reason);

    void LowFileName(std::basic_string<char> &filePath);

    nlohmann::json ReadFileById(const std::string &file, const std::string &id);

    nlohmann::json ReadFileByMethod(const std::string& file, const std::string& method);

    nlohmann::json ReadExpectedResult(std::string &baseFile);

    std::vector<ark::DocumentHighlight> ReadExpectedVector(std::string &baseFile);

    std::vector<ark::DocumentHighlight> CreateDocumentHighlightStruct(const nlohmann::json &exp);

    bool CheckDocumentHighlight(std::vector<ark::DocumentHighlight> exp, std::vector<ark::DocumentHighlight> act,
                                std::string &reasson);

    std::vector<ark::TypeHierarchyItem> ReadExpectedTypeHierarchyResult(
        std::string &baseFile, TypeHierarchyResult &expect);

    void CreatePrepareTypeHierarchyStruct(const nlohmann::json &exp, TypeHierarchyResult &actual);

    std::vector<ark::TypeHierarchyItem> CreateTypeHierarchyStruct(const nlohmann::json &exp,
                                                                  TypeHierarchyResult &expect);

    bool CheckTypeHierarchyResult(TypeHierarchyResult &actualReulst, TypeHierarchyResult &expectReulst,
                                  std::string &reason);

    void ReadExpectedCallHierarchyResult(std::string &baseFile, CallHierarchyResult &expect);

    void CreatePrePareCallHierarchyStruct(const nlohmann::json &exp, CallHierarchyResult &actual);

    void CreateCallHierarchyStruct(const nlohmann::json &exp, CallHierarchyResult &actual);

    bool CheckCallHierarchyResult(CallHierarchyResult &actual, CallHierarchyResult &expect, std::string &reason);

    std::vector<ark::Location> ReadLocationExpectedVector(const std::string &testFile, std::string &baseFile,
                                                          std::string &rootUri, bool &isMultiModule);

    std::vector<ark::Location> CreateLocationStruct(const nlohmann::json &exp);

    bool CheckLocationVector(std::vector<ark::Location> exp, std::vector<ark::Location> act, std::string &reason);

    std::vector<TextDocumentEditInfo> ReadTextDocumentEditVector(const std::string &testFile, std::string &baseFile,
                                                                 std::string &rootUri, bool &isMultiModule);

    std::vector<TextDocumentEditInfo> CreateTextDocumentEditStruct(const nlohmann::json &exp);

    bool CheckTextDocumentEditVector(std::vector<TextDocumentEditInfo> exp, std::vector<TextDocumentEditInfo> act,
                                     std::string &reason);

    std::vector<TestParam> GetTestCaseList(const std::string &key);

    bool CheckHoverResult(const nlohmann::json &expect, const nlohmann::json &actual, std::string &reason);

    bool CheckDiagnosticResult(const nlohmann::json &expect, const nlohmann::json &actual, std::string &reason);

    bool CheckDocumentSymbolResult(const nlohmann::json &expect, const nlohmann::json &actual, std::string &reason);

    bool CheckSignatureHelpResult(const nlohmann::json &expect, const nlohmann::json &actual, std::string &reason);

    std::vector<ark::SymbolInformation> CreateWorkspaceSymbolStruct(const nlohmann::json &data);

    bool CheckWorkspaceSymbolResult(const nlohmann::json &expect, const nlohmann::json &actual, std::string &reason);

    std::vector<ark::BreakpointLocation> CreateBreakpointStruct(const nlohmann::json &exp);

    std::vector<ark::BreakpointLocation> ReadExpectedBreakpointLocationItems(std::string &baseFile);

    bool CheckBreakpointResult(std::vector<ark::BreakpointLocation> exp, std::vector<ark::BreakpointLocation> act,
                               std::string &reason);

    std::vector<ark::CodeLens> CreateCodeLensStruct(const nlohmann::json &exp);

    std::vector<ark::CodeLens> ReadExpectedCodeLensItems(std::string &baseFile);

    bool CheckCodeLensResult(std::vector<ark::CodeLens> exp, std::vector<ark::CodeLens> act,
                             std::string &reason);    

    bool IsMacroExpandTest(const std::string &rootUri);
    
    bool IsMacroExpandTest(const std::string &rootUri);

    ark::FindOverrideMethodResult CreateOverrideMethodsResult(const nlohmann::json& exp);

    bool CheckOverrideMethodsResult(const nlohmann::json& expect, const nlohmann::json& actual, std::string& reason);
}
