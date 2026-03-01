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

#include "program.h"

#include "utils/logger.h"
#include "util/string_util.h"
#include "util/assert_util.h"
#include "configs/guard_context.h"
#include "graph_analyzer.h"
#include "annotation.h"

namespace {
constexpr std::string_view TAG = "[Program]";
constexpr std::string_view ANNOTATION_NODE_DELIMITER = ".";
}  // namespace

void panda::guard::Program::Create()
{
    LOG(INFO, PANDAGUARD) << TAG << "===== program create start =====";

    for (const auto &[_, record] : this->prog_->record_table) {
        this->CreateNode(record);
    }

    LOG(INFO, PANDAGUARD) << TAG << "===== program create end =====";
}

void panda::guard::Program::CreateNode(const pandasm::Record &record)
{
    auto node = std::make_shared<Node>(this, record.name);
    node->InitWithRecord(record);

    if (node->type_ == NodeType::JSON_FILE) {
        node->Create();
        this->nodeTable_.emplace(record.name, node);
        return;
    }

    if (node->type_ == NodeType::SOURCE_FILE) {
        if (GuardContext::GetInstance()->GetGuardOptions()->IsSkippedRemoteHar(node->pkgName_)) {
            LOG(INFO, PANDAGUARD) << TAG << "skip record: " << record.name;
            return;
        }

        node->Create();
        this->nodeTable_.emplace(record.name, node);
        return;
    }

    if (node->type_ == NodeType::ANNOTATION) {
        this->CreateAnnotation(record);
    }
}

void panda::guard::Program::CreateAnnotation(const pandasm::Record &record)
{
    if (Annotation::IsWhiteListAnnotation(record.name)) {
        return;
    }

    auto [nodeName, annoName] = StringUtil::RSplitOnce(record.name, ANNOTATION_NODE_DELIMITER.data());
    auto node = this->nodeTable_.find(nodeName);
    PANDA_GUARD_ASSERT_PRINT(node == this->nodeTable_.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "parse annotation get bad nodeName:" << nodeName);
    auto annotation = std::make_shared<Annotation>(this, annoName);
    annotation->nodeName_ = nodeName;
    annotation->recordName_ = record.name;
    annotation->needUpdate_ = node->second->needUpdate_;
    annotation->scope_ = TOP_LEVEL;
    annotation->export_ = node->second->moduleRecord_.IsExportVar(annoName);
    annotation->Create();
    node->second->annotations_.emplace_back(annotation);
}

void panda::guard::Program::EnumerateFunctions(const std::function<FunctionTraver> &callback)
{
    for (auto &[_, node] : this->nodeTable_) {
        node->EnumerateFunctions(callback);
    }
}

void panda::guard::Program::RemoveConsoleLog()
{
    if (!GuardContext::GetInstance()->GetGuardOptions()->IsRemoveLogObfEnabled()) {
        return;
    }

    this->EnumerateFunctions([](Function &function) { function.RemoveConsoleLog(); });
}

void panda::guard::Program::RemoveLineNumber()
{
    if (!GuardContext::GetInstance()->GetGuardOptions()->IsCompactObfEnabled()) {
        return;
    }

    this->EnumerateFunctions([](Function &function) { function.RemoveLineNumber(); });
}

void panda::guard::Program::Obfuscate()
{
    LOG(INFO, PANDAGUARD) << TAG << "===== program obfuscate start =====";

    for (auto &[name, node] : this->nodeTable_) {
        node->Obfuscate();
    }

    this->UpdateReference();

    this->RemoveConsoleLog();

    this->RemoveLineNumber();

    LOG(INFO, PANDAGUARD) << TAG << "===== program obfuscate end =====";
}

void panda::guard::Program::UpdateReference()
{
    this->EnumerateFunctions([](Function &function) -> void {
        function.UpdateReference();
        function.UpdateAnnotationReference();
    });
    for (auto &[_, node] : this->nodeTable_) {
        node->UpdateFileNameReferences();
    }
}
