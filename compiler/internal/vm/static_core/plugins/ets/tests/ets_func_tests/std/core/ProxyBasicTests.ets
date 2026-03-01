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

class SomeObject {
    constructor(other: int) {
        this.point = other
    }
    point: int = 0
}

interface AIface {
    message: string

    foo(): int
    boo(x: int, y: SomeObject): SomeObject
    goo(): double
}

class A implements AIface {
    message: string = "some default string"

    foo(): int {
        return 111
    }
    boo(x: int, y: SomeObject): SomeObject {
        return new SomeObject(x + y.point)
    }
    goo(): double {
        return 333.333
    }
}

interface BIface {
    message: string

    foo(): int
}

class AB implements AIface, BIface {
    message: string = "some default string"

    foo(): int {
        return 111
    }
    boo(x: int, y: SomeObject): SomeObject {
        return new SomeObject(x + y.point)
    }
    goo(): double {
        return 333.333
    }
}

class Base {}
class Derived extends Base {}

interface CIface {
    foo(b: Base): int
}

interface DIface {
    foo(d: Derived): int
}

class CD implements CIface, DIface {
    override foo(b: Base): int { return 1 }
}

interface EIface {
    foo(): double
}

interface FIface {
    foo(): double
}

class EF implements EIface, FIface {
    foo(): double {
        return 1.2
    }
}

interface GIface {
    foo(): double { return 1 }
}

interface HIface {
    foo(): double { return 1 }
}

class G implements GIface {}
class H implements HIface {}

class EFGH implements EIface, FIface, GIface, HIface {
    foo(): double {
        return 1.2
    }
}

interface IIface extends GIface {
}

interface JIface extends GIface {
}

class IJ implements IIface, JIface {}

interface KIface {
    foo(x: Base): string { return "1" }
}

interface LIface {
    foo(x: Derived): string { return "2" }
}

class KL implements KIface, LIface {
    foo(x: Base): string { return "3" }
}

interface MIface extends IIface, JIface {
    foo(): double { return 11 }
}

class M implements MIface {
    foo(): double { return 12 }
}

interface NIface {
    foo(): string
}

interface OIface {
    foo(): long
}

class N implements NIface {
    foo(): string {
        return "111"
    }
}

class O implements OIface {
    foo(): long {
        return 111
    }
}

interface PIface {
    foo(): string { return "123456" }
}

interface QIface {
    foo(): long { return 123456 }
}

class Q implements QIface {}
class P implements PIface {}

interface RIface {
    foo(): double
}

interface SIface {
    foo(): double { return 4.2 }
}

class RS implements RIface, SIface {}

interface TIface {
    foo(x: Base): double
}

interface UIface {
    foo(x: Derived): double { return -1.1 }
}

class TU implements TIface, UIface {
    // foo(x: Derived) with default implementation here
    foo(x: Base): double { return 2.0 }
}

interface VIface {
    foo(x: Base): double { return 1.2 }
}

interface WIface {
    foo(x: Derived): double
}

class VW implements VIface, WIface {
    // foo(x: Base)  with default implementation here
    // #32316 foo(x: Derived): double { return 2.2 }
}

class ValueHandler extends EmptyHandler {
    getSomeValue(): double {
        return 42.42
    }
}

interface XIface {
    getHandler(): reflect.InvocationHandler
}

class X implements XIface {
    getHandler(): reflect.InvocationHandler {
        return new ValueHandler()
    }
}

interface YIface {
    getHandler(): reflect.InvocationHandler {
        return new ValueHandler()
    }
}

class Y implements YIface {
}

interface ZIface {
    getHandler(): char
}

class Z implements ZIface {
    getHandler(): char {
        return c'f'
    }
}

enum Event {
    NONE,
    GETTER_CALLED,
    SETTER_CALLED,
    METHOD_CALLED,
}

let LATEST_EVENT: Event = Event.NONE

type AcceptableTypes = A | AB | CD | EF | G | H | EFGH | IJ | KL | M | N | O | P | Q | RS | TU | VW | X | Y | Z

class Handler implements reflect.InvocationHandler {
    target: AcceptableTypes

    constructor(other: AcceptableTypes) {
        this.target = other
    }

