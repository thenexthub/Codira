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
}
class ProxyTestImpl<Object> implements ProxyTest<Object>{
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
    a12: BaseImpl | Object = {}
    a13?: ResourceString
    a14?: Dimension
    a15?: SymbolGlyphModifier
    a16?: Array<number>
    a17?: Map<number, string>
}

class ResourceHandler implements reflect.InvocationHandler {
    instance: Resource
    constructor(other: Resource) {
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

class BaseHandler implements reflect.InvocationHandler {
    instance: Base
    constructor(other: Base) {
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

class TestHandler1 implements reflect.InvocationHandler {
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        LATEST_EVENT = Event.GETTER_CALLED
        return true
    }

    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        LATEST_EVENT = Event.SETTER_CALLED
    }

    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        LATEST_EVENT = Event.METHOD_CALLED
        return method.invoke(proxy, args)
    }
}

class TestHandler2 implements reflect.InvocationHandler {
    instance: AttributeModifier<SymbolGlyphAttribute>
    constructor(other: AttributeModifier<SymbolGlyphAttribute>) {
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

class ProxyTestHandler implements reflect.InvocationHandler {
    instance: ProxyTestImpl<Object>
    constructor(other: ProxyTestImpl<Object>) {
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




// Test Resource
function createResourceProxyTest(){
    let ifaces: FixedArray<Class> =  [ Class.from<Resource>() ]
    let proxy1: Resource = createProxy(ifaces, new ResourceHandler(new ResourceImpl("test1", "test2", 1))) as Resource
    createResourceProxyTestBody(proxy1)
    LATEST_EVENT = Event.NONE

    let proxy2: Resource = createProxy(ifaces, new ResourceHandler(new ResourceImpl("test1", "test2", 1))) as Resource
    createResourceProxyTestBody(proxy2)
    LATEST_EVENT = Event.NONE
}

function createResourceProxyTestBody(proxy: Resource) {
    arktest.assertEQ(LATEST_EVENT, Event.NONE)
    proxy.bundleName
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.bundleName = "test1"
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.moudleName
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.moudleName = "test2"
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.id
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.id = 1
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.params
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.params = new Array<Any>()
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.type
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.type = 1
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.type = undefined
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.resource
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.resource = "test"
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.resource = new ResourceImpl("test1", "test2", 1)
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
}

// Test Dimension
function createDimensionProxyTest(){
    let ifaces: FixedArray<Class> =  [ Class.from<DimensionI>() ]
    let proxy1: DimensionI = createProxy(ifaces, new TestHandler1()) as DimensionI
    createDimensionProxyTestBody(proxy1)
    LATEST_EVENT = Event.NONE

    let proxy2: DimensionI = createProxy(ifaces, new TestHandler1()) as DimensionI
    createDimensionProxyTestBody(proxy2)
    LATEST_EVENT = Event.NONE
}

function createDimensionProxyTestBody(proxy: DimensionI) {
    arktest.assertEQ(LATEST_EVENT, Event.NONE)
    proxy.dimension = "test"
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    LATEST_EVENT = Event.NONE
    proxy.dimension = new ResourceImpl("test1", "test2", 1)
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.dimension
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
}

// Test SymbolGlyphModifier
function createModifiernProxyTest(){
    let ifaces: FixedArray<Class> =  [ Class.from<AttributeModifier<SymbolGlyphAttribute>>() ]
    let proxy1: AttributeModifier<SymbolGlyphAttribute> = createProxy(ifaces, new TestHandler2(new SymbolGlyphModifier())) as AttributeModifier<SymbolGlyphAttribute>
    createModifierProxyTestBody(proxy1)
    LATEST_EVENT = Event.NONE

    let proxy2: AttributeModifier<SymbolGlyphAttribute> = createProxy(ifaces, new TestHandler2(new SymbolGlyphModifier())) as AttributeModifier<SymbolGlyphAttribute>
    createModifierProxyTestBody(proxy2)
    LATEST_EVENT = Event.NONE
}

function createModifierProxyTestBody(proxy: AttributeModifier<SymbolGlyphAttribute>) {
    arktest.assertEQ(LATEST_EVENT, Event.NONE)
    proxy.applyNormalAttribute(new SymbolGlyphAttribute())
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
}

// Test Base
function createBaseProxyTest(){
    let ifaces: FixedArray<Class> =  [ Class.from<Base>() ]
    let proxy1: Base = createProxy(ifaces, new BaseHandler(new BaseImpl())) as Base
    createBaseProxyTestBody(proxy1)
    LATEST_EVENT = Event.NONE

    let proxy2: Base = createProxy(ifaces, new BaseHandler(new BaseImpl())) as Base
    createBaseProxyTestBody(proxy2)
    LATEST_EVENT = Event.NONE
}

function createBaseProxyTestBody(proxy: Base) {
    arktest.assertEQ(LATEST_EVENT, Event.NONE)
    let id: number = proxy.getId() as number
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertEQ(id, 0)
}

// Test ProxyTest
function createProxyTest(){
    let ifaces: FixedArray<Class> =  [ Class.from<ProxyTest<Object>>() ]
    let proxy1: ProxyTest<Object> = createProxy(ifaces, new ProxyTestHandler(new ProxyTestImpl<Object>())) as ProxyTest<Object>
    createProxyTestBody(proxy1)
    LATEST_EVENT = Event.NONE

    let proxy2: ProxyTest<Object> = createProxy(ifaces, new ProxyTestHandler(new ProxyTestImpl<Object>())) as ProxyTest<Object>
    createProxyTestBody(proxy2)
    LATEST_EVENT = Event.NONE
}

function createProxyTestBody(proxy: ProxyTest<Object>) {
    arktest.assertEQ(LATEST_EVENT, Event.NONE)
    proxy.a1
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a1 = 1.0
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    arktest.assertEQ(proxy.a1, 1.0)
    proxy.a2
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a2 = 1
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    arktest.assertEQ(proxy.a2, 1)
    proxy.a3
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a3 = 1
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    arktest.assertEQ(proxy.a3, 1)
    proxy.a4
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a4 = 1
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    arktest.assertEQ(proxy.a4, 1)
    proxy.a5
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a5 = c'A'
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    arktest.assertEQ(proxy.a5, c'A')
    proxy.a6
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a6 = 1.0
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    arktest.assertEQ(proxy.a6, 1.0)
    proxy.a7
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a7 = "test"
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    arktest.assertEQ(proxy.a7, "test")
    proxy.a8
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a8 = 1.0
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    arktest.assertEQ(proxy.a8, 1.0)
    proxy.a9
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a9 = 1
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    arktest.assertEQ(proxy.a9, 1)
    proxy.a10
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a10 = "test"
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    arktest.assertEQ(proxy.a10, "test")
    proxy.a10 = 1.0
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.a11
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a11 = new BaseImpl()
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.a11 = 1.0
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    arktest.assertEQ(proxy.a11, 1.0)
    proxy.a12
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a12 = new BaseImpl()
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.a12 = new Object()
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.a13
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a13 = "test"
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    arktest.assertEQ(proxy.a13, "test")
    proxy.a13 = new ResourceImpl("test1", "test2", 1)
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    arktest.assertEQ((proxy.a13 as Resource).bundleName, "test1")
    proxy.a14
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a14 = "test"
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.a14 = 1
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    arktest.assertEQ(proxy.a14, 1)
    proxy.a14 = new ResourceImpl("test1", "test2", 1)
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.a15
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a15 = undefined
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.a15 = new SymbolGlyphModifier()
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.a16
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a16 = undefined
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.a16 = new Array<number>()
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.a17
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    proxy.a17 = undefined
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.a17 = new Map<number, string>
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.a17!.set(1.0, "one")
}

// Test ProxyTest %%get- and %%set-
function createProxyTestOpt(){
    let ifaces: FixedArray<Class> =  [ Class.from<ProxyTest<Object>>() ]
    let proxy1: ProxyTest<Object> = createProxy(ifaces, new ProxyTestHandler(new ProxyTestImpl<Object>())) as ProxyTest<Object>
    createProxyTestBody(proxy1)
    LATEST_EVENT = Event.NONE

    let proxy2: ProxyTest<Object> = createProxy(ifaces, new ProxyTestHandler(new ProxyTestImpl<Object>())) as ProxyTest<Object>
    createProxyTestBody(proxy2)
    LATEST_EVENT = Event.NONE
}

function createProxyTestOptBody(proxy: ProxyTest<Object>) {
    for (let i = 0; i < 200; i++) {
        arktest.assertEQ(LATEST_EVENT, Event.NONE)
        proxy.a1
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a1 = 1.0
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        arktest.assertEQ(proxy.a1, 1.0)
        proxy.a2
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a2 = 1
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        arktest.assertEQ(proxy.a2, 1)
        proxy.a3
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a3 = 1
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        arktest.assertEQ(proxy.a3, 1)
        proxy.a4
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a4 = 1
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        arktest.assertEQ(proxy.a4, 1)
        proxy.a5
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a5 = c'A'
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        arktest.assertEQ(proxy.a5, c'A')
        proxy.a6
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a6 = 1.0
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        arktest.assertEQ(proxy.a6, 1.0)
        proxy.a7
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a7 = "test"
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        arktest.assertEQ(proxy.a7, "test")
        proxy.a8
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a8 = 1.0
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        arktest.assertEQ(proxy.a8, 1.0)
        proxy.a9
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a9 = 1
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        arktest.assertEQ(proxy.a9, 1)
        proxy.a10
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a10 = "test"
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        arktest.assertEQ(proxy.a10, "test")
        proxy.a10 = 1.0
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        proxy.a11
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a11 = new BaseImpl()
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        proxy.a11 = 1.0
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        arktest.assertEQ(proxy.a11, 1.0)
        proxy.a12
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a12 = new BaseImpl()
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        proxy.a12 = new Object()
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        proxy.a13
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a13 = undefined
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        proxy.a13 = "test"
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        arktest.assertEQ(proxy.a13, "test")
        proxy.a13 = new ResourceImpl("test1", "test2", 1)
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        arktest.assertEQ((proxy.a13 as Resource).bundleName, "test1")
        proxy.a14
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a14 = undefined
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        proxy.a14 = "test"
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        proxy.a14 = 1
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        arktest.assertEQ(proxy.a14, 1)
        proxy.a14 = new ResourceImpl("test1", "test2", 1)
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        proxy.a15
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a15 = undefined
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        proxy.a15 = new SymbolGlyphModifier()
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        proxy.a16
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a16 = undefined
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        proxy.a16 = new Array<number>()
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        proxy.a17
        arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
        proxy.a17 = undefined
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        proxy.a17 = new Map<number, string>
        arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
        proxy.a17!.set(1.0, "one")
    }
}



function main() {
    const suite = new arktest.ArkTestsuite('Proxy test')
    suite.addTest('Test create Resource proxyClass', createResourceProxyTest)
    suite.addTest('Test create Dimension proxyClass', createDimensionProxyTest)
    suite.addTest('Test create SymbolGlyphModifier proxyClass', createModifiernProxyTest)
    suite.addTest('Test create Base proxyClass', createBaseProxyTest)
    suite.addTest('Test create proxyClass', createProxyTest)
    suite.addTest('Test create proxyClass opt', createProxyTestOpt)
    return suite.run()
}
