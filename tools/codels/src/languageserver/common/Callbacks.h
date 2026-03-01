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

#ifndef LSPSERVER_CALLBACK_H
#define LSPSERVER_CALLBACK_H
#include <iostream>
#include <vector>
#include "../../json-rpc/Protocol.h"
#include "../logger/Logger.h"
#include "Codira/Basic/DiagnosticEngine.h"

namespace Codira {
struct TextEdit {
    Position start;
    Position end;
    std::string newText;
    friend bool operator!=(const TextEdit& teOne, const TextEdit& teTwo)
    {
        return !(teOne.start == teTwo.start && teOne.end == teTwo.end && teOne.newText == teTwo.newText);
    }
    friend bool operator==(const TextEdit& teOne, const TextEdit& teTwo)
    {
        return teOne.start == teTwo.start && teOne.end == teTwo.end && teOne.newText == teTwo.newText;
    }
};
} // namespace Codira

namespace ark {
class Callbacks {
public:
    virtual ~Callbacks() = default;

    virtual std::vector<DiagnosticToken> GetDiagsOfCurFile(std::string) = 0;
    virtual void UpdateDiagnostic(std::string, DiagnosticToken) = 0;
    virtual void RemoveDiagOfCurPkg(const std::string &dirName) = 0;
    virtual void RemoveDiagnostic(std::string, DiagnosticToken) = 0;
    virtual void ReadyForDiagnostics(std::string, int64_t, std::vector <DiagnosticToken>) = 0;
    virtual std::string GetContentsByFile(const std::string &file) = 0;
    virtual int64_t GetVersionByFile(const std::string &file) = 0;
    virtual void RemoveDocByFile(const std::string &file)  = 0;
    virtual bool NeedReParser(const std::string &file) = 0;
    virtual void UpdateDocNeedReparse(const std::string &file, int64_t version, bool needReParser) = 0;
    virtual void AddDocWhenInitCompile(const std::string &file) = 0;
    virtual void ReportCodeoVersionErr(std::string message) = 0;
    virtual void PublishCompletionTip(const CompletionTip &params) = 0;
    bool isRenameDefined = false;
    std::string path = "";
};
} // namespace ark
#endif // LSPSERVER_CALLBACK_H