    setTarget(target: AcceptableTypes) {
        this.target = target
    }

    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        LATEST_EVENT = Event.GETTER_CALLED
        return method.invoke(this.target)
    }

    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        LATEST_EVENT = Event.SETTER_CALLED
        method.invoke(this.target, [value])
    }

    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        LATEST_EVENT = Event.METHOD_CALLED
        return method.invoke(this.target, args)
    }
}

class EmptyHandler implements reflect.InvocationHandler {
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return true
    }

    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
    }

    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        return true
    }
}

function checkIsProxy(proxyObj: reflect.Proxy) {
    arktest.assertTrue(reflect.Proxy.isProxyClass(Class.of(proxyObj)))
}

function testGetHandler(proxyHandler: reflect.InvocationHandler, proxy: reflect.Proxy, otherHandler: reflect.InvocationHandler) {
    arktest.assertEQ(proxy.getHandler(), proxyHandler)
    arktest.assertNE(proxy.getHandler(), otherHandler)
}

function testEmpty() {
    let stubObj = new A()
    let handler = new Handler(stubObj)
    let proxyInstance = reflect.Proxy.create(Class.of(stubObj).getLinker(), [], handler)
    checkIsProxy(proxyInstance)

    testGetHandler(handler, proxyInstance, new Handler(stubObj))
}

function testInvokeAIface(proxyObj: AIface) {
    let fooRes = proxyObj.foo()
    arktest.assertTrue(fooRes instanceof int)
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertEQ(fooRes, 111)
    LATEST_EVENT = Event.NONE

    let booArg1 = 10
    let someObjectArg = 115
    let booArgs2 = new SomeObject(someObjectArg)
    let booRes = proxyObj.boo(booArg1, booArgs2)
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(booRes instanceof SomeObject)
    arktest.assertEQ(booRes.point, booArg1 + someObjectArg)
    LATEST_EVENT = Event.NONE

    let gooRes = proxyObj.goo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(gooRes instanceof double)
    arktest.assertEQ(gooRes, 333.333)
    LATEST_EVENT = Event.NONE
}

function testSetGetAIface(proxyObj: AIface) {
    let newMessage = "new message"
    proxyObj.message = newMessage
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    LATEST_EVENT = Event.NONE

    let getRes = proxyObj.message
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    arktest.assertEQ(getRes, newMessage)
    LATEST_EVENT = Event.NONE
}

function testAIface() {
    let obj = new A()
    let linker = Class.of(obj).getLinker()
    let aIfaceClass = Class.from<AIface>()
    let interfaces: FixedArray<Class> = [ aIfaceClass ]
    let handler = new Handler(obj)
    let proxyInstance = reflect.Proxy.create(linker, interfaces, handler)
    let proxyAIface = proxyInstance as AIface

    checkIsProxy(proxyInstance)

    testGetHandler(handler, proxyInstance, new Handler(new A))
    testInvokeAIface(proxyAIface)
    testSetGetAIface(proxyAIface)
}

function testInconsistentInput() {
    let obj = new A
    let linker = Class.of(obj).getLinker()
    let handler = new Handler(obj)

    let interfaces1: FixedArray<Class> = [ Class.from<AIface | BIface>() ]
    const errorcheck1 = (e: Error) => {
        arktest.assertTrue(e instanceof TypeError)
        return e.message === "All given to Proxy classes must be an interface. But given class 'ULProxyBasicTests.AIface;LProxyBasicTests.BIface; is not an interface"
    }
    arktest.expectThrow((): void => { let proxyInstance = reflect.Proxy.create(linker, interfaces1, handler) }, errorcheck1)

    let interfaces2: FixedArray<Class> = [ Class.from<BIface>(), Class.from<string>(), Class.from<AIface>() ]
    const errorcheck2 = (e: Error) => {
        arktest.assertTrue(e instanceof TypeError)
        return e.message === "All given to Proxy classes must be an interface. But given class 'std.core.String is not an interface"
    }
    arktest.expectThrow((): void => { let proxyInstance = reflect.Proxy.create(linker, interfaces2, handler) }, errorcheck2)
}

function testInvokeBIface(proxyObj: BIface) {
    let fooRes = proxyObj.foo()
    arktest.assertTrue(fooRes instanceof int)
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertEQ(fooRes, 111)
    LATEST_EVENT = Event.NONE
}

