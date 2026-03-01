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

function main(): void {
    let regexpNames = new RegExp("姓氏：(?<first>.+)，名字：(?<last>.+)", "gmd");
    let users = `姓氏：李，名字：雷\n姓氏：韩，名字：梅梅`;
    let result = regexpNames.exec(users);
    arktest.assertEQ(regexpNames.hasIndices, true, "The value of hasIndices is expected to be true");
    arktest.assertEQ(regexpNames.flags, "dgm", "The value of flags is expected to be 'dgm'");
    arktest.assertEQ(String(result?.indices[0] || ''), "0,9", "The value of indices[0] is expected to be [0, 9]");
    arktest.assertEQ(String(result?.indices[1] || ''), "3,4", "The value of indices[1] is expected to be [3, 4]");
    arktest.assertEQ(String(result?.indices[2] || ''), "8,9", "The value of indices[2] is expected to be [8, 9]");
    arktest.assertEQ(String(result?.groups?.['first'] || ''), "李", "The value of groups.first is expected to be ‘李’");
    arktest.assertEQ(String(result?.groups?.['last'] || ''), "雷", "The value of groups.last is expected to be ‘雷’");

    result = regexpNames.exec(users);
    arktest.assertEQ(String(result?.indices[0] || ''), "10,20", "The value of indices[0] is expected to be [10, 20]");
    arktest.assertEQ(String(result?.indices[1] || ''), "13,14", "The value of indices[1] is expected to be [13, 14]");
    arktest.assertEQ(String(result?.indices[2] || ''), "18,20", "The value of indices[2] is expected to be [18, 20]");
    arktest.assertEQ(String(result?.groups?.['first'] || ''), "韩", "The value of groups.first is expected to be ‘韩’");
    arktest.assertEQ(String(result?.groups?.['last'] || ''), "梅梅", "The value of groups.last is expected to be ‘梅梅’");

    let regexp2 = new RegExp("(π)", "du");
    let result2 = "bπb".match(regexp2)?.indices;
    arktest.assertEQ(String(result2?.[0] || ''), "1,2", "The value of result2[0] is expected to be [1, 2]");
    arktest.assertEQ(String(result2?.[1] || ''), "1,2", "The value of result2[1] is expected to be [1, 2]");
    let regexpFlags = new RegExp("", "dgimsuy").flags;
    arktest.assertEQ(regexpFlags, "dgimsuy", "The value of regexpFlags is expected to be 'dgimsuy'");
}
