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


/*---
desc: Issue 21151. Trailing comma in tuple causes SyntaxError
tags: [compile-only]
---*/

interface II {
}

type TLambda = () => void

type IDecl = II

type GoodTupleWithTrailingComma0 = [string, number, ]
type GoodTupleWithTrailingComma1 = [string, number, II, ]
type GoodTupleWithTrailingComma2 = [string, number, TLambda, ]
type GoodTupleWithTrailingComma3 = [string, number, IDecl, ]
type GoodTupleWithTrailingComma4 = [string, number, GoodTupleWithTrailingComma0, ]