function testSetGetBIface(proxyObj: BIface) {
    let newMessage = "new message"
    proxyObj.message = newMessage
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    LATEST_EVENT = Event.NONE

    let getRes = proxyObj.message
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
    arktest.assertEQ(getRes, newMessage)
    LATEST_EVENT = Event.NONE
}

function testABProxyClass() {
    let obj = new AB()
    let linker = Class.of(obj).getLinker()
    let interfaces1: FixedArray<Class> = [ Class.from<AIface>(), Class.from<BIface>() ]
    let handler1 = new Handler(obj)
    let proxyInstance1 = reflect.Proxy.create(linker, interfaces1, handler1)

    checkIsProxy(proxyInstance1)
    testGetHandler(handler1, proxyInstance1, new Handler(new AB))
    testInvokeAIface(proxyInstance1 as AIface)
    testSetGetAIface(proxyInstance1 as AIface)
    testInvokeBIface(proxyInstance1 as BIface)
    testSetGetBIface(proxyInstance1 as BIface)

    let interfaces2: FixedArray<Class> = [ Class.from<BIface>(), Class.from<AIface>() ]
    let handler2 = new Handler(obj)
    let proxyInstance2 = reflect.Proxy.create(linker, interfaces2, handler2)

    checkIsProxy(proxyInstance2)
    testGetHandler(handler2, proxyInstance2, new Handler(new AB))
    testInvokeBIface(proxyInstance2 as BIface)
    testSetGetBIface(proxyInstance2 as BIface)
    testInvokeAIface(proxyInstance2 as AIface)
    testSetGetAIface(proxyInstance2 as AIface)
}

function testInvokeCIface(proxyObj: CIface) {
    let fooRes = proxyObj.foo(new Base())
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof int)
    arktest.assertEQ(fooRes, 1)
    LATEST_EVENT = Event.NONE
}

function testInvokeDIface(proxyObj: DIface) {
    let fooRes = proxyObj.foo(new Derived())
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof int)
    arktest.assertEQ(fooRes, 1)
    LATEST_EVENT = Event.NONE
}

function testOverloads() {
    let obj = new CD()
    let linker = Class.of(obj).getLinker()
    let interfaces1: FixedArray<Class> = [ Class.from<CIface>(), Class.from<DIface>() ]
    let handler1 = new Handler(obj)
    let proxyInstance1 = reflect.Proxy.create(linker, interfaces1, handler1)

    checkIsProxy(proxyInstance1)
    testInvokeCIface(proxyInstance1 as CIface)
    testInvokeDIface(proxyInstance1 as DIface) // owner type still CIface

    let interfaces2: FixedArray<Class> = [ Class.from<DIface>(), Class.from<CIface>() ]
    let handler2 = new Handler(obj)
    // even DIface passed foremost in Proxy.create, foo from CIface will be chosen as more basic.
    let proxyInstance2 = reflect.Proxy.create(linker, interfaces2, handler2)

    checkIsProxy(proxyInstance2)
    testInvokeCIface(proxyInstance2 as CIface)
    testInvokeDIface(proxyInstance2 as DIface)
}

function testInvokeEIface(proxyObj: EIface) {
    let fooRes = proxyObj.foo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 1.2)
    LATEST_EVENT = Event.NONE
}

function testInvokeFIface(proxyObj: FIface) {
    let fooRes = proxyObj.foo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 1.2)
    LATEST_EVENT = Event.NONE
}

function testTwoInterfacesWithSameMethod() {
    let obj = new EF
    let linker = Class.of(obj).getLinker()

    let interfaces1: FixedArray<Class> = [ Class.from<EIface>(), Class.from<FIface>() ]
    let handler1 = new Handler(obj)
    // EIface passed foreword, so EIface.foo will be chosen
    let proxyInstance1 = reflect.Proxy.create(linker, interfaces1, handler1)

    checkIsProxy(proxyInstance1)
    testInvokeEIface(proxyInstance1 as EIface)
    testInvokeFIface(proxyInstance1 as FIface) // owner still EIface

    let interfaces2: FixedArray<Class> = [ Class.from<FIface>(), Class.from<EIface>() ]
    let handler2 = new Handler(obj)
    let proxyInstance2 = reflect.Proxy.create(linker, interfaces2, handler2)

    checkIsProxy(proxyInstance2)
    testInvokeEIface(proxyInstance2 as EIface)
    testInvokeFIface(proxyInstance2 as FIface)
}

