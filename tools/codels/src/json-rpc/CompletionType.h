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

#ifndef LSPSERVER_COMPLETIONTYPE_H
#define LSPSERVER_COMPLETIONTYPE_H

#include <optional>
#include "Common.h"

/**
 * According to the language service protocol to create structure
 * see https://microsoft.github.io/language-server-protocol/specifications/specification-3-16/#baseProtocol
 */
namespace ark {

enum class CompletionTriggerKind {
    INVOKED = 1,

    TRIGGER_CHAR = 2,
};

struct CompletionContext {
    CompletionTriggerKind triggerKind = CompletionTriggerKind::INVOKED;

    std::string triggerCharacter = "";

    CompletionContext(): triggerKind(CompletionTriggerKind::INVOKED), triggerCharacter("") {};
};

enum class CompletionItemKind {
    CIK_MISSING = 0,
    CIK_METHOD = 2,
    CIK_FUNCTION = 3,
    CIK_CONSTRUCTOR = 4,
    CIK_VARIABLE = 6,
    CIK_CLASS = 7,
    CIK_INTERFACE = 8,
    CIK_MODULE = 9,
    CIK_ENUM = 13,
    CIK_KEYWORD = 14,
    CIK_STRUCT = 22,
};

enum class InsertTextFormat {
    MISSING = 0,

    PLAIN_TEXT = 1,

    SNIPPET = 2,
};

struct TextEdit {
    Range range;

    std::string newText;

    TextEdit() {};

    TextEdit(const Range &range, const std::string &newText) : range(range), newText(newText)
    {
    }

    bool operator<(const TextEdit &rhs) const
    {
        return std::tie(this->range, this->newText) < std::tie(rhs.range, rhs.newText);
    }

    bool operator==(const TextEdit &other) const {
        return this->range == other.range && this->newText == other.newText;
    }
};

struct CompletionItem {
    std::string label = "";

    CompletionItemKind kind = CompletionItemKind::CIK_MISSING;

    std::string detail = "";

    std::string documentation = "";

    std::string sortText = "";

    std::string filterText = "";

    std::string insertText = "";

    InsertTextFormat insertTextFormat = InsertTextFormat::MISSING;

    std::optional<TextEdit> textEdit;

    std::optional<std::vector<TextEdit>> additionalTextEdits;

    bool deprecated = false;

    CompletionItem()
        : label(""),
          kind(CompletionItemKind::CIK_MISSING),
          detail(""),
          documentation(""),
          sortText(""),
          filterText(""),
          insertText(""),
          insertTextFormat(InsertTextFormat::MISSING),
          textEdit(),
          deprecated(false)
    {};
};
}
#endif // LSPSERVER_COMPLETIONTYPE_H
