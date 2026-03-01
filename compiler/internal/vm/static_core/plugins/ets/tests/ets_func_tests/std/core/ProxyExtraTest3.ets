/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

type Dimension = PX | VP | FP | LPX | Percentage | Resource | undefined

interface Resource {
   bundleName: string
   moudleName: string
   id: number
   params?: Any[]
   type?: number
   resource?: ResourceString

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


interface A {
    a1?: Array<string>
    a2?: Array<Resource>
    a3: Array<Dimension>
}

class AImpl implements A {
    a1?: Array<string>
    a2?: Array<Resource>
    a3: Array<Dimension> = []
}

interface B {
    a1?: Map<string, string>
    a2?: Map<string, Resource>
    a3: Map<string, Dimension>
}

class BImpl implements B {
    a1?: Map<string, string>
    a2?: Map<string, Resource>
    a3: Map<string, Dimension> = new Map<string, Dimension>()
}

class AHandler implements reflect.InvocationHandler {
    instance: A
    constructor(other: A) {
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

class BHandler implements reflect.InvocationHandler {
    instance: B
    constructor(other: B) {
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


// Test Array
function createAProxyTest(){
    let ifaces: FixedArray<Class> =  [ Class.from<A>() ]
    let proxy1: A = createProxy(ifaces, new AHandler(new AImpl())) as A
    createAProxyTestBody(proxy1)

    let proxy2: A = createProxy(ifaces, new AHandler(new AImpl())) as A
    createAProxyTestBody(proxy2)
}

function createAProxyTestBody(proxy: A) {
    let a1 = proxy.a1
    arktest.assertEQ(proxy.a1, undefined)
    a1 = new Array<string>()
    a1.push("test1")
    a1.push("test2")
    arktest.assertEQ(a1[0], "test1")
    arktest.assertEQ(a1[1], "test2")
    a1 = undefined

    let a2 = proxy.a2
    arktest.assertEQ(proxy.a2, undefined)
    a2 = new Array<Resource>()
    a2.push(new ResourceImpl("test1", "test2", 1))
    a2.push(new ResourceImpl("test2", "test1", 2))
    arktest.assertEQ(a2[0].id, 1)
    arktest.assertEQ(a2[1].id, 2)
    a2 = undefined

    let a3 = proxy.a3
    arktest.assertEQ(a3.length, 0)
    let d1: Dimension = "test1"
    a3.push(d1)
    let d2: Dimension = 1
    a3.push(d2)
    let d3: Dimension = new ResourceImpl("test1", "test2", 3)
    a3.push(d3)
    a3.push(undefined)
    arktest.assertEQ(a3[0], "test1")
    arktest.assertEQ(a3[1], 1)
    arktest.assertEQ((a3[2] as Resource).id, 3)
    arktest.assertEQ(a3[3], undefined)
}

function createBProxyTest(){
    let ifaces: FixedArray<Class> =  [ Class.from<B>() ]
    let proxy1: B = createProxy(ifaces, new BHandler(new BImpl())) as B
    createBProxyTestBody(proxy1)

    let proxy2: B = createProxy(ifaces, new BHandler(new BImpl())) as B
    createBProxyTestBody(proxy2)
}

// Test Map
function createBProxyTestBody(proxy: B) {
    let a1 = proxy.a1
    arktest.assertEQ(proxy.a1, undefined)
    a1 = new Map<string, string>
    a1.set("test1", "test2")
    a1.set("test3", "test4")
    arktest.assertEQ(a1.get("test1"), "test2")
    arktest.assertEQ(a1.get("test3"), "test4")
    a1 = undefined

    let a2 = proxy.a2
    arktest.assertEQ(proxy.a2, undefined)
    a2 = new Map<string, Resource>
    a2.set("test1", new ResourceImpl("test1", "test2", 1))
    a2.set("test2", new ResourceImpl("test2", "test1", 2))
    arktest.assertEQ(a2.get("test1")?.id, 1)
    arktest.assertEQ(a2.get("test2")?.id, 2)
    a2 = undefined

    let a3 = proxy.a3
    arktest.assertEQ(a3.size, 0)
    let d1: Dimension = "test1"
    a3.set("test1", d1)
    let d2: Dimension = 1
    a3.set("test2", d2)
    let d3: Dimension = new ResourceImpl("test1", "test2", 3)
    a3.set("test3", d3)
    a3.set("test4", undefined)
    arktest.assertEQ(a3.get("test1"), "test1")
    arktest.assertEQ(a3.get("test2"), 1)
    arktest.assertEQ((a3.get("test3") as Resource).id, 3)
    arktest.assertEQ(a3.get("test4"), undefined)
}


function main() {
    const suite = new arktest.ArkTestsuite('Proxy test')
    suite.addTest('Test proxy opt array', createAProxyTest)
    suite.addTest('Test proxy opt map', createBProxyTest)
    return suite.run()
}
