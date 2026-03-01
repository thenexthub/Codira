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

class ProxyHandler extends proxy.DefaultReadonlyArrayProxyHandler<string> {
    getTrapped = false

    override get(target: ReadonlyArray<string>, index: int): string {
        this.getTrapped = true
        return super.get(target, index)
    }
}

function testReadonlyArrayProxyOps(): void {
    const array: ReadonlyArray<string> = Array.of<string>("I", "II", "III")
    const handler = new ProxyHandler()
    const arrayProxy = proxy.Proxy.create(array, handler)

    arktest.assertEQ(arrayProxy.length, 3, "unexpected ReadonlyArray proxy length")
    arktest.assertEQ(arrayProxy[0], "I", "ReadonlyArray proxy first element unexpected value")
    arktest.assertTrue(handler.getTrapped)
}

class ReadonlyArrayProxyHandler extends proxy.DefaultReadonlyArrayProxyHandler<number> {}

function testReadonlyArrayProxyLastIndexOf(): void {
    let array: ReadonlyArray<number> = [1, 2, 3, 2, 4];
    const arrayProxy = proxy.Proxy.create(array, new ReadonlyArrayProxyHandler());
    let res = arrayProxy.lastIndexOf(2);
    arktest.assertEQ(res, 3);

    res = arrayProxy.lastIndexOf(2, arrayProxy.length);
    arktest.assertEQ(res, 3);

    res = arrayProxy.lastIndexOf(2, 2);
    arktest.assertEQ(res, 1);

    res = arrayProxy.lastIndexOf(2, undefined);
    arktest.assertEQ(res, -1);
}

class ArrayProxyHandler extends proxy.DefaultArrayProxyHandler<number> {}

function testArrayProxyLastIndexOf(): void {
    let array: number[] = [1, 2, 3, 2, 4];
    const arrayProxy = proxy.Proxy.create(array, new ArrayProxyHandler());
    let res = arrayProxy.lastIndexOf(2);
    arktest.assertEQ(res, 3);

    res = arrayProxy.lastIndexOf(2, arrayProxy.length);
    arktest.assertEQ(res, 3);

    res = arrayProxy.lastIndexOf(2, 2);
    arktest.assertEQ(res, 1);

    res = arrayProxy.lastIndexOf(2, undefined);
    arktest.assertEQ(res, -1);
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Proxy API");

    suite.addTest("ReadonlyArray proxy operations", testReadonlyArrayProxyOps);
    suite.addTest("ReadonlyArray Proxy lastIndexOf", testReadonlyArrayProxyLastIndexOf);
    suite.addTest("Array Proxy lastIndexOf", testArrayProxyLastIndexOf);

    return suite.run();
}
