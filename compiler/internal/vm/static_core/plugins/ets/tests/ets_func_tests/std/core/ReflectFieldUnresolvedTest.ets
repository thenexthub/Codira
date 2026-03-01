/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

class ThrowOnInit {
    public static thrower: int = ((): int => {throw new Error('Class initialization;'); return 1 })()
    public static staticFld: Number = 42
}

declare class NoImpClass {}

class classWithNoImplField {
    public fld: NoImpClass | undefined = undefined
    public static staticFld: NoImpClass | Number = 11
}

function testInstanceUnresolvedField() {
    const klass = Class.from<classWithNoImplField>()
    const fld = klass.getInstanceField("fld") 
    const staticFld = klass.getStaticField("staticFld")

    arktest.expectThrow(() => {
        const tp = fld!.getType()
    }, (e: Error) => { return e instanceof LinkerUnresolvedClassError; });

    arktest.expectThrow(() => {
        staticFld!.getValue()
    }, (e: Error) => { return e instanceof LinkerUnresolvedClassError; });

    arktest.expectThrow(() => {
        staticFld!.setValue(1)
    }, (e: Error) => { return e instanceof LinkerUnresolvedClassError; });
}

function testInitialization() {
    const klass = Class.from<ThrowOnInit>()
    const staticFld = klass.getStaticField("staticFld")

    arktest.expectThrow(() => {
        staticFld!.getValue()
    }, (e: Error) => { return e.message == 'Class initialization;'; });
}

function testGeneralBehaviorField() {
    testInstanceUnresolvedField()
    testInitialization()
}

function main() {
    const suite = new arktest.ArkTestsuite('Reflect Field Unresolved tests')
    suite.addTest('Test Reflect general behavior Field APIs', testGeneralBehaviorField)

    return suite.run()
}
