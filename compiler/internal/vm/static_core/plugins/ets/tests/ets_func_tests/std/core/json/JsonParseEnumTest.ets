/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

enum EnumTestIntegar {
    e1 = 2.0,
    e2 = 1.23,
    e3 = 5.1,
}

enum EnumTestStr {
    s1 = 'abc',
    s2 = 'def',
}

class TestIntegar {
    t1:EnumTestIntegar = EnumTestIntegar.e1
    t2:EnumTestStr = EnumTestStr.s1
}

class TestStr {
    t1: TestIntegar = new TestIntegar()
    t2: EnumTestStr = EnumTestStr.s2    
}

function jsonParseEnumForClass(): void {
    let integarObj = new TestIntegar()
    let strObj = new TestStr()
    integarObj.t1 = EnumTestIntegar.e2
    integarObj.t2 = EnumTestStr.s2
    strObj.t1.t1 = EnumTestIntegar.e3
    let integarJson = JSON.stringify(integarObj)
    let strJson = JSON.stringify(strObj)
    let parsedIntegarObj = JSON.parse<TestIntegar>(integarJson, Class.from<TestIntegar>()) as TestIntegar
    let parsedStrObj = JSON.parse<TestStr>(strJson, Class.from<TestStr>()) as TestStr
    arktest.assertEQ(parsedIntegarObj.t1, EnumTestIntegar.e2)
    arktest.assertEQ(parsedIntegarObj.t2, EnumTestStr.s2)
    arktest.assertEQ(parsedStrObj.t1.t1, EnumTestIntegar.e3)
    arktest.assertEQ(parsedStrObj.t1.t2, EnumTestStr.s1)
    arktest.assertEQ(parsedStrObj.t2, EnumTestStr.s2)
}

function jsonParseForPureEnum(): void {
    let enumIntJson = JSON.stringify(EnumTestIntegar.e1)
    let enumStrJson = JSON.stringify(EnumTestStr.s1)
    let parsedEnumInt = JSON.parse<EnumTestIntegar>(enumIntJson, Class.from<EnumTestIntegar>()) as EnumTestIntegar
    let parsedEnumStr = JSON.parse<EnumTestStr>(enumStrJson, Class.from<EnumTestStr>()) as EnumTestStr
    arktest.assertEQ(parsedEnumInt, EnumTestIntegar.e1)
    arktest.assertEQ(parsedEnumStr, EnumTestStr.s1)
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Json stringify for ArrayLike tests ')
    suite.addTest('JSON.parse for Class with Enum', jsonParseEnumForClass);
    suite.addTest('JSON.parse for pure Enum', jsonParseForPureEnum);

    return suite.run()
}