function testInvokeGIface(proxyObj: GIface) {
    let fooRes = proxyObj.foo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 1)
    LATEST_EVENT = Event.NONE
}

function testInvokeHIface(proxyObj: HIface) {
    let fooRes = proxyObj.foo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 1)
    LATEST_EVENT = Event.NONE
}

function testTwoConflictingDefaultMethods() {
    let obj1 = new G
    let obj2 = new H

    let linker = Class.from<GIface>().getLinker()
    let handler = new Handler(obj1)
    let interfaces1: FixedArray<Class> = [ Class.from<GIface>(), Class.from<HIface>() ]

    let proxyInstance1 = reflect.Proxy.create(linker, interfaces1, handler)
    checkIsProxy(proxyInstance1)

    testInvokeGIface(proxyInstance1 as GIface)
    testInvokeHIface(proxyInstance1 as HIface)
}

function testDefaultMethods1InvokeGIface(proxyObj: GIface) {
    let fooRes = proxyObj.foo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 1.2)
    LATEST_EVENT = Event.NONE
}

function testDefaultMethods1InvokeHIface(proxyObj: HIface) {
    let fooRes = proxyObj.foo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 1.2)
    LATEST_EVENT = Event.NONE
}

function testDefaultMethods1() {
    let obj = new EFGH
    let linker = Class.of(obj).getLinker()
    // in case of the same signature methods with and without default implmenetations, we take first method without default implementation
    // E.foo is foremost in passed array of interfaces.
    let handler1 = new Handler(obj)
    let interfaces1: FixedArray<Class> = [ Class.from<EIface>(), Class.from<FIface>(), Class.from<GIface>(), Class.from<HIface>() ]
    let proxyInstance1 = reflect.Proxy.create(linker, interfaces1, handler1)

    checkIsProxy(proxyInstance1)
    testInvokeEIface(proxyInstance1 as EIface)
    testInvokeFIface(proxyInstance1 as FIface)
    testDefaultMethods1InvokeGIface(proxyInstance1 as GIface)
    testDefaultMethods1InvokeHIface(proxyInstance1 as HIface)

    let interfaces2: FixedArray<Class> = [ Class.from<GIface>(), Class.from<HIface>(), Class.from<EIface>(), Class.from<FIface>() ]
    let proxyInstance2 = reflect.Proxy.create(linker, interfaces2, handler1)

    checkIsProxy(proxyInstance2)
    testInvokeEIface(proxyInstance1 as EIface)
    testInvokeFIface(proxyInstance1 as FIface)
    testDefaultMethods1InvokeGIface(proxyInstance1 as GIface)
    testDefaultMethods1InvokeHIface(proxyInstance1 as HIface)

    let handler3 = new Handler(obj)
    let interfaces3: FixedArray<Class> = [ Class.from<GIface>(), Class.from<FIface>(), Class.from<HIface>(), Class.from<EIface>() ]
    let proxyInstance3 = reflect.Proxy.create(linker, interfaces3, handler3)

    checkIsProxy(proxyInstance3)
    testInvokeEIface(proxyInstance3 as EIface)
    testInvokeFIface(proxyInstance3 as FIface)
    testDefaultMethods1InvokeGIface(proxyInstance3 as GIface)
    testDefaultMethods1InvokeHIface(proxyInstance3 as HIface)
}

function testInvokeIIface(proxyObj: IIface) {
    let fooRes = proxyObj.foo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 1)
    LATEST_EVENT = Event.NONE
}

function testInvokeJIface(proxyObj: JIface) {
    let fooRes = proxyObj.foo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 1)
    LATEST_EVENT = Event.NONE
}

function testDefaultMethods2() {
    let obj = new IJ
    let linker = Class.of(obj).getLinker()
    let handler = new Handler(obj)

    let interfaces1: FixedArray<Class> = [ Class.from<IIface>(), Class.from<JIface>() ]
    let proxyInstance1 = reflect.Proxy.create(linker, interfaces1, handler)
    checkIsProxy(proxyInstance1)
    testInvokeIIface(proxyInstance1 as IIface)
    testInvokeJIface(proxyInstance1 as JIface)

    let interfaces2: FixedArray<Class> = [ Class.from<JIface>(), Class.from<IIface>() ]
    let proxyInstance2 = reflect.Proxy.create(linker, interfaces2, handler)
    checkIsProxy(proxyInstance2)
    testInvokeIIface(proxyInstance2 as IIface)
    testInvokeJIface(proxyInstance2 as JIface)
}

