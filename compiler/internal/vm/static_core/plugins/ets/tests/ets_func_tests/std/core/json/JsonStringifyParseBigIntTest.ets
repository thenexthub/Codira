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

function main() {
    testStringifyParseBigInt()
    testStringifyParseOptOne()
    testStringifyParseOptZero()
    testStringifyParseBigIntWithReviver()
    testParseNoBigInt()
    testParseNoBigIntParseAsBigint()
    testParseNoBigIntAlwaysParseAsBigint()
}

export class User {}

class UserWithFields extends User {
    id: BigInt = new BigInt()
    // NOTE(akmaev): if keep just Boolean it will throw RTE,
    // because it will try to assign std.core.Boolean to primitive boolean
    // seems need to fix Json.parse internals
    g: Boolean|undefined = false
    name: String = ""
}

class SuperUser extends UserWithFields {
    users: FixedArray<UserWithFields> = []
}

const str1 = "{\"id\":10,\"g\":true,\"name\":\"Name\",\"users\":[{\"id\":12,\"g\":true,\"name\":\"Name1\"},{\"id\":13,\"g\":false,\"name\":\"Name2\"}]}"

function doTest(mode: jsonx.BigIntMode, filter?: (k: string, v: Any) => Any) {
    const classType = Type.of(new SuperUser() as Object)
    return JSON.parse<SuperUser>(str1, filter, classType, {bigIntMode: mode} as jsonx.ParseOptions) as SuperUser
}

function testStringifyParseBigInt(): void {
    let x = JSON.stringify(doTest(jsonx.BigIntMode.ALWAYS_PARSE_AS_BIGINT))
    arktest.assertEQ(x, str1)
}

function testStringifyParseOptOne(): void {
    arktest.expectThrow(() => { doTest(jsonx.BigIntMode.PARSE_AS_BIGINT) })
}

function testStringifyParseOptZero(): void {
    arktest.expectThrow(() => { doTest(jsonx.BigIntMode.DEFAULT) })
}

function testStringifyParseBigIntWithReviver(): void {
    let reviver = (k: string, v: Any) => {
        if (k === "id") {
            return new BigInt(1)
        }
        return v
    }
    let expected_str = "{\"id\":1,\"g\":true,\"name\":\"Name\",\"users\":[{\"id\":1,\"g\":true,\"name\":\"Name1\"},{\"id\":1,\"g\":false,\"name\":\"Name2\"}]}";
    let x = JSON.stringify(doTest(jsonx.BigIntMode.ALWAYS_PARSE_AS_BIGINT, reviver))
    arktest.assertEQ(x, expected_str)
}


class NumbersTest extends User {
    val: Number = 1
    str: String = ""
    bul: Boolean|undefined = false
}


const str2 = "{\"val\":123444444421,\"str\":\"true\",\"bul\":false}"
function doTest2(mode: jsonx.BigIntMode) {
    let classType = Type.of(new NumbersTest())
    return JSON.stringify(JSON.parse<NumbersTest>(str2, undefined, classType, {bigIntMode: mode} as jsonx.ParseOptions) as NumbersTest)
}

function testParseNoBigInt() {
    let x = doTest2(jsonx.BigIntMode.DEFAULT)
    arktest.assertEQ(x, str2)
}

function testParseNoBigIntParseAsBigint() {
    let x = doTest2(jsonx.BigIntMode.PARSE_AS_BIGINT)
    arktest.assertEQ(x, str2)
}

function testParseNoBigIntAlwaysParseAsBigint() {
    arktest.expectThrow(() => { doTest2(jsonx.BigIntMode.ALWAYS_PARSE_AS_BIGINT) })
}
