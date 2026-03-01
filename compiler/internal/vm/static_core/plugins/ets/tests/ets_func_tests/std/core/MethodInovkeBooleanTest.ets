/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

class MethodTestClass {
    uppercaseBooleanTest(b: Boolean): Boolean {
        return b;
    }

    static uppercaseBooleanTest(b: Boolean): Boolean {
        return b;
    }

    lowercaseBooleanTest(b: boolean): boolean {
        return b;
    }

    static lowercaseBooleanTest(b: boolean): boolean {
        return b;
    }

    restUppercaseBooleanTest(...args: Boolean[]): string {
        let len = args.length;
        let s = '';
        for (let i = 0; i < len; i++) {
            s += args[i].toString();
        }
        return s;
    }

    static restLowercaseBooleanTest(...args: boolean[]): string {
        let len = args.length;
        let s = '';
        for (let i = 0; i < len; i++) {
            s += args[i].toString();
        }
        return s;
    }
}

function testBoolean(): void {
    let obj = new MethodTestClass();
    let ct = Type.from<MethodTestClass>() as ClassType;
    let num = ct.getMethodsNum();
    for (let i = 0; i < num; i++) {
        let method = ct.getMethod(i);
        if (method.getName().endsWith('BooleanTest') && !method.getName().startsWith('rest')) {
            let ret = method.invoke(method.isStatic() ? null : obj, [false]);
            arktest.assertFalse(ret as boolean);
            ret = method.invoke(method.isStatic() ? null : obj, [new Boolean(true)]);
            arktest.assertTrue(ret as boolean);
        }
    }
}

function testRestBoolean(): void {
    let obj = new MethodTestClass();
    let ct = Type.from<MethodTestClass>() as ClassType;
    let num = ct.getMethodsNum();
    let args = [true, new Boolean(true), false, new Boolean(false)];
    let result = 'truetruefalsefalse';
    for (let i = 0; i < num; i++) {
        let method = ct.getMethod(i);
        if (method.getName().startsWith('rest')) {
            let ret = method.invoke(method.isStatic() ? null : obj, [args]);
            arktest.assertEQ(result, ret);
        }
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Method invoke');
    suite.addTest('for boolean', testBoolean);
    suite.addTest('for rest boolean', testRestBoolean);
    return suite.run();
}