function testInvokeKIface(proxyObj: KIface) {
    let fooRes = proxyObj.foo(new Base())
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof string)
    arktest.assertEQ(fooRes, "3")
    LATEST_EVENT = Event.NONE
}

function testInvokeLIface(proxyObj: LIface) {
    let fooRes = proxyObj.foo(new Derived()) // foo will be taken from KIface with signature foo(Base), but Derived is assignable.
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof string)
    arktest.assertEQ(fooRes, "3")
    LATEST_EVENT = Event.NONE
}

function testDefaultMethods3() {
    let obj = new KL
    let linker = Class.of(obj).getLinker()
    let handler = new Handler(obj)

    let interfaces1: FixedArray<Class> = [ Class.from<KIface>(), Class.from<LIface>() ]
    let proxyInstance1 = reflect.Proxy.create(linker, interfaces1, handler)
    checkIsProxy(proxyInstance1)
    testInvokeKIface(proxyInstance1 as KIface)
    testInvokeLIface(proxyInstance1 as LIface)

    let interfaces2: FixedArray<Class> = [ Class.from<LIface>(), Class.from<KIface>() ]
    let proxyInstance2 = reflect.Proxy.create(linker, interfaces2, handler)
    checkIsProxy(proxyInstance2)
    testInvokeKIface(proxyInstance2 as KIface)
    testInvokeLIface(proxyInstance2 as LIface)
}

function testInvokeMIface(proxyObj: MIface) {
    let fooRes = proxyObj.foo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 12)
    LATEST_EVENT = Event.NONE
}

function testDefaultMethods4() {
    let obj = new M
    let linker = Class.of(obj).getLinker()
    let handler = new Handler(obj)

    let interfaces: FixedArray<Class> = [ Class.from<MIface>() ]
    let proxyInstance = reflect.Proxy.create(linker, interfaces, handler)
    checkIsProxy(proxyInstance)

    testInvokeMIface(proxyInstance as MIface)
}

function testInvokeNIface(proxyObj: NIface) {
    let fooRes = proxyObj.foo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof string)
    arktest.assertEQ(fooRes, "111")
    LATEST_EVENT = Event.NONE
}

function testInvokeOIface(proxyObj: OIface) {
    let fooRes = proxyObj.foo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof long)
    arktest.assertEQ(fooRes,  111)
    LATEST_EVENT = Event.NONE
}

function testTwoInterfacesDifferentReturnTypes() {
    let obj1 = new N
    let obj2 = new O
    let linker = Class.of(obj1).getLinker()

    let handler = new Handler(obj1)
    let interfaces: FixedArray<Class> = [ Class.from<NIface>(), Class.from<OIface>() ]

    let proxyInstance = reflect.Proxy.create(linker, interfaces, handler)
    checkIsProxy(proxyInstance)

    testInvokeNIface(proxyInstance as NIface)

    const errorcheck = (e: Error) => {
        return e.message === "Object type [ProxyBasicTests.N] is not compatible with method owner type [ProxyBasicTests.OIface]"
    }
    arktest.expectThrow((): void => { testInvokeOIface(proxyInstance as OIface) }, errorcheck)

    handler.setTarget(obj2)
    testInvokeOIface(proxyInstance as OIface)

    let proxyClass = Class.of(proxyInstance)
    let methods = proxyClass.getInstanceMethods()
    arktest.assertEQ(methods.length, 3)

    const errorcheck2 = (e: Error) => {
        return e.message === "Found two methods with the same name 'foo'. Please specify signature."
    }
    arktest.expectThrow((): void => { proxyClass.getInstanceMethod("foo") }, errorcheck2)

    // first match here
    proxyClass.getInstanceMethod("foo", [])
}

function testInvokePIface(proxyObj: PIface) {
    let fooRes = proxyObj.foo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof string)
    arktest.assertEQ(fooRes, "123456")
    LATEST_EVENT = Event.NONE
}

function testInvokeQIface(proxyObj: QIface) {
    let fooRes = proxyObj.foo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof long)
    arktest.assertEQ(fooRes,  123456)
    LATEST_EVENT = Event.NONE
}

