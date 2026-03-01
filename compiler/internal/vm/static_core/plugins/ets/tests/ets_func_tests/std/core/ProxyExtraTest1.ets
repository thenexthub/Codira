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

enum Event {
    NONE,
    GETTER_CALLED,
    SETTER_CALLED,
    METHOD_CALLED,
}

let LATEST_EVENT: Event = Event.NONE

function createProxy(ifaces: FixedArray<Class>, handle: reflect.InvocationHandler) : reflect.Proxy {
    let linker = Class.current().getLinker()
    return reflect.Proxy.create(linker, ifaces, handle)
}

type ResourceString = Resource | string

type PX = string

type VP = string | number

type FP = string

type LPX = string

type Percentage = string

type Degree = string

type Dimension = PX | VP | FP | LPX | Percentage | Resource

interface Resource {
   bundleName: string
   moudleName: string
   id: number
   params?: Any[]
   type?: number
   resource?: ResourceString

}

interface DimensionI {
    dimension?: Dimension
}

class SymbolGlyphAttribute {
    soo(): int {
        return 1
    }
}

interface AttributeModifier<T> {
    modify(attribute: T): void;
    reset(attribute: T): void;
    applyNormalAttribute(instance: SymbolGlyphAttribute): void;
}

class SymbolGlyphModifier extends SymbolGlyphAttribute implements AttributeModifier<SymbolGlyphAttribute> {

    constructor(src?: Resource){

    }

    applyNormalAttribute(instance: SymbolGlyphAttribute): void {

    }
    modify(attribute: SymbolGlyphAttribute): void {

    }
    reset(attribute: SymbolGlyphAttribute): void {

    }
}

class ResourceImpl implements Resource {
    bundleName: string;
    moudleName: string;
    id: number;
    params?: Any[];
    type?: number;
    resource?: ResourceString;
    constructor(bundleName: string, moudleName: string, id: number) {
        this.bundleName = bundleName;
        this.moudleName = moudleName;
        this.id = id;
    }
}

interface Base {
    _id: number

    getId(): number {
        return this._id
    }
}

class BaseImpl implements Base{
    _id: number = 0
}

interface ProxyTest<T> {
    a1: number
    a2: int
    a3: short
    a4: byte
    a5: char
    a6: double
    a7: string
    a8: float
    a9: long
    a10: string | number
    a11: BaseImpl | number
    a12: BaseImpl | T
    a13?: ResourceString
    a14?: Dimension
    a15?: SymbolGlyphModifier
    a16?: Array<number>
    a17?: Map<number, string>

    testGet1(): number {
        return this.a1
    }
    testGet2(): int {
        return this.a2
    }
    testGet3(): short {
        return this.a3
    }
    testGet4(): byte {
        return this.a4
    }
    testGet5(): char {
        return this.a5
    }
    testGet6(): double {
        return this.a6
    }
    testGet7(): string {
        return this.a7
    }
    testGet8(): float {
        return this.a8
    }
    testGet9(): long {
        return this.a9
    }
    testGet10(): string | number {
        return this.a10
    }
    testGet11(): BaseImpl | number {
        return this.a11
    }
    testGet12(): BaseImpl | T {
        return this.a12
    }
    testGet13(): ResourceString | undefined {
        return this.a13
    }
    testGet14(): Dimension | undefined {
        return this.a14
    }
    testGet15(): SymbolGlyphModifier | undefined {
        return this.a15
    }
    testGet16(): Array<number> | undefined {
        return this.a16
    }
    testGet17(): Map<number, string> | undefined {
        return this.a17
    }

    testSet1(d: number): void {
        this.a1 = d
    }
    testSet2(d: int): void {
        this.a2 = d
    }
    testSet3(d: short): void {
        this.a3 = d
    }
    testSet4(d: byte): void {
        this.a4 = d
    }
    testSet5(d: char): void {
        this.a5 = d
    }
    testSet6(d: double): void {
        this.a6 = d
    }
    testSet7(d: string): void {
        this.a7 = d
    }
    testSet8(d: float): void {
        this.a8 = d
    }
    testSet9(d: long): void {
        this.a9 = d
    }
    testSet10(d: string | number): void {
        this.a10 = d
    }
    testSet11(d: BaseImpl | number): void {
        this.a11 = d
    }
    testSet12(d: BaseImpl | T): void {
        this.a12 = d
    }
    testSet13(d: ResourceString | undefined): void {
        this.a13 = d
    }
    testSet14(d: Dimension | undefined): void {
        this.a14 = d
    }
    testSet15(d: SymbolGlyphModifier | undefined): void {
        this.a15 = d
    }
    testSet16(d: Array<number> | undefined): void {
        this.a16 = d
    }
    testSet17(d: Map<number, string> | undefined): void {
        this.a17 = d
    }
}

