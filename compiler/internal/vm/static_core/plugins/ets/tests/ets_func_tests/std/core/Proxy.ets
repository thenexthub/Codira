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

interface I1 {
    fld: number
    foo()
    goo(val: number)
}

class I1Impl implements I1 {
    fld: number = 1.0
    foo() {

    }
    goo(val: number) {
        this.fld = val
    }
}

interface I2 {
    foo()
    bar()
}

class I2Impl implements I2 {
    foo() {
    }
    bar() {
    }
}

interface I3 extends I1 {
    foo()
}

class I3Impl implements I3 {
    fld: number = 2.0
    foo() {

    }
    goo(val: number) {
        this.fld = val
    }
}

enum Event {
    NONE,
    GETTER_CALLED,
    SETTER_CALLED,
    METHOD_CALLED,
}

let LATEST_EVENT: Event = Event.NONE

class BasicHandler implements reflect.InvocationHandler {
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        LATEST_EVENT = Event.GETTER_CALLED
        return 1.0
    }

    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        LATEST_EVENT = Event.SETTER_CALLED
    }

    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        LATEST_EVENT = Event.METHOD_CALLED
        return true
    }
}

function createProxy(ifaces: FixedArray<Class>, handle: reflect.InvocationHandler) : reflect.Proxy {
    let linker = Class.current().getLinker()
    return reflect.Proxy.create(linker, ifaces, handle)
}

// Test proxyClass basic properties
function createProxyBasicTest(){
    let ifaces: FixedArray<Class> = [ Class.from<I1>() ]

    let proxy1 = createProxy(ifaces, new BasicHandler())
    arktest.assertFalse(Class.of(proxy1).isAbstract())
    arktest.assertTrue(Class.of(proxy1).isFinal())
    arktest.assertEQ(Class.of(proxy1).getSuper(), Class.from<reflect.Proxy>())

    let proxy2 = createProxy(ifaces, new BasicHandler())
    arktest.assertFalse(Class.of(proxy2).isAbstract())
    arktest.assertTrue(Class.of(proxy2).isFinal())
    arktest.assertEQ(Class.of(proxy2).getSuper(), Class.from<reflect.Proxy>())
}

// Test example
function exampleDispatchTest() {
    let ifaces: FixedArray<Class> =  [ Class.from<I1>() ]
    let proxy1: I1 = createProxy(ifaces, new BasicHandler()) as I1
    exampleDispatchTestBody(proxy1)
    LATEST_EVENT = Event.NONE

    let proxy2: I1 = createProxy(ifaces, new BasicHandler()) as I1
    exampleDispatchTestBody(proxy2)
    LATEST_EVENT = Event.NONE
}

function exampleDispatchTestBody(proxy: I1) {
    arktest.assertEQ(LATEST_EVENT, Event.NONE)
    proxy.foo()
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
    proxy.fld = 1.0
    arktest.assertEQ(LATEST_EVENT, Event.SETTER_CALLED)
    proxy.fld
    arktest.assertEQ(LATEST_EVENT, Event.GETTER_CALLED)
}

/** The following section contains basic tests for dispatch */

// init
let i1 = Class.from<I1>()
let i2 = Class.from<I2>()
let i3 = Class.from<I3>()

let i1foo = i1.getInstanceMethod('foo')
let i1goo = i1.getInstanceMethod('goo')
let i1fldSetter = i1.getInstanceMethod('%%set-fld')
let i1fldGetter = i1.getInstanceMethod('%%get-fld')

let i2foo = i2.getInstanceMethod('foo')
let i2bar = i2.getInstanceMethod('bar')

let i3foo = i3.getInstanceMethod('foo')
let i3goo = i3.getInstanceMethod('goo')
let i3fldSetter = i3.getInstanceMethod('%%set-fld')
let i3fldGetter = i3.getInstanceMethod('%%get-fld')

// get proxy
let ifaces1: FixedArray<Class> =  [ Class.from<I1>() ]
let proxyClass1: I1 = createProxy(ifaces1, new TestHandler1(new I1Impl())) as I1
let proxy1: I1 = createProxy(ifaces1, new TestHandler1(new I1Impl())) as I1


