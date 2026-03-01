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

function check(str: String): int {
    let it = str.$_iterator()
    let fin = ""
    while (true) {
        let res = it.next()
        if (res.done) {
            break
        }
        fin += res.value
    }
    if (str != fin) {
        console.log("Incorrect iterator working!")
        return 1
    }
    return 0
}

function main(): int {
    let fails = 0
    let tests = [
        "",
        "abc",
        "aaaaaaaaaaaaaaaaaaaaaaaaaa",
        "iuq634o7vpq43w76iyvu3oqnpmc-mqynu",
        "FGkT Et t ФКШПШГЙ\N\N\N\n\n\n\r\r\r",
        "\taa \t   \t      \tt\t\taaaaaaaaaaaaaaaaaa\t",
        "\r\n\n\r",
        "\n\r\r\n",
        "\v\v\v\n\n\n\v\v\n\r\r\v\r\n\r\n\v\r\v",
        "\0",
        "aaa\0aaa",
        "SS$SS aaаа",
    ]
    for (let test of tests) {
        fails += check(test)
    }
    return fails
}