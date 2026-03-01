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
class AnyTestClass {
    public isNull(obj: Any): void {
        arktest.assertEQ(obj, null);
    }

    public static isNull(obj: Any): void {
        arktest.assertEQ(obj, null);
    }

    public isRestNull(...args: Any[]): void {
        arktest.assertEQ(args[0], null);
    }

    public static isRestNull(...args: Any[]): void {
        arktest.assertEQ(args[0], null);
    }
}

function anyNullTest() {
    let ct = Type.from<AnyTestClass>() as ClassType;
    let obj = new AnyTestClass();
    let num = ct.getMethodsNum();
    for (let i = 0; i < num; i++) {
        let method = ct.getMethod(i);
        if (method.getName() == 'isNull') {
            method.invoke(method.isStatic() ? null : obj, [null]);
        }

        if (method.getName() == 'isRestNull') {
            let args = [null];
            method.invoke(method.isStatic() ? null : obj, [args]);
        }
    }
}

class ObjectTestClass {
    public isNull(obj: Object): void {
        arktest.assertEQ(obj, null);
    }

    public static isNull(obj: Object): void {
        arktest.assertEQ(obj, null);
    }

    public isRestNull(...args: Object[]): void {
        console.log(args[0]);
    }

    public static isRestNull(...args: Object[]): void {
        console.log(args[0]);
    }
}

function objectNullTest() {
    let ct = Type.from<ObjectTestClass>() as ClassType;
    let obj = new ObjectTestClass();
    let num = ct.getMethodsNum();
    for (let i = 0; i < num; i++) {
        let method = ct.getMethod(i);
        if (method.getName() == 'isNull') {
            method.invoke(method.isStatic() ? null : obj, [null]);
        }

        if (method.getName() == 'isRestNull') {
            let args = [null];
            arktest.expectThrow(() => { method.invoke(method.isStatic() ? null : obj, [args]) }, (e) => e instanceof ClassCastError)
        }
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Method invoke');
    suite.addTest('Any for null', anyNullTest);
    suite.addTest('Object for null', objectNullTest);
    return suite.run();
}
