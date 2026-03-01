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

class Base {
    private _id: number = 0

    getId(): number {
        return this._id
    }
}

class ObjWithProps extends Base {
    private _name: string = ""

    get name(): string {
        return this._name
    }

    set name(name: string) {
        this._name = name
    }

    isNameSet(): boolean {
        return this._name != ""
    }

    add(name: string): void {
        this._name += "," + name
    }

    add(name1: string, name2: string): void {
        this._name += "," + name1 + "," + name2
    }

    toString(): string {
        return `ObjWithProps: { _name: '${this.name}' }`
    }
}

class ObjProxyHandler extends proxy.DefaultProxyHandler<ObjWithProps> {
    getTrapped: boolean = false
    setTrapped: boolean = false
    getIdTrapped: boolean = false

    get(target: ObjWithProps, name: string): Any {
        this.getTrapped = true

        const value = super.get(target, name)
        if (name == "name") {
            return value + "1"
        } else {
            return value
        }
    }

    set(target: ObjWithProps, name: string, value: Any): boolean {
        this.setTrapped = true
        return super.set(target, name, value)
    }

    invoke(target: ObjWithProps, method: Method, args: FixedArray<Any>): Any {
        const methodName = method.getName()
        if (methodName == "isNameSet") {
            return false
        } else if (methodName == "getId") {
            this.getIdTrapped = true
        }

        return super.invoke(target, method, args)
    }
}

interface ProxyTest {
    a1:number
    a2:int
    a3:short
    a4:byte
    a5:char
    a6:double
    a7:string
    a8:float
    a9:long
}

class interfaceProxyHandler<T extends Object> implements proxy.ProxyHandler<T>{
    public get(target:T, name:string):Any {
        return (reflect.Value.of(target) as ClassValue).getFieldByName(name).getData()
    }

    public set(target:T, name:string, newValue:Any):boolean {
        const targetValue = reflect.Value.of(target) as ClassValue
        const value = targetValue.getFieldByName(name).getData()

        if (value !== newValue) {
            try {
                targetValue.setFieldByName(name, reflect.Value.of(newValue))
            } catch(e) {
                console.error(e)
                return false
            }
        }
        return true
    }

    public invoke(target:T, method:Method, args:FixedArray<Any>):Any {
        return method.invoke(target, args)
    }
}

function verifyObjectProxyOps(obj: ObjWithProps, objProxyHandler: ObjProxyHandler): void {
    arktest.assertFalse(objProxyHandler.getTrapped)
    arktest.assertFalse(objProxyHandler.setTrapped)

    const objProxy = proxy.Proxy.create(obj, objProxyHandler)

    objProxy.name = "foo"
    arktest.assertTrue(objProxyHandler.setTrapped)

    const objName = objProxy.name
    arktest.assertTrue(objProxyHandler.getTrapped)

    arktest.assertEQ(objName,  "foo1")

    arktest.assertTrue(obj.isNameSet())
    arktest.assertFalse(objProxy.isNameSet())

    objProxy.add("bar")
    arktest.assertEQ(objProxy.name, "foo,bar1")

    objProxy.add("baz", "goo")
    arktest.assertEQ(objProxy.name, "foo,bar,baz,goo1")

    const id = objProxy.getId()
    arktest.assertTrue(objProxyHandler.getIdTrapped)

    arktest.assertEQ(proxy.Proxy.tryGetTarget(objProxy), obj)
    arktest.assertEQ(proxy.Proxy.tryGetHandler(objProxy), objProxyHandler)
}

function testObjectProxyOps(): void {
    verifyObjectProxyOps(new ObjWithProps(), new ObjProxyHandler())
}

class Handler extends proxy.DefaultProxyHandler<Map<string, string>> {}
function testStdLibClassesProxying(): void {
    const map = new Map<string, string>()

    const mapProxy = proxy.Proxy.create(map, new Handler())

    arktest.assertTrue(map === mapProxy)
}

function testInterfaceProxying(): void {
    let testObj: ProxyTest = {a1: 100.2, a2: 100, a3: 2, a4: 3, a5: c'4', a6: 100.3, a7:"abc", a8: 100.23f, a9:99999999}
    let proxyObj = proxy.Proxy.create(testObj, new interfaceProxyHandler<ProxyTest>())
    proxyObj.a1 = 200.2
    proxyObj.a2 = 200
    proxyObj.a3 = 3
    proxyObj.a4 = 4
    proxyObj.a5 = c'5'
    proxyObj.a6 = 100.4
    proxyObj.a7 = "def"
    proxyObj.a8 = 200.23f
    proxyObj.a9 = 111111111
    arktest.assertTrue(proxyObj.a1 === 200.2)
    arktest.assertTrue(proxyObj.a2 === 200)
    arktest.assertTrue(proxyObj.a3 === 3)
    arktest.assertTrue(proxyObj.a4 === 4)
    arktest.assertTrue(proxyObj.a5 === c'5')
    arktest.assertTrue(proxyObj.a6 === 100.4)
    arktest.assertTrue(proxyObj.a7 === "def")
    arktest.assertTrue(proxyObj.a8 === 200.23f)
    arktest.assertTrue(proxyObj.a9 === 111111111)
}

interface A {
    a: Base | undefined
}

class InterfaceProxyHandler<T extends Object> extends proxy.DefaultProxyHandler<T> {
    public get(target: T, name: string): Any {
        const value = super.get(target, name)
        return value;
    }

    public set(target: T, name: string, newValue: Any): boolean {
        if (super.get(target, name)  !== newValue) {
            const result = super.set(target, name, newValue);
            return result;
        }
        return true;
    }

    public invoke(target: T, method: Method, args: FixedArray<Any>): Any {
        return method.invoke(target, args);
    }
}

function testUndefinedProxying(): void {
    let testObj: A = { a: new Base() }
    let p = proxy.Proxy.create(testObj, new InterfaceProxyHandler<A>());
    arktest.assertTrue(Type.of(testObj.a).equals(Type.from<Base>()))
    arktest.assertTrue(Type.of(p.a).equals(Type.from<Base>()))
    p.a = undefined
    arktest.assertTrue(p.a == undefined)
    arktest.assertTrue(testObj.a == undefined)
    p.a = new Base()
    arktest.assertTrue(Type.of(testObj.a).equals(Type.from<Base>()))
    arktest.assertTrue(Type.of(p.a).equals(Type.from<Base>()))
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Proxy API")

    suite.addTest("Object proxy operations", testObjectProxyOps)
    suite.addTest("StdLib classes proxying", testStdLibClassesProxying)
    suite.addTest("interface primitive fields proxying", testInterfaceProxying)
    suite.addTest("Union with undefined fields proxying", testUndefinedProxying)

    return suite.run()
}
