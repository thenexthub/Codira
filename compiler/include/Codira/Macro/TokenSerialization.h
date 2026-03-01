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

/*
 * @file
 *
 * This file declares the Codira Token Serialization.
 */

#ifndef CODIRA_MODULES_TOKENSERIALIZATION_H
#define CODIRA_MODULES_TOKENSERIALIZATION_H

#include <fstream>
#include <set>
#include <vector>
#include <cstdint>

#include "Codira/Lex/Token.h"

namespace TokenSerialization {
using namespace Codira;

std::vector<uint8_t> GetTokensBytes(const std::vector<Token>& tokens);

std::vector<Token> GetTokensFromBytes(const uint8_t* pBuffer);

uint8_t* GetTokensBytesWithHead(const std::vector<Token>& tokens);
} // namespace TokenSerialization

#endif // CODIRA_MODULES_TOKENSERIALIZATION_H
