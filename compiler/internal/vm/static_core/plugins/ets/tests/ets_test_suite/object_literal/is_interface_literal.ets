/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

interface TestIface {
    str: string
    num: number
    bl: boolean
}

interface TestIfaceArr {
    strs: string[]
    nums: number[]
    bls: boolean[]
}

interface TestIfaceObj {
    obj1: Object
    obj2: Object
}

interface TestIfaceGeneric<T> {
    a: T
}

class TestIfaceGenericImpl<T> implements TestIfaceGeneric<T> {
    constructor(a: T) {
        this.a_ = a
    }

    public get a(): T { return this.a_ }
    public set a(a: T) { this.a_ = a }

    private a_: T
}

class TestIfaceImplObj implements TestIfaceObj {
    constructor(obj1: Object, obj2: Object) {
        this.obj1_ = obj1;
        this.obj2_ = obj2;
    }

    public get obj1(): Object { return this.obj1_ }
    public set obj1(obj1: Object) { this.obj1_ = obj1 }
    public get obj2(): Object { return this.obj2_ }
    public set obj2(obj2: Object) { this.obj2_ = obj2 }

    private obj1_: Object
    private obj2_: Object
}

class TestIfaceImplArr implements TestIfaceArr {
    constructor(strs: string[], nums: number[], bls: boolean[]) {
        this.strs_ = strs
        this.nums_ = nums
        this.bls_ = bls
    }

    public get strs(): string[] { return this.strs_ }
    public get nums(): number[] { return this.nums_ }
    public set strs(strs: string[]) { this.strs_ = strs }
    public set nums(nums: number[]) { this.nums_ = nums }
    public get bls(): boolean[] { return this.bls_ }
    public set bls(bls: boolean[]) { this.bls_ = bls }

    private strs_: string[]
    private nums_: number[]
    private bls_: boolean[]
}

class TestIfaceImpl implements TestIface {
    constructor(str: string, num: number, bl: boolean) {
        this.str_ = str
        this.num_ = num
        this.bl_ = bl
    }

    public get str(): string { return this.str_ }
    public get num(): number { return this.num_ }
    public set str(str: string) { this.str_ = str }
    public set num(num: number) { this.num_ = num }
    public get bl(): boolean { return this.bl_ }
    public set bl(bl: boolean) { this.bl_ = bl }

    private str_: string
    private num_: number
    private bl_: boolean
}

function testIfaceLiteralInit() {
    let tif: TestIface = { str: "strstr", num: 42, bl: false }
    arktest.assertEQ(reflect.isLiteralInitializedInterface(tif), true)
}

function testIfaceNonLiteralInit() {
    let tif: TestIface = new TestIfaceImpl("strstr", 42, false)
    arktest.assertEQ(reflect.isLiteralInitializedInterface(tif), false)
}

function testIfaceLiteralInitArr() {
    let tif: TestIfaceArr = { strs: ["strstr", "abaaba"], nums: [42, 24], bls: [false, true] }
    arktest.assertEQ(reflect.isLiteralInitializedInterface(tif), true)
}

function testIfaceNonLiteralInitArr() {
    let tif: TestIfaceArr = new TestIfaceImplArr(["strstr", "abaaba"], [42, 24], [false, true])
    arktest.assertEQ(reflect.isLiteralInitializedInterface(tif), false)
}

function testIfaceLiteralInitObj() {
    let tif: TestIfaceObj = { obj1: {}, obj2: {} }
    arktest.assertEQ(reflect.isLiteralInitializedInterface(tif), true)
}

function testIfaceNonLiteralInitObj() {
    let tif: TestIfaceImplObj = new TestIfaceImplObj(new Object(), new Object())
    arktest.assertEQ(reflect.isLiteralInitializedInterface(tif), false)
}

function testIfaceLiteralInitGeneric() {
    let tif: TestIfaceGeneric<int> = {a: 42}
    arktest.assertEQ(reflect.isLiteralInitializedInterface(tif), true)
}

function testIfaceNonLiteralInitGeneric() {
    let tif: TestIfaceGeneric<int> = new TestIfaceGenericImpl<int>(42)
    arktest.assertEQ(reflect.isLiteralInitializedInterface(tif), false)
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Interface literal initialization tests")
    suite.addTest("Positive test simple", testIfaceLiteralInit)
    suite.addTest("Negative test simple", testIfaceNonLiteralInit)
    suite.addTest("Positive test array", testIfaceLiteralInitArr)
    suite.addTest("Negative test array", testIfaceNonLiteralInitArr)
    suite.addTest("Positive test object", testIfaceLiteralInitObj)
    suite.addTest("Negative test object", testIfaceNonLiteralInitObj)
    suite.addTest("Positive test generic", testIfaceLiteralInitGeneric)
    suite.addTest("Negative test generic", testIfaceNonLiteralInitGeneric)
    return suite.run()
}