function testTwoInterfacesDefaultDifferentReturnTypes() {
    let obj1 = new P
    let obj2 = new Q
    let linker = Class.of(obj1).getLinker()
    let handler = new Handler(obj1)
    let interfaces: FixedArray<Class> = [ Class.from<PIface>(), Class.from<QIface>() ]

    let proxyInstance = reflect.Proxy.create(linker, interfaces, handler)
    checkIsProxy(proxyInstance)

    testInvokePIface(proxyInstance as PIface)

    const errorcheck = (e: Error) => {
        return e.message === "Object type [ProxyBasicTests.P] is not compatible with method owner type [ProxyBasicTests.QIface]"
    }
    arktest.expectThrow((): void => { testInvokeQIface(proxyInstance as QIface) }, errorcheck)

    handler.setTarget(obj2)
    testInvokeQIface(proxyInstance as QIface)
}

function testInvokeRIface(proxyObj: RIface) {
    let fooRes = proxyObj.foo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 4.2)
    LATEST_EVENT = Event.NONE
}

function testInvokeSIface(proxyObj: SIface) {
    let fooRes = proxyObj.foo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 4.2)
    LATEST_EVENT = Event.NONE
}

function testTwoIntfOneDefaultImpl() {
    let obj = new RS
    let linker = Class.of(obj).getLinker()
    let handler = new Handler(obj)
    let interfaces1: FixedArray<Class> = [ Class.from<RIface>(), Class.from<SIface>() ]

    let proxyInstance1 = reflect.Proxy.create(linker, interfaces1, handler)
    checkIsProxy(proxyInstance1)

    testInvokeRIface(proxyInstance1 as RIface)
    testInvokeSIface(proxyInstance1 as SIface)

    let interfaces2: FixedArray<Class> = [ Class.from<SIface>(), Class.from<RIface>() ]

    let proxyInstance2 = reflect.Proxy.create(linker, interfaces2, handler)
    checkIsProxy(proxyInstance2)

    testInvokeRIface(proxyInstance2 as RIface)
    testInvokeSIface(proxyInstance2 as SIface)
}

function testInvokeTIface(proxyObj: TIface) {
    let fooRes = proxyObj.foo(new Base)
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 2.0)
    LATEST_EVENT = Event.NONE

    fooRes = proxyObj.foo(new Derived)
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 2.0)
    LATEST_EVENT = Event.NONE
}

function testInvokeUIface(proxyObj: UIface) {
    let fooRes = proxyObj.foo(new Derived)
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 2.0)
    LATEST_EVENT = Event.NONE
}

function testTwoIntfOneDefaultImpl2() {
    let obj = new TU
    let linker = Class.of(obj).getLinker()
    let handler = new Handler(obj)
    let interfaces: FixedArray<Class> = [ Class.from<TIface>(), Class.from<UIface>() ]

    let proxyInstance = reflect.Proxy.create(linker, interfaces, handler)
    checkIsProxy(proxyInstance)

    testInvokeTIface(proxyInstance as TIface)
    testInvokeUIface(proxyInstance as UIface)
}

function testInvokeVIface(proxyObj: VIface) {
    let fooRes = proxyObj.foo(new Base)
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 2.2)
    LATEST_EVENT = Event.NONE

    fooRes = proxyObj.foo(new Derived)
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 2.2)
    LATEST_EVENT = Event.NONE
}

function testInvokeWIface(proxyObj: WIface) {
    let fooRes = proxyObj.foo(new Derived)
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(fooRes instanceof double)
    arktest.assertEQ(fooRes, 2.2)
    LATEST_EVENT = Event.NONE
}

function testTwoIntfOneDefaultImpl3() {
    let obj = new VW
    let linker = Class.of(obj).getLinker()
    let handler = new Handler(obj)
    let interfaces: FixedArray<Class> = [ Class.from<VIface>(), Class.from<WIface>() ]

    let proxyInstance = reflect.Proxy.create(linker, interfaces, handler)
    checkIsProxy(proxyInstance)

    testInvokeVIface(proxyInstance as VIface)
    testInvokeWIface(proxyInstance as WIface)
}