class ProxyTestImpl<Resource> implements ProxyTest<Resource>{
    a1: number = 1.0
    a2: int = 2
    a3: short = 3
    a4: byte = 4
    a5: char = c'5'
    a6: double = 6
    a7: string = "7"
    a8: float = 8.0
    a9: long = 8000000000
    a10: string | number = 1
    a11: BaseImpl | number = 1
    a12: BaseImpl | Resource = (new ResourceImpl("test1", "test2", 1)) as Resource
    a13?: ResourceString
    a14?: Dimension
    a15?: SymbolGlyphModifier
    a16?: Array<number>
    a17?: Map<number, string>
}

class BasicHandler implements reflect.InvocationHandler {
    instance: ProxyTest<Resource>
    constructor(other: ProxyTest<Resource>) {
        this.instance = other
    }
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        LATEST_EVENT = Event.GETTER_CALLED
        return method.invoke(this.instance)
    }

    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        LATEST_EVENT = Event.SETTER_CALLED
        method.invoke(this.instance, [value])
    }

    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        LATEST_EVENT = Event.METHOD_CALLED
        return method.invoke(this.instance, args)
    }
}

function exampleDispatchTest() {
    let ifaces: FixedArray<Class> =  [ Class.from<ProxyTest<Resource>>() ]
    let proxy1: ProxyTest<Resource> = createProxy(ifaces, new BasicHandler(new ProxyTestImpl<Resource>())) as ProxyTest<Resource>
    exampleDispatchTestBody(proxy1)
    LATEST_EVENT = Event.NONE

    let proxy2: ProxyTest<Resource> = createProxy(ifaces, new BasicHandler(new ProxyTestImpl<Resource>())) as ProxyTest<Resource>
    exampleDispatchTestBody(proxy2)
    LATEST_EVENT = Event.NONE
}

function exampleDispatchTestBody(proxy: ProxyTest<Resource>) {
    arktest.assertEQ(LATEST_EVENT, Event.NONE)
    proxy.testSet1(1.0)
    let d1: number = proxy.testGet1()
    arktest.assertEQ(d1, 1)
    proxy.testSet2(2)
    let d2: int = proxy.testGet2()
    arktest.assertEQ(d2, 2)
    proxy.testSet3(3)
    let d3: short = proxy.testGet3()
    arktest.assertEQ(d3, 3)
    proxy.testSet4(4)
    let d4: byte = proxy.testGet4()
    arktest.assertEQ(d4, 4)
    proxy.testSet5(c'A')
    let d5: char = proxy.testGet5()
    arktest.assertEQ(d5, c'A')
    proxy.testSet6(5.0)
    let d6: double = proxy.testGet6()
    arktest.assertEQ(d6, 5.0)
    proxy.testSet7("6.0")
    let d7: string = proxy.testGet7()
    arktest.assertEQ(d7, "6.0")
    proxy.testSet8(7.0)
    let d8: float = proxy.testGet8()
    arktest.assertEQ(d8, 7.0)
    proxy.testSet9(80000000000000)
    let d9: long = proxy.testGet9()
    arktest.assertEQ(d9, 80000000000000)
    proxy.testSet10("9")
    let d10: string | number = proxy.testGet10()
    arktest.assertEQ(d10, "9")
    proxy.testSet10(10.0)
    d10 = proxy.testGet10()
    arktest.assertEQ(d10, 10.0)
    proxy.testSet11(10.0)
    let d11: BaseImpl | number = proxy.testGet11()
    arktest.assertEQ(d11, 10.0)
    proxy.testSet12(new BaseImpl())
    let d12: BaseImpl | Resource = proxy.testGet12()
    arktest.assertEQ((d12 as BaseImpl).getId(), 0)
    proxy.testSet12((new ResourceImpl("test1", "test2", 1)) as Resource)
    d12 = proxy.testGet12() as Resource
    arktest.assertEQ((d12 as Resource).bundleName, "test1")
    proxy.testSet13((new ResourceImpl("test1", "test2", 1)) as Resource)
    let d13: ResourceString | undefined = proxy.testGet13()
    arktest.assertEQ((d13 as Resource).moudleName, "test2")
    let d131: ResourceString | undefined = undefined
    d13 = d131
    arktest.assertEQ(d13, undefined)
    proxy.testSet14(10.0)
    let d14: Dimension | undefined = proxy.testGet14()
    arktest.assertEQ(d14 , 10.0)
    proxy.testSet15(new SymbolGlyphModifier())
    let d15: SymbolGlyphModifier | undefined = proxy.testGet15()
    arktest.assertEQ((d15 as SymbolGlyphModifier).soo(), 1)
    proxy.testSet16(new Array<number>())
    let d16: Array<number> | undefined = proxy.testGet16()
    arktest.assertEQ(d16!.length, 0)
    let map: Map<number, string> = new Map<number, string>()
    map.set(1.0, "one")
    proxy.testSet17(map)
    let d17: Map<number, string> | undefined = proxy.testGet17()
    arktest.assertEQ(d17!.get(1.0), "one")
}

function main() {
    const suite = new arktest.ArkTestsuite('Proxy test')
    suite.addTest('Test proxy invoke', exampleDispatchTest)
    return suite.run()
}
