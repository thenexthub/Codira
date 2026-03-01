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

class TestClass {}

interface TestInterface {}

enum TestEnum {
    False = 0,
    True = 1
}

class UndefinedUnionTypeTestClass {
    public charUndefined(arg: char | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public booleanUndefined(arg: boolean | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public byteUndefined(arg: byte | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public shortUndefined(arg: short | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public intUndefined(arg: int | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public longUndefined(arg: long | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public floatUndefined(arg: float | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public doubleUndefined(arg: double | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public stringUndefined(arg: string | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public classUndefined(arg: TestClass | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public interfaceUndefined(arg: TestInterface | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public arrayUndefined(arg: Array<int> | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public fixedArrayUndefined(arg: FixedArray<number> | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public tuplefaceUndefined(arg: [string, int, number] | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public functionUndefined(arg: (a: int) => boolean | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public unionUndefined(arg: int | double | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public enumUndefined(arg: TestEnum | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public anyUndefined(arg: Any | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static charUndefined(arg: char | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static booleanUndefined(arg: boolean | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static byteUndefined(arg: byte | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static shortUndefined(arg: short | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static intUndefined(arg: int | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static longUndefined(arg: long | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static floatUndefined(arg: float | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static doubleUndefined(arg: double | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static stringUndefined(arg: string | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static classUndefined(arg: TestClass | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static interfaceUndefined(arg: TestInterface | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static arrayUndefined(arg: Array<int> | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static fixedArrayUndefined(arg: FixedArray<number> | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static tuplefaceUndefined(arg: [string, int, number] | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static functionUndefined(arg: (a: int) => boolean | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static unionUndefined(arg: int | double | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static enumUndefined(arg: TestEnum | undefined): int {
        return arg === undefined ? 1 : 0;
    }

    public static anyUndefined(arg: Any | undefined): int {
        return arg === undefined ? 1 : 0;
    }
}

function undefinedUnionTypeTest() {
    let obj = new UndefinedUnionTypeTestClass();
    let ct = Type.from<UndefinedUnionTypeTestClass>() as ClassType;
    let num = ct.getMethodsNum();
    for (let i = 0; i < num; i++) {
        let method = ct.getMethod(i);
        if (method.getName().contains('Undefined', 0)) {
            arktest.assertEQ(method.invoke(method.isStatic() ? null : obj, [undefined]), 1);
        }
    }
}

class ReferenceTypeTestClass {
    public stringWithUndefined(arg: string): int {
        return arg === undefined ? 1 : 0;
    }

    public anyWithUndefined(arg: Any): int {
        return arg === undefined ? 1 : 0;
    }

    public arrayWithUndefined(arg: Array<int>): int {
        return arg === undefined ? 1 : 0;
    }

    public fixedArrayWithUndefined(arg: FixedArray<int>): int {
        return arg === undefined ? 1 : 0;
    }

    public tupleWithUndefined(arg: [int, double, byte]): int {
        return arg === undefined ? 1 : 0;
    }

    public recordWithUndefined(arg: Record<string, double>): int {
        return arg === undefined ? 1 : 0;
    }

    public mapWithUndefined(arg: Map<int, int>): int {
        return arg === undefined ? 1 : 0;
    }

    public setWithUndefined(arg: Set<int>): int {
        return arg === undefined ? 1 : 0;
    }

    public restWithUndefined<T>(...args: T[]): int {
        return args === undefined ? 1 : 0;
    }

    public enumWithUndefined(arg: TestEnum): int {
        return arg === undefined ? 1 : 0;
    }

    public classWithUndefined(arg: TestClass): int {
        return arg === undefined ? 1 : 0;
    }

    public interfaceWithUndefined(arg: TestInterface): int {
        return arg === undefined ? 1 : 0;
    }

    public unionWithUndefined(arg: int | double): int {
        return arg === undefined ? 1 : 0;
    }

    public static stringWithUndefined(arg: string): int {
        return arg === undefined ? 1 : 0;
    }

    public static anyWithUndefined(arg: Any): int {
        return arg === undefined ? 1 : 0;
    }

    public static arrayWithUndefined(arg: Array<int>): int {
        return arg === undefined ? 1 : 0;
    }

    public static fixedArrayWithUndefined(arg: FixedArray<int>): int {
        return arg === undefined ? 1 : 0;
    }

    public static tupleWithUndefined(arg: [int, double, byte]): int {
        return arg === undefined ? 1 : 0;
    }

    public static recordWithUndefined(arg: Record<string, double>): int {
        return arg === undefined ? 1 : 0;
    }

    public static mapWithUndefined(arg: Map<int, int>): int {
        return arg === undefined ? 1 : 0;
    }

    public static setWithUndefined(arg: Set<int>): int {
        return arg === undefined ? 1 : 0;
    }

    public static restWithUndefined<T>(...args: T[]): int {
        return args === undefined ? 1 : 0;
    }

    public static enumWithUndefined(arg: TestEnum): int {
        return arg === undefined ? 1 : 0;
    }

    public static classWithUndefined(arg: TestClass): int {
        return arg === undefined ? 1 : 0;
    }

    public static interfaceWithUndefined(arg: TestInterface): int {
        return arg === undefined ? 1 : 0;
    }

    public static unionWithUndefined(arg: int | double): int {
        return arg === undefined ? 1 : 0;
    }

    public restTest(...args: string[]): int {
        return args[0] == 'a' && args[1] === undefined && args[2] == 'b' ? 1 : 0;
    }

    public static restTest(...args: string[]): int {
        return args[0] == 'a' && args[1] === undefined && args[2] == 'b' ? 1 : 0;
    }
}

function referenceTypeTest() {
    let obj = new ReferenceTypeTestClass();
    let ct = Type.from<ReferenceTypeTestClass>() as ClassType;
    let num = ct.getMethodsNum();
    for (let i = 0; i < num; i++) {
        let method = ct.getMethod(i);
        if (method.getName().contains('Undefined', 0)) {
            arktest.assertEQ(method.invoke(method.isStatic() ? null : obj, [undefined]), 1);
        }

        if (method.getName() == 'restTest') {
            arktest.assertEQ(method.invoke(method.isStatic() ? null : obj, ['a', undefined, 'b']), 1);
        }
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Method invoke');
    suite.addTest('Union type for undefined test', undefinedUnionTypeTest);
    suite.addTest('Reference type for undefined test', undefinedUnionTypeTest);
    return suite.run();
}