let ifaces2: FixedArray<Class> =  [ Class.from<I1>() ]
let proxyClass2: I1 = createProxy(ifaces2, new TestHandler2(new I1Impl())) as I1
let proxy2: I1 = createProxy(ifaces2, new TestHandler2(new I1Impl())) as I1

let ifaces3: FixedArray<Class> =  [ Class.from<I3>() ]
let proxyClass3: I3 = createProxy(ifaces3, new TestHandler3()) as I3
let proxy3: I3 = createProxy(ifaces3, new TestHandler3()) as I3


//dumplicate
let ifaces4: FixedArray<Class> =  [ Class.from<I3>() ]
let proxyClass4: I1 = createProxy(ifaces4, new TestHandler4(new I3Impl())) as I1
let proxy4: I1 = createProxy(ifaces4, new TestHandler4(new I3Impl())) as I1
let proxyClass5: I3 = createProxy(ifaces4, new TestHandler5(new I3Impl())) as I3
let proxy5: I3 = createProxy(ifaces4, new TestHandler5(new I3Impl())) as I3

let ifaces5: FixedArray<Class> =  [ Class.from<I2>() ]
let proxyClass6: I2 = createProxy(ifaces5, new TestHandler6(new I2Impl())) as I2
let proxy6: I2 = createProxy(ifaces5, new TestHandler6(new I2Impl())) as I2
let proxyClass7: I2 = createProxy(ifaces5, new TestHandler7(new I2Impl())) as I2
let proxy7: I2 = createProxy(ifaces5, new TestHandler7(new I2Impl())) as I2

// CreatProxy for test
class TestHandler1 implements reflect.InvocationHandler {
    instance: I1
    constructor(other: I1) {
        this.instance = other
    }
    //check that proxy === p and method.equals(i1fldGetter)
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        arktest.assertTrue(proxy === proxyClass1 || proxy === proxy1)
        arktest.assertTrue(method.equals(i1fldGetter!))
        return method.invoke(this.instance)
    }
    //check that proxy === p and method.equals(i1fldSetter) and value === v
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void{
        arktest.assertTrue(proxy === proxyClass1 || proxy === proxy1)
        arktest.assertTrue(method.equals(i1fldSetter!))
        arktest.assertEQ(value, 1)
        method.invoke(this.instance, [value])
    }
    //check that proxy === p and method.equals(i1foo) and args.length === 0
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        arktest.assertTrue(proxy === proxyClass1 || proxy === proxy1)
        arktest.assertTrue(method.equals(i1foo!))
        arktest.assertEQ(args.length, 0)
        return true
    }
}

class TestHandler2 implements reflect.InvocationHandler {
    instance: I1
    constructor(other: I1) {
        this.instance = other
    }
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return method.invoke(this.instance)
    }
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        method.invoke(this.instance, [value])
    }
    //check proxy === p and method.equals(i1goo) and args.length === 1 and args[0] === 1
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        arktest.assertTrue(proxy === proxyClass2 || proxy === proxy2)
        arktest.assertTrue(method.equals(i1goo!))
        arktest.assertEQ(args.length, 1)
        arktest.assertEQ(args[0], 1)
        return true
    }
}

class TestHandler3 implements reflect.InvocationHandler {
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
       return true
    }
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
    }
    //check proxy === p and args.length === 0
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        arktest.assertTrue(proxy === proxyClass3 || proxy === proxy3)
        arktest.assertEQ(args.length, 0)
        return true
    }
}

class TestHandler4 implements reflect.InvocationHandler {
    instance: I1
    constructor(other: I1) {
        this.instance = other
    }
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return method.invoke(this.instance)
    }
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        method.invoke(this.instance, [value])
    }
    //check proxy === p1 and args.length === 0
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        arktest.assertTrue(proxy === proxyClass4 || proxy === proxy4)
        arktest.assertEQ(args.length, 0)
        return method.invoke(this.instance, args)
    }
}

class TestHandler5 implements reflect.InvocationHandler {
    instance: I3
    constructor(other: I3) {
        this.instance = other
    }
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return method.invoke(this.instance)
    }
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        method.invoke(this.instance, [value])
    }
    //check proxy === p1 and args.length === 0
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        arktest.assertTrue(proxy === proxyClass5 || proxy === proxy5)
        arktest.assertEQ(args.length, 0)
        return method.invoke(this.instance, args)
    }
}

