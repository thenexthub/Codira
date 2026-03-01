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

/**
 * @file
 *
 * This file declares the Annotation Token related classes, which is set of lexcial annotation tokens of Codira.
 */

#ifndef CODIRA_LEX_ANNOTATION_TOKEN_H
#define CODIRA_LEX_ANNOTATION_TOKEN_H

#include <cstring>
#include <string>

#include "Codira/Basic/Position.h"

namespace Codira {
inline const char* ANNOTATION_TOKENS[] = {
#define ANNOTATION_TOKEN(ID, VALUE, LITERAL, INVALID_PRECEDENCE) LITERAL,
#include "Codira/Lex/AnnotationTokens.inc"
#undef ANNOTATION_TOKEN
};

} // namespace Codira

#endif // CODIRA_LEX_ANNOTATION_TOKEN_H
