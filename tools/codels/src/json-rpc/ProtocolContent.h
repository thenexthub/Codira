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

#ifndef LSPSERVER_PROTOCOL_CONTENT_H
#define LSPSERVER_PROTOCOL_CONTENT_H

namespace ark {

enum class SignatureHelpTriggerKind {
    END = 4
};

// Sync document changes strategy for language server
enum class TextDocumentSyncKind {
    // Documents not be synced at any time.
    SK_NONE = 0,

    // Smart sync, Documents are synced using the full content on open.
    // After only incremental updates to the document.
    SK_INCREMENTAL = 2,
};

enum class DocumentHighlightKind {
    TEXT = 1,
};

enum class FileChangeType {
    // The instruction file was created.
    CREATED = 1,
    // The instruction file was changed.
    CHANGED = 2,
    // The instruction file was deleted.
    DELETED = 3,
};

enum class ErrorCode {
    PARSE_ERROR = -32700,
    INVALID_REQUEST = -32600,
    METHOD_NOT_FOUND = -32601,
    SERVER_NOT_INITIALIZED = -32002,
    UNKNOWN_ERROR_CODE = -32001,
    // Customized error code. (>= -31999 or <= -32900)
    INVALID_RENAME_FOR_MACRO_CALL_FILE = -31999
};

enum class SemanticTokenTypes {
    COMMENT_T = 0,
    KEYWORD_T = 1,
    NUMBER_T = 3,
    OPERATOR_T = 5,
    NAMESPACE_T = 6,
    TYPE_T = 7,
    STRUCT_T = 8,
    CLASS_T = 9,
    INTERFACE_T = 10,
    ENUM_T = 11,
    TYPE_PARAMETER_T = 12,
    FUNCTION_T = 13,
    PROPERTY_T = 15,
    MACRO_T = 16,
    VARIABLE_T = 17,
    LABEL_T = 19
};

enum class HighlightKind {
    FILE_H = 1,
    MODULE_H = 2,
    NAMESPACE_H = 3,
    PACKAGE_H = 4,
    CLASS_H = 5,
    METHOD_H = 6,
    PROPERTY_H = 7,
    FIELD_H = 8,
    CONSTRUCTOR_H = 9,
    ENUM_H = 10,
    INTERFACE_H = 11,
    FUNCTION_H = 12,
    VARIABLE_H = 13,
    CONSTANT_H = 14,
    NUMBER_H = 16,
    BOOLEAN_H = 17,
    ARRAY_H = 18,
    OBJECT_H = 19,
    KEY_H = 20,
    MISSING_H = 21,
    ENUMMEMBER_H = 22,
    STRUCT_H = 23,
    EVENT_H = 24,
    OPERATOR_H = 25,
    TYPEPARAMETER_H = 26,
    COMMENT_H = 27,
    RECORD_H = 28,
    TRAIT_H = 29,
    INACTIVECODE_H
};
} // namespace ark
#endif // LSPSERVER_PROTOCOL_CONTENT_H