class TestHandler6 implements reflect.InvocationHandler {
    instance: I2
    constructor(other: I2) {
        this.instance = other
    }
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return method.invoke(this.instance)
    }
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void{
        method.invoke(this.instance, [value])
    }
    //check proxy === p2 and method.equals(i2foo) and args.length === 0
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        arktest.assertTrue(proxy === proxyClass6 || proxy === proxy6)
        arktest.assertTrue(method.equals(i2foo!))
        arktest.assertEQ(args.length, 0)
        return method.invoke(this.instance, args)
    }
}

class TestHandler7 implements reflect.InvocationHandler {
    instance: I2
    constructor(other: I2) {
        this.instance = other
    }
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return method.invoke(this.instance)
    }
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        method.invoke(this.instance, [value])
    }
    //check proxy === p2 and method.equals(i2bar) and args.length === 0
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        arktest.assertTrue(proxy === proxyClass7 || proxy === proxy7)
        arktest.assertTrue(method.equals(i2bar!))
        arktest.assertEQ(args.length, 0)
        return method.invoke(this.instance, args)
    }
}

function basicDispatchTest() {
    // basicDispatchTest
    basicDispatchTest1(proxyClass1)
    basicDispatchTest1(proxy1)

    // invokeWithValueTest
    basicDispatchTest2(proxyClass2)
    basicDispatchTest2(proxy2)

    // invokeOverrideTest
    basicDispatchTest3(proxyClass3)
    basicDispatchTest3(proxy3)

    // invokeDuplicateTest ( p1 as I1)
    basicDispatchTest4(proxyClass4)
    basicDispatchTest4(proxy4)

    // invokeDuplicateTest ( p1 as I3)
    basicDispatchTest5(proxyClass5)
    basicDispatchTest5(proxy5)

    // invokeDuplicateTest ( p2 as I1)
    basicDispatchTest6(proxyClass6)
    basicDispatchTest6(proxy6)

    // invokeDuplicateTest ( p2 as I3)
    basicDispatchTest7(proxyClass7)
    basicDispatchTest7(proxy7)
}

//basicDispatchTest
function basicDispatchTest1(proxy: I1) {
    proxy.foo()
    proxy.fld = 1.0
    proxy.fld
}

//invokeWithValueTest
function basicDispatchTest2(proxy: I1) {
   proxy.goo(1)
}

//invokeOverrideTest
function basicDispatchTest3(proxy: I3) {
   proxy.foo()
}

//invokeDuplicateTest
function basicDispatchTest4(proxy: I1) {
   proxy.foo()
}

function basicDispatchTest5(proxy: I3) {
   proxy.foo()
}

function basicDispatchTest6(proxy: I2) {
   proxy.foo()
}

function basicDispatchTest7(proxy: I2) {
   proxy.bar()
}

// extra Test
interface A {
    foo()
    goo()
}

interface B {
    foo()
    hoo()
}

interface C extends A, B {
    zoo()
}

interface D {
    koo(): int{
        return 1
    }
}

class dImpl implements D{
    foo(): double { return 1.423}
}

interface AIface{
    aoo(): int
}

class AI implements AIface {
    public aoo(): int {
        return 2
    }
}

interface E {
    foo(): int {
        return 1
    }
    goo(num: number)
    hoo()
}

interface F {
    foo(): int
    goo(num: number)
    ioo()
}

class EF implements E, F {
    goo(num: number) {
    }
    hoo() {
    }
    ioo() {
    }
}

class FE implements F, E {
    goo(num: number) {
    }
    hoo() {
    }
    ioo() {
    }
}

let iA = Class.from<A>()
let iB = Class.from<B>()
let iC = Class.from<C>()
let iE = Class.from<E>()
let iF = Class.from<F>()

let fooA = iA.getInstanceMethod('foo')
let goo = iA.getInstanceMethod('goo')
let fooB = iB.getInstanceMethod('foo')
let hoo = iB.getInstanceMethod('hoo')
let zoo = iC.getInstanceMethod('zoo')
let gooE = iE.getInstanceMethod('goo')
let gooF = iF.getInstanceMethod('goo')

