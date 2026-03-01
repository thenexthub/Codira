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

class SubObj {
    s: string = "qwerty"
}
class ObjToDump {
    a: bigint = new BigInt(1)
    b: string = "abcbcb"
    c: Number = 21222.22219
    d: undefined = undefined
    e: SubObj = new SubObj()
}

function main(): void {
    testStringigyEmptyStr()
    testStringifyObj()
    testStringifyObjWithArrayReplacer()
    testStringifyObjWithFuncReplacer()
    testStringifyObjNullReplacer()
    testStringifyObjUndefReplacer()
    testStringifyNull()
    testStringifyUndefined()
}

function testStringigyEmptyStr() {
    arktest.assertEQ(JSON.stringify(""), "\"\"")
}

function testStringifyObj() {
    arktest.assertEQ(JSON.stringify(new ObjToDump()), "{\"a\":1,\"b\":\"abcbcb\",\"c\":21222.22219,\"e\":{\"s\":\"qwerty\"}}")
}

function testStringifyObjWithArrayReplacer() {
    arktest.assertEQ(JSON.stringify(new ObjToDump(), ["a", "b"]), "{\"a\":1,\"b\":\"abcbcb\"}" )
}

function testStringifyObjWithFuncReplacer() {
    let f = (k: string, value: Any): Any => {
        if (k === "a") {
            return 77777
        } else if (k === "not existing") {
            return "blablabla"
        }
        return value
    }
    arktest.assertEQ(JSON.stringify(new ObjToDump(), f, " "), "{\n" + " \"a\": 77777,\n" + " \"b\": \"abcbcb\",\n" + 
        " \"c\": 21222.22219,\n" + " \"e\": {\n" + "  \"s\": \"qwerty\"\n" + " }\n" +"}")
}

function testStringifyObjNullReplacer() {
    arktest.assertEQ(JSON.stringify(new ObjToDump(), null, 0), "{\"a\":1,\"b\":\"abcbcb\",\"c\":21222.22219,\"e\":{\"s\":\"qwerty\"}}")
}

function testStringifyObjUndefReplacer() {
    arktest.assertEQ(JSON.stringify(new ObjToDump(), undefined, 0), "{\"a\":1,\"b\":\"abcbcb\",\"c\":21222.22219,\"e\":{\"s\":\"qwerty\"}}")
}

function testStringifyNull() {
    arktest.assertEQ(JSON.stringify(null, undefined, 0), "null")
}

function testStringifyUndefined() {
    arktest.assertEQ(JSON.stringify(undefined, undefined, 0), "undefined")
}
