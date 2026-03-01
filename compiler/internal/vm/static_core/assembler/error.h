/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_ASSEMBLER_ERROR_H
#define PANDA_ASSEMBLER_ERROR_H

#include <string>

namespace ark::pandasm {

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct Error {
    enum class ErrorClass { WARNING = 0, ERROR };
    enum class ErrorType {
        ERR_NONE = 0,

        // Lexer
        ERR_STRING_MISSING_TERMINATING_CHARACTER,

        // Parser
        ERR_BAD_LABEL,
        ERR_BAD_LABEL_EXT,
        ERR_BAD_NAME_ID,
        ERR_BAD_NAME_REG,
        ERR_BAD_INTEGER_NAME,
        ERR_BAD_INTEGER_WIDTH,
        ERR_BAD_FLOAT_NAME,
        ERR_BAD_FLOAT_WIDTH,
        ERR_BAD_NUMBER_OPERANDS,
        ERR_BAD_OPERAND,
        ERR_BAD_OPERATION_NAME,
        ERR_BAD_NONEXISTING_OPERATION,
        ERR_BAD_ID_FUNCTION,
        ERR_BAD_ID_RECORD,
        ERR_BAD_ID_ARRAY,
        ERR_BAD_ID_FIELD,
        ERR_BAD_FUNCTION_NAME,
        ERR_BAD_RECORD_NAME,
        ERR_BAD_ARRAY_NAME,
        ERR_BAD_DEFINITION_METADATA,
        ERR_BAD_DEFINITION_FUNCTION,
        ERR_BAD_DEFINITION_RECORD,
        ERR_BAD_METADATA_BOUND,
        ERR_BAD_METADATA_UNKNOWN_ATTRIBUTE,
        ERR_BAD_METADATA_INVALID_VALUE,
        ERR_BAD_METADATA_MISSING_ATTRIBUTE,
        ERR_BAD_METADATA_MISSING_VALUE,
        ERR_BAD_METADATA_UNEXPECTED_ATTRIBUTE,
        ERR_BAD_METADATA_UNEXPECTED_VALUE,
        ERR_BAD_METADATA_MULTIPLE_ATTRIBUTE,
        ERR_BAD_FUNCTION_PARAMETERS,
        ERR_BAD_FUNCTION_RETURN_VALUE,
        ERR_FUNCTION_ARGUMENT_MISMATCH,
        ERR_BAD_FIELD_MISSING_NAME,
        ERR_BAD_FIELD_VALUE_TYPE,
        ERR_BAD_FIELD_METADATA,
        ERR_BAD_ARRAY_TYPE,
        ERR_BAD_ARRAY_SIZE,
        ERR_BAD_ARRAY_SIZE_VALUE,
        ERR_BAD_ARRAY_ELEMENT_MISSING_VALUE,
        ERR_BAD_ARRAY_ELEMENT_VALUE_TYPE,
        ERR_BAD_ARRAY_ELEMENT_VALUE,
        ERR_BAD_ARRAY_ELEMENT_VALUE_INTEGER,
        ERR_BAD_ARRAY_ELEMENT_VALUE_FLOAT,
        ERR_BAD_ARRAY_ELEMENT_VALUE_STRING,
        ERR_BAD_CHARACTER,
        ERR_BAD_KEYWORD,
        ERR_BAD_DEFINITION,
        ERR_BAD_BOUND,
        ERR_BAD_END,
        ERR_BAD_CLOSE,
        ERR_BAD_ARGS_BOUND,
        ERR_BAD_TYPE,
        ERR_BAD_PARAM_NAME,
        ERR_BAD_NOEXP_DELIM,
        ERR_BAD_STRING_INVALID_HEX_ESCAPE_SEQUENCE,
        ERR_BAD_STRING_UNKNOWN_ESCAPE_SEQUENCE,
        ERR_BAD_ARRAY_TYPE_BOUND,
        ERR_UNDEFINED_TYPE,
        ERR_MULTIPLE_DIRECTIVES,
        ERR_INCORRECT_DIRECTIVE_LOCATION,
        ERR_BAD_DIRECTIVE_DECLARATION,
        ERR_UNKNOWN_LANGUAGE,
        ERR_BAD_MNEMONIC_NAME,
        ERR_REPEATING_FIELD_NAME,
        ERR_FUNCTION_MULTIPLE_ALTERNATIVES,
        ERR_BAD_SIGNATURE,
        ERR_BAD_SIGNATURE_PARAMETERS,

        // Warnings
        WAR_UNEXPECTED_RETURN_TYPE,
        WAR_UNEXPECTED_TYPE_ID,
    };

    ErrorClass type;
    std::string wholeLine;
    size_t pos; /* positions to highlight the word */
    size_t end;
    ErrorType err;
    std::string message;
    std::string verbose;
    size_t lineNumber;

    inline Error() : Error("No messages", 0, ErrorType::ERR_NONE, "", 0, 0, "") {}

    // CC-OFFNXT(G.FUN.01-CPP) solid logic
    inline Error(std::string s, size_t line, ErrorType errorType, std::string overinfo, size_t p, size_t e,
                 std::string buff, ErrorClass classType = ErrorClass::ERROR)
        : type(classType),
          wholeLine(std::move(buff)),
          pos(p),
          end(e),
          err(errorType),
          message(std::move(s)),
          verbose(std::move(overinfo)),
          lineNumber(line)
    {
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

using ErrorList = std::vector<Error>;
}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ERROR_H
