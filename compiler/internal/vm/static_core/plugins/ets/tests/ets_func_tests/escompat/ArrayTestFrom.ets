/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

const success : int = 0;
const fail    : int = 1;


function main(): int {
    const suite = new arktest.ArkTestsuite('Array .from API tests ')

    suite.addTest('Test method from builtin array and () => U callback', testFromBuiltinArray0);
    suite.addTest('Test method from builtin array and (value: T) => U callback', testFromBuiltinArray1);
    suite.addTest('Test method from builtin array and (value: T, index: int) => U callback', testFromBuiltinArray2);

    return suite.run()
}

type cbFn0<U> = () => U
type cbFn1<T, U> = (v: T) => U
type cbFn2<T, U> = (v: T, k: int) => U
type cbFn<T, U> = cbFn2<T, U>
type NumberToStringFn = cbFn<Number, String>
type NumberToStringCase = [Number[], NumberToStringFn, String[]]


function testF(
    args: NumberToStringCase,
    fromCallFn: (arr: Number[], cb: NumberToStringFn) => Array<String>
): void {
    const expected = args[2]
    const a = fromCallFn(args[0], args[1])
    console.log(expected, a);
    arktest.assertEQ(a.length, expected.length, 'Source and final array length mismatch')
    let r = success
    for (let i = 0; i < a.length; ++i) {
        arktest.assertEQ(
            a[i], expected[i],
             `Array values mismatch at ${i} index. Expected "${expected[i]}", got "${a[i]}"`
        );
    }
}


const testFromBuiltinArray0 = parametrize<NumberToStringCase>(
    'testBuiltinArray0',
    [
        [
            [new Number(1), new Number(2)],
            (): String => 'foo',
            ['foo', 'foo']
        ] as NumberToStringCase,
    ],
    (args: NumberToStringCase) =>
        testF(args, (arr: Number[], cb: NumberToStringFn) =>
            Array.from<Number, String>(arr, cb as cbFn0<String>))
)

const testFromBuiltinArray1 = parametrize<NumberToStringCase>(
    'testBuiltinArray1',
    [
        [
            [new Number(1), new Number(2)],
            (x: Number): String => x.toString(),
            ['1', '2']
        ] as NumberToStringCase,
    ],
    (args: NumberToStringCase) =>
        testF(args, (arr: Number[], cb: NumberToStringFn) =>
            Array.from<Number, String>(arr, cb as cbFn1<Number, String>))
)

const testFromBuiltinArray2 = parametrize<NumberToStringCase>(
    'testBuiltinArray2',
    [
        [
            [new Number(1), new Number(2)],
            (x: Number, i: int): String => `${x}:${i}`,
            ['1:0', '2:1']
        ] as NumberToStringCase,
    ],
    (args: NumberToStringCase) =>
        testF(args, (arr: Number[], cb: NumberToStringFn) =>
            Array.from<Number, String>(arr, cb as cbFn2<Number, String>))
)


function parametrize<T>(name: string, inputs: T[], cb: (x: T) => void): () => void {
    const inner = (): void => {
        for (let i = 0; i < inputs.length; ++i) {
            cb(inputs[i])
        }
    }
    return inner
}