function testGetHandlerProxy1() {
    let obj = new X
    let linker = Class.of(obj).getLinker()
    let handler = new Handler(obj)

    let interfaces: FixedArray<Class> = [ Class.from<XIface>()]
    let proxyInstance = reflect.Proxy.create(linker, interfaces, handler)
    checkIsProxy(proxyInstance)

    let xifaceProxyObj = proxyInstance as XIface
    let res = xifaceProxyObj.getHandler()
    // It is prohibited to proxy interface with method `getHandler: reflect.InvocationHandler`
    // In such case original getHandler from reflect.Proxy will be called and proxy candidate will be ignored
    arktest.assertEQ(LATEST_EVENT, Event.NONE)
    arktest.assertEQ(res, handler)
    arktest.assertEQ(res, proxyInstance.getHandler())
}

function testGetHandlerProxy2() {
    let obj = new Y
    let linker = Class.of(obj).getLinker()
    let handler = new Handler(obj)

    let interfaces: FixedArray<Class> = [ Class.from<YIface>()]
    let proxyInstance = reflect.Proxy.create(linker, interfaces, handler)
    checkIsProxy(proxyInstance)

    let yifaceProxyObj = proxyInstance as YIface
    let res = yifaceProxyObj.getHandler()
    // It is prohibited to proxy interface with method `getHandler: reflect.InvocationHandler`
    // In such case original getHandler from reflect.Proxy will be called and proxy candidate will be ignored
    arktest.assertEQ(LATEST_EVENT, Event.NONE)
    arktest.assertEQ(res, handler)
    arktest.assertEQ(res, proxyInstance.getHandler())
}

function testInvokeZIface(proxyObj: ZIface) {
    let res = proxyObj.getHandler()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    arktest.assertTrue(res instanceof char)
    arktest.assertEQ(res, c'f')
    LATEST_EVENT = Event.NONE
}

function testGetHandlerProxy3() {
    let obj = new Z
    let linker = Class.of(obj).getLinker()
    let handler = new Handler(obj)

    let interfaces: FixedArray<Class> = [ Class.from<ZIface>()]
    let proxyInstance = reflect.Proxy.create(linker, interfaces, handler)
    checkIsProxy(proxyInstance)

    testInvokeZIface(proxyInstance as ZIface)

    arktest.assertEQ(proxyInstance.getHandler(), handler)
    arktest.assertEQ((proxyInstance as ZIface).getHandler(), c'f')

    arktest.assertEQ(Class.of(proxyInstance).getInstanceMethods().length, 2)
}

function main() {
    const suite = new arktest.ArkTestsuite('Proxy test')

    suite.addTest('Empty interface', testEmpty)
    suite.addTest('Single interface', testAIface)
    suite.addTest('Inconsistent input', testInconsistentInput)

    suite.addTest('Two interface', testABProxyClass)
    suite.addTest('Two interface with overloads and override-compatible signatures', testOverloads)
    suite.addTest('Two interfaces, both add method foo with the same parameters and return type', testTwoInterfacesWithSameMethod)

    suite.addTest('Two interfaces that provide conflicting default implementations', testTwoConflictingDefaultMethods)
    suite.addTest('Interfaces with multiple default methods implementations', testDefaultMethods1)
    suite.addTest('Two interfaces with methods that both refer to same method with default implementation', testDefaultMethods2)
    suite.addTest('Interfaces with the same method signatures but different return types', testTwoInterfacesDifferentReturnTypes)
    suite.addTest('Interfaces with the same method signatures but different return types (default implementations)', testTwoInterfacesDefaultDifferentReturnTypes)
    suite.addTest('Two interfaces, both has compatible method, one of which has default implementations: part1', testTwoIntfOneDefaultImpl)
    suite.addTest('Two interfaces, both has compatible method, one of which has default implementations: part2', testTwoIntfOneDefaultImpl2)

    // enable after #32216
    // suite.addTest('Two interfaces, both has compatible method, one of which has default implementations: part3', testTwoIntfOneDefaultImpl3)

    // enable after #31839 is fixed.
    // suite.addTest('Two interfaces, both provide override-compatible methods with same name', testDefaultMethods3)
    // suite.addTest('Diamon interfaces structure with overriding in the most specific interface', testDefaultMethods4)

    suite.addTest('Interface with method `getHandler: reflect.InvocationHandler`', testGetHandlerProxy1)
    suite.addTest('Interface with default implementation of method `getHandler: reflect.InvocationHandler`', testGetHandlerProxy2)
    suite.addTest('Interface with method `getHandler: char`', testGetHandlerProxy3)

    return suite.run()
}
