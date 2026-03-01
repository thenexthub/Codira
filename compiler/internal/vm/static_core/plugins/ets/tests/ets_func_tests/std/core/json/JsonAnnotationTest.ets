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

const INT8_MAX: number = Byte.MAX_VALUE; //127
const INT8_MIN: number = Byte.MIN_VALUE; //-128

class TestClass1 {
    @JSONRename("__backing_user")
    user: string = ""
    @JSONStringifyIgnore
    @JSONParseIgnore
    password: number = 0
    @JSONParseIgnore
    nickname: string = ""

    setPassword(pwd: number) {
        this.password = pwd
    }
    setUserName(name: string) {
        this.user = name
    }
    setNickName(name: string) {
        this.nickname = name
    }
}

class TestClass2 {
    @JSONRename("nickname")
    user: string = ""
    password: number = 0
    nickname: string = ""
}

class TestClass3 {
    @JSONRename("nickname1")
    user: string = ""
    password: number = 0
    @JSONRename("nickname1")
    nickname: string = ""
}

class TestClass4 {
    @JSONRename("nickname1")
    user: string = ""
    password: number = 0
    nickname: string = ""
}

class TestClass5 {
    name1: string = "class5"
}

class TestClass6 extends TestClass5 {
    name1: string = "class6"
    nickname: string = "class6_nickname"
}

class TestClass7 extends TestClass5 {
    name1: string = "class7"
    nickname: string = "class7_nickname"
}

class TestClass8 {
    @JSONRename("name8")
    name1: string = "TestClass8"
    nickname: string = "class8_nickname"
}

class TestClass9 {
    name1: string = "TestClass9"
    nickname: TestClass8 = new TestClass8()
}

class TestClass10 extends TestClass8 {
    @JSONRename("name8")
    name10: string = "TestClass10"
}

class TestClass11 extends TestClass8 {
    @JSONRename("name1")
    name11: string = "TestClass11"
}

class TestClass12 extends TestClass8 {
    @JSONRename("name12test")
    name12: string = "TestClass12"
}

class TestClass13 {
    @JSONRename("name9")
    name1: string = "TestClass13"
    @JSONRename("nicknametest")
    nickname: string = "class13_nickname"
    nickname2: string = "class13_nickname2"
    nickname3: string = "class13_nickname3"
}

class TestClass14 extends TestClass13 {
    @JSONRename("name14test")
    name10: string = "TestClass14"
    name9: string = "TestClass14"
    nicknametest: string = "class14_nickname"
    nickname3: string = "class14_nickname3"
}

class TestClass15 {
    @JSONRename("record15")
    record: Record<string, Any> = new Record<string, Any>()
    @JSONRename("map15")
    map: Map<string, string> = new Map<string, string>()
    @JSONRename("nestedRecord15")
    nestedRecord: Record<string, Any> = new Record<string, Any>()
    @JSONStringifyIgnore
    i8Array: Int8Array = Int8Array.of(INT8_MAX, INT8_MIN, 100, INT8_MIN + 1)
    @JSONRename("fixArray15")
    fixArray: FixedArray<String> = ["\a", "\b"]
}

function jsonStringify() {
    let obj: TestClass1 = new TestClass1()
    obj.setPassword(123456)
    obj.setUserName("tester")
    obj.setNickName("test123")
    let json = JSON.stringify(obj)
    arktest.assertEQ(json, '{"__backing_user":"tester","nickname":"test123"}')
    let jsonObj: TestClass1 = JSON.parse<TestClass1>(json, Type.of(obj)) as TestClass1
    arktest.assertEQ(Object.keys(jsonObj).length, 3)
    arktest.assertEQ(jsonObj?.user, "tester")
    arktest.assertEQ(jsonObj?.password, 0)
    arktest.assertEQ(jsonObj?.nickname, "")
}

function checkThrow(fn: () => void, expectMsg: string) {
    arktest.expectThrow(fn, (e) => {
        if (e.message === expectMsg) {
            return true
        } else {
            return e.message
        }
    })
}

function jsonThrowWithRename() {
    let obj: TestClass2 = new TestClass2()
    obj.password = 123456
    obj.user = "tester"
    obj.nickname = "test123"
    const expectMsg = "Cannot rename nickname in keys of JsonAnnotationTest.TestClass2"
    checkThrow(() => {JSON.stringify(obj)}, expectMsg)
}

function jsonThrowWithDoubleRename() {
    let obj: TestClass3 = new TestClass3()
    obj.password = 123456
    obj.user = "tester"
    obj.nickname = "test123"
    const expectMsg = "Cannot double rename nickname1 in keys of JsonAnnotationTest.TestClass3"
    checkThrow(() => {JSON.stringify(obj)}, expectMsg)
}

