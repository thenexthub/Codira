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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_CONTEXT_H
#define PANDA_ASSEMBLER_ASSEMBLY_CONTEXT_H

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "assembly-ins.h"
#include "assembly-type.h"
#include "error.h"
#include "lexer.h"

namespace ark::pandasm {

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
/*
 * Used to move around tokens.
 * *context :
 * Returns current value of a token
 * ++context :
 * sets the next token value
 * returns current value of a token
 * context++ :
 * sets the next token value
 * returns the next value of a token
 * similarly --context and context--
 */
struct Context {
    std::string_view token;                   /* current token */
    std::vector<ark::pandasm::Token> tokens;  /* token list */
    size_t number = 0;                        /* line number */
    bool end = false;                         /* end of line flag */
    Token::Type id = Token::Type::ID_BAD;     /* current token type */
    Token::Type signop = Token::Type::ID_BAD; /* current token operand type (if it is an operation) */
    ark::pandasm::Error err;                  /* current error */
    int64_t *maxValueOfReg = nullptr;
    size_t insNumber = 0;
    Type currFuncReturnType;
    std::vector<std::pair<size_t, size_t>> *functionArgumentsList = nullptr;
    std::unordered_map<std::string, std::vector<std::pair<size_t, size_t>>> functionArgumentsLists;

    void Make(const std::vector<ark::pandasm::Token> &t);
    void UpSignOperation();
    bool ValidateRegisterName(char c, size_t n = 0) const;
    bool ValidateFoundedRegisterName(char c, size_t n) const;
    bool ValidateParameterName(size_t numberOfParamsAlreadyIs) const;
    bool ValidateLabel();
    bool Mask();
    bool NextMask();
    size_t Len() const;
    std::string_view GiveToken();
    Token::Type WaitFor();
    Token::Type Next();

    Token::Type operator++(int);  // NOLINT(cert-dcl21-cpp)
    Token::Type operator--(int);  // NOLINT(cert-dcl21-cpp)
    Token::Type operator++();
    Token::Type operator--();
    Token::Type operator*();
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_CONTEXT_H