class ExtraHandler1 implements reflect.InvocationHandler {
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
       return true
    }
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
    }
    //check tree structure
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        if (method.getName() == 'foo') {
            arktest.assertTrue(method.equals(fooA!))
            arktest.assertEQ(args.length, 0)
        }
        if (method.getName() == 'goo') {
            arktest.assertTrue(method.equals(goo!))
            arktest.assertEQ(args.length, 0)
        }
        if (method.getName() == 'hoo') {
            arktest.assertTrue(method.equals(hoo!))
            arktest.assertEQ(args.length, 0)
        }
        if (method.getName() == 'zoo') {
            arktest.assertTrue(method.equals(zoo!))
            arktest.assertEQ(args.length, 0)
        }
        return true
    }
}

class ExtraHandler2 implements reflect.InvocationHandler {
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
       return true
    }
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
    }
    //check
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        arktest.assertTrue(method.equals(fooA!))
        arktest.assertEQ(args.length, 0)
        return true
    }
}

class ExtraHandler3 implements reflect.InvocationHandler {
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return true
    }
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
    }
    //check
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        arktest.assertTrue(method.equals(fooA!))
        arktest.assertEQ(args.length, 0)
        return true
    }
}

class ExtraHandler4 implements reflect.InvocationHandler {
    instance: D
    constructor(other: D) {
        this.instance = other
    }
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
       return method.invoke(this.instance)
    }
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
    }
    //check
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        arktest.assertEQ(args.length, 0)
        return method.invoke(this.instance, args)
    }
}

class ExtraHandler5 implements reflect.InvocationHandler {
    instance: AI
    constructor(other: AI) {
        this.instance = other
    }
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return method.invoke(this.instance)
    }
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        method.invoke(this.instance, [value])
    }
    //check
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        arktest.assertEQ(args.length, 0)
        LATEST_EVENT = Event.METHOD_CALLED
        return method.invoke(this.instance, args)
    }
}

class ExtraHandler6 implements reflect.InvocationHandler {
    instance: EF
    constructor(other: EF) {
        this.instance = other
    }
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return method.invoke(this.instance)
    }
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        method.invoke(this.instance, [value])
    }
    //check
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        if (method.getName() == 'goo') {
            arktest.assertTrue(method.equals(gooE!))
        }
        return method.invoke(this.instance, args)
    }
}

class ExtraHandler7 implements reflect.InvocationHandler {
    instance: EF
    constructor(other: EF) {
        this.instance = other
    }
    get(proxy: reflect.Proxy, method: reflect.InstanceMethod): Any {
        return method.invoke(this.instance)
    }
    set(proxy: reflect.Proxy, method: reflect.InstanceMethod, value: Any): void {
        method.invoke(this.instance, [value])
    }
    //check
    invoke(proxy: reflect.Proxy, method: reflect.InstanceMethod, args: FixedArray<Any>): Any {
        if (method.getName() == 'goo') {
            arktest.assertTrue(method.equals(gooF!))
        }
        return method.invoke(this.instance, args)
    }
}

//Test different variant of interfaces three structure
function extraDispatchTest1() {
    let ifaces: FixedArray<Class> =  [ Class.from<C>() ]
    let proxy1: C = createProxy(ifaces, new ExtraHandler1()) as C
    extraDispatchTestBody1(proxy1)
    LATEST_EVENT = Event.NONE

    let proxy2: C = createProxy(ifaces, new ExtraHandler1()) as C
    extraDispatchTestBody1(proxy2)
    LATEST_EVENT = Event.NONE
}

function extraDispatchTestBody1(proxy: C) {
    proxy.foo()
    proxy.goo()
    proxy.hoo()
    proxy.zoo()
}

//Check behavoior of proxy change if order of passing interfaces to creating proxy changed
function extraDispatchTest2() {
    let ifaces: FixedArray<Class> =  [ Class.from<C>() ]
    let proxy1: B = createProxy(ifaces, new ExtraHandler2()) as B
    extraDispatchTestBody2(proxy1)

    let proxy2: B = createProxy(ifaces, new ExtraHandler2()) as B
    extraDispatchTestBody2(proxy2)
}

function extraDispatchTestBody2(proxy: B) {
    proxy.foo()
}