function jsonParseThrow() {
    let obj: TestClass4 = new TestClass4()
    let json1: string = '{"user":"tester","nickname":"test123"}'
    let json2: string = '{"nickname2":"tester","nickname":"test123"}'
    let json3: string = '{"nickname1":"tester","password":123456,"nickname":"test123"}'
    const expectMsg1 = "Cannot find password in keys of JsonAnnotationTest.TestClass4"
    const expectMsg2 = "Cannot find rename nickname1 in keys of JsonAnnotationTest.TestClass4"
    checkThrow(() => {JSON.parse<TestClass4>(json1, Type.of(obj))}, expectMsg1)
    checkThrow(() => {JSON.parse<TestClass4>(json2, Type.of(obj))}, expectMsg2)
    let jsonObj: TestClass4 = JSON.parse<TestClass4>(json3, Type.of(obj)) as TestClass4
    arktest.assertEQ(Object.keys(jsonObj).length, 3)
    arktest.assertEQ(jsonObj?.user, "tester")
    arktest.assertEQ(jsonObj?.password, 123456)
    arktest.assertEQ(jsonObj?.nickname, "test123")
    arktest.assertEQ(Object.hasOwn(jsonObj, "nickname1"), false)
}

function jsonStringifyExtend() {
    let obj6: TestClass6 = new TestClass6()
    let obj7: TestClass7 = new TestClass7()
    let json6 = JSON.stringify(obj6)
    arktest.assertEQ(json6, '{"name1":"class6","nickname":"class6_nickname"}')
    let json7 = JSON.stringify(obj7)
    arktest.assertEQ(json7, '{"name1":"class7","nickname":"class7_nickname"}')
}

function jsonStringifyNest() {
    let obj9: TestClass9 = new TestClass9()
    let obj8: TestClass8 = new TestClass8()
    let json9 = JSON.stringify(obj9)
    arktest.assertEQ(json9, '{"name1":"TestClass9","nickname":{"name8":"TestClass8","nickname":"class8_nickname"}}')
    let jsonObj: TestClass9 = JSON.parse<TestClass9>(json9, Type.from<TestClass9>()) as TestClass9
    arktest.assertEQ(Object.keys(jsonObj).length, 2)
    arktest.assertEQ(jsonObj.name1, "TestClass9")
    arktest.assertEQ(Type.of(jsonObj.nickname).equals(Type.of(obj8)), true)
    let obj3 = (jsonObj.nickname) as TestClass8
    arktest.assertEQ(Object.hasOwn(obj3, "name8"), false)
    arktest.assertEQ(obj3.name1, "TestClass8")
    arktest.assertEQ(obj3.nickname, "class8_nickname")
}


function jsonThrowWithExtends() {
    let obj10: TestClass10 = new TestClass10()
    let obj11: TestClass11 = new TestClass11()
    const expectMsg1 = "Cannot double rename name8 in keys of JsonAnnotationTest.TestClass10"
    const expectMsg2 = "Cannot rename name1 in keys of JsonAnnotationTest.TestClass11"
    checkThrow(() => {JSON.stringify(obj10)}, expectMsg1)
    checkThrow(() => {JSON.stringify(obj11)}, expectMsg2)
}

function jsonTestWithExtends() {
    let obj: TestClass14 = new TestClass14()
    const targetOutput = '{"name9":"TestClass14","nicknametest":"class14_nickname",' +
        '"nickname2":"class13_nickname2","nickname3":"class14_nickname3","name14test":"TestClass14"}'
    let json = JSON.stringify(obj)
    arktest.assertEQ(json, targetOutput)
}

function jsonTestWithTypes() {
    let obj: TestClass15 = new TestClass15()
    obj.record["name"] = "arkts"
    obj.record["version"] = 1
    obj.map.set("key1", "value1")
    obj.nestedRecord = { "name": "arkts", "version": 1}
    const nest: Record<string, Any> = { "desc": "nested" }
    obj.nestedRecord["nested"] = nest
    let json = JSON.stringify(obj)
    const targetOutput = '{"record15":{"name":"arkts","version":1},"map15":{},' +
        '"nestedRecord15":{"name":"arkts","version":1,"nested":{"desc":"nested"}},"fixArray15":["a","\\b"]}'
    arktest.assertEQ(json, targetOutput)
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Json stringify && parse for annotation tests ")
    suite.addTest("JSON.stringify test with annotaion", jsonStringify)
    suite.addTest("JSON throw test with annotaion", jsonThrowWithRename)
    suite.addTest("JSON throw test with double annotaion", jsonThrowWithDoubleRename)
    suite.addTest("JSON parse throw test", jsonParseThrow)
    suite.addTest("JSON stringify extends test", jsonStringifyExtend)
    suite.addTest("JSON stringify with nest field test", jsonStringifyNest)
    suite.addTest("JSON throw test with extends", jsonThrowWithExtends)
    suite.addTest("JSON test with extends", jsonTestWithExtends)
    suite.addTest("JSON test with different types", jsonTestWithTypes)
    return suite.run()
}
