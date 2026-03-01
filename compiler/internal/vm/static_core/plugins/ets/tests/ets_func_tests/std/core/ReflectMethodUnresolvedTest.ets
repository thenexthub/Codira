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

declare class NoImpClass {}

interface classWithNoImplField {
    bar(tmp: NoImpClass): NoImpClass
}

class classWithNoImplField1 {
    a: NoImpClass | undefined = undefined
    public static foo(tmp: NoImpClass | Number): Number { return 1}
}

function testStaticUnresolvedReturnTypeMethod() {
    const klass = Class.from<classWithNoImplField>()
    const bar = klass.getInstanceMethod("bar")
    arktest.expectThrow(() => {
        bar!.getReturnType()
    }, (e: Error) => { return e instanceof LinkerClassNotFoundError; });
}

function testStaticUnresolvedArgumentMethodGetArgument() {
    const klass = Class.from<classWithNoImplField1>()
    const foo = klass.getStaticMethod("foo")
    arktest.expectThrow(() => {
        foo!.getParameterType(0)
    }, (e: Error) => { return e instanceof LinkerClassNotFoundError; });

    arktest.expectThrow(() => {
        foo!.getParametersTypes()
    }, (e: Error) => { return e instanceof LinkerClassNotFoundError; });
}

function testStaticUnresolvedArgumentMethodInvoke() {
    const klass = Class.from<classWithNoImplField1>()
    const foo = klass.getStaticMethod("foo")
    arktest.expectThrow(() => {
        foo!.invoke([42])
    }, (e: Error) => { return e instanceof LinkerClassNotFoundError; });
}

function testGeneralBehaviorMethod() {
    testStaticUnresolvedReturnTypeMethod()
    testStaticUnresolvedArgumentMethodGetArgument()
    testStaticUnresolvedArgumentMethodInvoke()
}

function main() {
    const suite = new arktest.ArkTestsuite('Reflect Method Unresolved tests')
    suite.addTest('Test Reflect general behavior Method APIs', testGeneralBehaviorMethod)

    return suite.run()
}
