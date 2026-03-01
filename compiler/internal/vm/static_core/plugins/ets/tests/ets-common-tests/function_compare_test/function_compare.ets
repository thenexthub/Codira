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

class X {
  static baz() {}
  foo() {}
}

class NotDerived {};

function bar() {}

function staticLambda(): void {}

function outerSingleCapture(): () => int {
  let captured = 42;
  return () => captured;
}

function outerMultiCapture(a: number, b: number): () => number {
  return () => a + b;
}

function makeLambda(): () => void {
  let x = new NotDerived();
  return () => { let _ = x };
}

// Basic test for identical function references
function testFunction() {
  let bar1 = bar
  let bar2 = bar
  arktest.assertEQ(bar1, bar2)
}

// Testing instance method references
function testInstanceMethod() {
  let x1 = new X()
  let x2 = new X()
  arktest.assertNE(x1, x2)
  let foo1 = x1.foo
  let foo2 = x1.foo
  let foo3 = x2.foo
  let baz1 = X.baz
  arktest.assertEQ(foo1, foo2)
  arktest.assertNE(foo1, foo3)
  arktest.assertNE(foo1, baz1)
}

// Static method references are shared
function testStaticMethod() {
  let baz1 = X.baz
  let baz2 = X.baz
  arktest.assertEQ(baz1, baz2)
}

// Arrow function capturing nothing vs capturing something
function testArrowFunctionCaptureIrrelevant() {
  let prev: Function | undefined = undefined
  for (let i = 0; i < 2; i++) {
    let capture = i
    let arrow = () => capture
    if (prev !== undefined) {
      arktest.assertNE(prev, arrow)
    }
    prev = arrow
  }
}

// Arrow function with no capture
function testArrowFunctionNoCapture() {
  let c: Function | undefined = undefined
  for (let i = 0; i < 2; i++) {
    let d = () => {}
    if (c != undefined) {
      arktest.assertNE(c, d)
    }
    c = d
  }
}

// Rest parameter test
function restTest(...args: number[]): number {
  return args.length
}
function testRestParameterFunctions() {
  let r1 = restTest
  let r2 = restTest
  arktest.assertEQ(r1, r2)
}

// Generic function
function generic<T>(value: T): T {
  return value
}
function testGenericFunctions<T>() {
  let g1 = generic<T>
  let g2 = generic<T>
  arktest.assertEQ(g1, g2)
}

// "Overloaded" function
function overloaded(value: number | string): number | string {
  return value
}
function testOverloadedFunction() {
  let o1 = overloaded
  let o2 = overloaded
  arktest.assertEQ(o1, o2)
}

// Inheritance and overriding
class Base {
  method() {}
}
class Derived extends Base {
  override method() {}
}
function testOverridingBehavior() {
    let b = new Base()
    let d = new Derived()
    arktest.assertNE(b.method, d.method)

    let baseMethod = (new Base()).method
    let derivedMethod = (new Derived()).method
    arktest.assertNE(baseMethod, derivedMethod)
}

// VerifyLambda tests
function testVerifyLambda_Static_NoFields() {
  let lambda1 = staticLambda
  let lambda2 = staticLambda
  arktest.assertEQ(lambda1, lambda2)
}

function testVerifyLambda_NonStatic_SingleField() {
  let lambda1 = outerSingleCapture()
  let lambda2 = outerSingleCapture()
  arktest.assertNE(lambda1, lambda2)
}

function testVerifyLambda_NonStatic_MultiFieldSimulated() {
  let lambda1 = outerMultiCapture(1, 2)
  let lambda2 = outerMultiCapture(3, 4)
  arktest.assertNE(lambda1, lambda2)
}

function testVerifyLambda_NonStatic_InvalidAssignable() {
  let lambda = makeLambda()
  arktest.assertTrue(typeof lambda === "function")
}

function main() {
  const suite = new arktest.ArkTestsuite('Function Compare tests')
  suite.addTest('Function Compare', testFunction)
  suite.addTest('Instance Method Compare', testInstanceMethod)
  suite.addTest('Static Method Compare', testStaticMethod)
  suite.addTest('Arrow Function (No Capture)', testArrowFunctionNoCapture)
  suite.addTest('Arrow Function (Capture)', testArrowFunctionCaptureIrrelevant)
  suite.addTest('Rest Parameter Function', testRestParameterFunctions)
  suite.addTest('Generic Function Comparison (Int)', testGenericFunctions<Int>)
  suite.addTest('Generic Function Comparison (String)', testGenericFunctions<String>)
  suite.addTest('Overloaded Function Comparison', testOverloadedFunction)
  suite.addTest('Override & Inheritance Comparison', testOverridingBehavior)
  suite.addTest('VerifyLambda: Static no fields', testVerifyLambda_Static_NoFields)
  suite.addTest('VerifyLambda: Non-static single captured field', testVerifyLambda_NonStatic_SingleField)
  suite.addTest('VerifyLambda: Non-static multi captured fields', testVerifyLambda_NonStatic_MultiFieldSimulated)
  suite.addTest('VerifyLambda: Captures unrelated type', testVerifyLambda_NonStatic_InvalidAssignable)

  return suite.run()
}
