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
desc: >-
    An embedded expression is an expression specified inside the dollar
    sign '$' and curly braces. The type of a string interpolation expression
    is string.
---*/

const count = `${ 2 + (() => 3 - 1)()}`
const interpolation: String = 'interpolation'
const cr = '\n'

const sentence = `This is an example of
a multiline string, contains ${count} lines and with an ${interpolation}
which should be enclosed in${cr}backticks`

function main() {
    let exp = "This is an example of\na multiline string, contains 4 lines and with an interpolation\nwhich should be enclosed in\nbackticks"
    arktest.assertEQ(exp, sentence)
}