function extraDispatchTest3() {
    let ifaces: FixedArray<Class> =  [ Class.from<C>() ]
    let proxy1: A = createProxy(ifaces, new ExtraHandler3()) as A
    extraDispatchTestBody3(proxy1)

    let proxy2: A = createProxy(ifaces, new ExtraHandler3()) as A
    extraDispatchTestBody3(proxy2)
}

function extraDispatchTestBody3(proxy: A) {
    proxy.foo()
}

//Test that default methods in interface can be proxied successfully
function extraDispatchTest4() {
    let ifaces: FixedArray<Class> =  [ Class.from<D>() ]
    let proxy1: D = createProxy(ifaces, new ExtraHandler4(new dImpl())) as D
    extraDispatchTestBody4(proxy1)

    let proxy2: D = createProxy(ifaces, new ExtraHandler4(new dImpl())) as D
    extraDispatchTestBody4(proxy2)
}

function extraDispatchTestBody4(proxy: D) {
    let i = proxy.koo()
    arktest.assertEQ(i, 1)
}

//Test for non-trivial handler that will obtain instance of interface and call method from it
function extraDispatchTest5() {
    let ifaces: FixedArray<Class> =  [ Class.from<AIface>() ]
    let proxy1: AIface = createProxy(ifaces, new ExtraHandler5(new AI())) as AIface
    extraDispatchTestBody5(proxy1)
    LATEST_EVENT = Event.NONE
    let proxy2: AIface = createProxy(ifaces, new ExtraHandler5(new AI())) as AIface
    extraDispatchTestBody5(proxy2)
    LATEST_EVENT = Event.NONE
}

function extraDispatchTestBody5(proxy: AIface) {
    let i = proxy.aoo()
    arktest.assertEQ(i, 2)
    arktest.assertEQ(LATEST_EVENT, Event.METHOD_CALLED)
}

//Test a scenario involving multiple interfaces
function extraDispatchTest6() {
    let ifaces: FixedArray<Class> =  [ Class.from<E>(), Class.from<F>() ]
    let proxy1: E = createProxy(ifaces, new ExtraHandler6(new EF())) as E
    let proxy2: F = createProxy(ifaces, new ExtraHandler6(new EF())) as F
    extraDispatchTestBody6(proxy1)
    extraDispatchTestBody7(proxy2)
    LATEST_EVENT = Event.NONE
    let proxy3: E = createProxy(ifaces, new ExtraHandler6(new EF())) as E
    let proxy4: F = createProxy(ifaces, new ExtraHandler6(new EF())) as F
    extraDispatchTestBody6(proxy3)
    extraDispatchTestBody7(proxy4)
    LATEST_EVENT = Event.NONE
}

function extraDispatchTestBody6(proxy: E) {
    let i = proxy.foo()
    proxy.goo(1.0)
    proxy.hoo()
}

//Test a scenario involving multiple interfaces
function extraDispatchTest7() {
    let ifaces: FixedArray<Class> =  [ Class.from<F>(), Class.from<E>() ]
    let proxy1: F = createProxy(ifaces, new ExtraHandler7(new EF())) as F
    extraDispatchTestBody7(proxy1)
    LATEST_EVENT = Event.NONE
    let proxy2: F = createProxy(ifaces, new ExtraHandler7(new EF())) as F
    extraDispatchTestBody7(proxy2)
    LATEST_EVENT = Event.NONE
}

function extraDispatchTestBody7(proxy: F) {
    let i = proxy.foo()
    proxy.goo(2.0)
    proxy.ioo()
}

function main() {
    const suite = new arktest.ArkTestsuite('Proxy test')
    suite.addTest('Test create proxyClass', createProxyBasicTest)
    suite.addTest('Test example proxyClass', exampleDispatchTest)
    suite.addTest('Test basic dispatch', basicDispatchTest)
    suite.addTest('Test different variant of interfaces', extraDispatchTest1)
    suite.addTest('Test proxy changed', extraDispatchTest2)
    suite.addTest('Test proxy changed', extraDispatchTest3)
    suite.addTest('Test default methods', extraDispatchTest4)
    suite.addTest('Test class proxy', extraDispatchTest5)
    suite.addTest('Test multiple interfaces proxy: part 1', extraDispatchTest6)
    suite.addTest('Test multiple interfaces proxy: part 2', extraDispatchTest7)
    return suite.run()
}
