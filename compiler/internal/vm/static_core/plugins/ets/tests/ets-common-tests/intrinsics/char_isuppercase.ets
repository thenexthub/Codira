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

function main(): int {
    let upperCase = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'.getChars();
    let lowerCase = 'abcdefghijklmnopqrstuvwxyz'.getChars();
    for (let i = 0; i < upperCase.length; i++) {
        let uc = upperCase[i];
        let lc = lowerCase[i];
        if (!Char.isUpperCase(uc) || Char.isLowerCase(uc)) {
            return 1;
        }
        if (!Char.isLowerCase(lc) || Char.isUpperCase(lc)) {
            return 2;
        }
        if (Char.toLowerCase(lc) != lc || Char.toLowerCase(uc) != lc) {
            return 3;
        }
        if (Char.toUpperCase(lc) != uc || Char.toUpperCase(uc) != uc) {
            return 4;
        }
    }
    for (let ch: char = c'\0'; ch != c'A'; ch = (ch.toInt() + 1).toChar()) {
        if (Char.toUpperCase(ch) != ch || Char.toLowerCase(ch) != ch || Char.isUpperCase(ch) || Char.isLowerCase(ch)) {
            return 5;
        }
    }
    for (let ch: char = (c'Z'.toInt() + 1).toChar(); ch != c'a'; ch = (ch.toInt() + 1).toChar()) {
        if (Char.toUpperCase(ch) != ch || Char.toLowerCase(ch) != ch || Char.isUpperCase(ch) || Char.isLowerCase(ch)) {
            return 6;
        }
    }
    for (let ch: char = (c'z'.toInt() + 1).toChar(); ch != Int.toChar(65535); ch = (ch.toInt() + 1).toChar()) {
        if (Char.toUpperCase(ch) != ch || Char.toLowerCase(ch) != ch || Char.isUpperCase(ch) || Char.isLowerCase(ch)) {
            return 6;
        }
    }
    return 0;
}
