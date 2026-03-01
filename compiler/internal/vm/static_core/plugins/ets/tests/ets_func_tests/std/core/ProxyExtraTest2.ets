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
    a1?: string
    a2?: Resource
    a3: Dimension
    a4: B
}

class AImpl implements A {
    a1?: string
    a2?: Resource
    a3: Dimension = "test"
    a4: B
    constructor(b: B) {
        this.a4 = b
    }
}

interface B {
    a1?: string
    foo (num: number): number | undefined
}

class BImpl implements B {
    a1?: string
    foo (num: number): number | undefined {
        return num
    }
}

class AHandler implements reflect.InvocationHandler {
    instance: A
    constructor(other: A) {
        this.instance = other
    }
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return method.invoke(this.instance)
    }

    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        method.invoke(this.instance, [value])
    }

    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        return method.invoke(this.instance, args)
    }
}

class BHandler implements reflect.InvocationHandler {
    instance: B
    constructor(other: B) {
        this.instance = other
    }
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return method.invoke(this.instance)
    }

    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        method.invoke(this.instance, [value])
    }

    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        return method.invoke(this.instance, args)
    }
}

// Test Array
function createAProxyTest(){
    let ifacesB: FixedArray<Class> =  [ Class.from<B>() ]
    let proxyB1: B = createProxy(ifacesB, new BHandler(new BImpl())) as B
    let ifacesA: FixedArray<Class> =  [ Class.from<A>() ]
    let proxy1: A = createProxy(ifacesA, new AHandler(new AImpl(proxyB1))) as A
    createAProxyTestBody(proxy1)
    let proxyB2: B = createProxy(ifacesB, new BHandler(new BImpl())) as B
    let proxy2: A = createProxy(ifacesA, new AHandler(new AImpl(proxyB2))) as A
    createAProxyTestBody(proxy2)
}

function createAProxyTestBody(proxy: A) {
    let a1 = proxy.a1
    let a2 = proxy.a2
    let a3 = proxy.a3
    let a4 = proxy.a4
    arktest.assertEQ(proxy.a1, undefined)
    a1 = "test1"
    a2 = new ResourceImpl("test1", "test2", 1)
    a3 = "test2"
    arktest.assertEQ(a1, "test1")
    arktest.assertEQ((a2 as Resource).id, 1)
    arktest.assertEQ(a3, "test2")

    a4.a1 = "test3"
    let num = a4.foo(1.0)
    arktest.assertEQ(a4.a1, "test3")
    arktest.assertDoubleEQ(num!, 1.0, 0.00001)
    num = a4.foo(2.1)
    arktest.assertDoubleEQ(num!, 2.1, 0.00001)
}

function main() {
    const suite = new arktest.ArkTestsuite('Proxy test')
    suite.addTest('Test proxy nested scenario', createAProxyTest)
    return suite.run()
}
