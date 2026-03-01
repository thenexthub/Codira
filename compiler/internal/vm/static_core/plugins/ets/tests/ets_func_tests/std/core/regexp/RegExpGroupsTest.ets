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

let fromTS = [
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "groups is undefined",
    "undefined",
    "a:a,",
    "a:a,",
    "a:a,",
    "a:a,",
    "a:a,",
    "a:a,",
    "a:a,",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "a:a,",
    "a:a,",
    "a:a,",
    "undefined",
    "undefined",
    "undefined",
    "a:abacaba,",
    "a:abacaba,",
    "a:abacaba,",
    "a:abacaba,",
    "a:abacaba,",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "a:abacaba,b:bacadaba,",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "a:abacaba,b:undefined,",
    "a:abacaba,b:undefined,",
    "a:abacaba,b:undefined,",
    "a:abacaba,b:undefined,",
    "a:abacaba,b:undefined,",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "a:abacaba,b:bacadaba,",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "a:123,b:123,",
    "a:345,b:345,",
    "undefined",
    "a:123,b:345,",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "a:123,b:undefined,",
    "a:123,b:undefined,",
    "a:345,b:undefined,",
    "a:345,b:undefined,",
    "a:123,b:undefined,",
    "a:123,b:undefined,",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "a:123,b:123,",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
    "undefined",
]

function groupsTest(): void {
    let patterns = [
        "",
        "()",
        "(?<a>a)",
        "(?<a>abacaba)",
        "(?<a>abacaba)123",
        "123(?<a>abacaba)",
        "(?<a>abacaba)(?<b>bacadaba)",
        "(?<a>abacaba)|(?<b>bacadaba)",
        "(?<a>abacaba)aaa(?<b>bacadaba)",
        "(?<a>123|345)(?<b>345|123)",
        "(?<a>123|345)|(?<b>345|123)",
        "(?<a>123|345)aaa(?<b>345|123)",
        "(?<a>[0-9]\\d{4})aaa(?<b>[a-z]\\d{4})",
    ]

    let inputs = [
        "",
        "aavbaadkjtnyrioucvyr347 t5b v8721487c12578 14785t v8721487c12578",
        "akldst 5rtuiq4 667tyrqtr48quoi4rtfv7q43htfr67q4g8dq64ht67qdhwgrqcmhd",
        "abacababacadaba",
        "abacabaaaabacadaba",
        "abacaba1325143614356bacadaba",
        "abacaba____________bacadaba",
        "abacaba\nbacadaba",
        "1234567890987654321",
        "123123123",
        "345345",
        "345",
        "123345",
        "123aaa123",
        "13252435234623aaa12256562363",
        "13252435234623aa12256562363",
    ]

    let currentId = 0
    for (let pattern of patterns) {
        for (let input of inputs) {
            let ans = ""
            let res = new RegExp(pattern).exec(input)
            if (!res) {
                ans += 'undefined'
            } else {
                if (!res!.groups) {
                    ans += 'groups is undefined'
                } else {
                    let groups: Record<String, String | undefined> = res!.groups!
                    for (let g of groups!.keys()) {
                        ans += g + ':' + groups![g] + ','
                    }
                }
            }
            arktest.assertEQ(ans, fromTS[currentId], "new RegExp('" + pattern + "').exec('" + input + "') : FAILED\n" +
                "expected: " + fromTS[currentId] + "\n" +
                "got: " + ans)
            currentId += 1
        }
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite("ArrayBuffer properties")

    suite.addTest("RegExp named groups check", groupsTest)

    return suite.run()
}
