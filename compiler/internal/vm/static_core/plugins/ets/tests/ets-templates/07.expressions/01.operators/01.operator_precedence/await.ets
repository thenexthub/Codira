/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

/*---
desc: await with other operators
---*/

async function fetchBool(val: boolean): Promise<boolean> {
    return val;
}

async function fetchInt(i: int): Promise<int> {
    const results = await Promise.all(new Array<int>());
    if (0 == results.length) return i;
    return 100500;
}

class PromiseValue {
    x: number = 42;
}

class PromiseHolder {

    async fetchValue(): Promise<PromiseValue> {
        const results = await Promise.all(new Array<PromiseValue>());
        if (0 == results.length) return new PromiseValue();
        return new PromiseValue();
    }

}

async function testDotChainAndCast() {
    let p = new PromiseHolder()
    let result = await p.fetchValue();
    arktest.assertEQ(result.x, 42);
    result = await (p?.fetchValue() as Promise<PromiseValue>);
    arktest.assertEQ(result.x, 42);
}

async function testInstanceof() {
    let p = new PromiseHolder()
    const result = await p.fetchValue() instanceof Object;
    arktest.assertTrue(result === true);
}

async function testNullCoalesce() {
    let p = new PromiseHolder();
    let q = new PromiseValue();
    q.x = 13;
    const result = await p.fetchValue() ?? q;
    arktest.assertEQ(result.x, 42);
}

async function testArithmetic() {
    let result = await fetchInt(2) - await fetchInt(1) + await fetchInt(4) * await fetchInt(3) / await fetchInt(6);
    arktest.assertEQ(result, (2 - 1 + 4 * 3 / 6));  // 3
    result += await fetchInt(1);
    result -= await fetchInt(2);
    result *= await fetchInt(3);
    result /= await fetchInt(6);  // 1
    arktest.assertEQ(result, (3 + 1 - 2) * 3 / 6);
}

async function testBoolean() {
    let result = await fetchBool(true) && await fetchBool(false) || await fetchBool(true);
    arktest.assertTrue(result);
    result = await fetchInt(13) == await fetchInt(13) && result;
    arktest.assertTrue(result);
    result = await fetchInt(13) != 100500 && result;
    arktest.assertTrue(result);
    result = ! await fetchBool(false);
    arktest.assertTrue(result);
    let result1 = await fetchBool(false) ? await fetchInt(5) : await fetchInt(6)
    arktest.assertEQ(result1, 6);
}

function main(): void {
    waitForCompletion(testDotChainAndCast);
    waitForCompletion(testInstanceof);
    waitForCompletion(testNullCoalesce);
    waitForCompletion(testArithmetic);
    waitForCompletion(testBoolean);
}
