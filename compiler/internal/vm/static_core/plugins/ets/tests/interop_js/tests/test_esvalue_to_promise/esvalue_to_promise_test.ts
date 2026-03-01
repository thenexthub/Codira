/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
let etsVm = globalThis.gtest.etsVm;
let checkToPromise = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkToPromise');
let checkToNumber = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkToNumber');
let checkToString = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkToString');
let checkToBoolean = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkToBoolean');
let checkToBigInt = etsVm.getFunction('Lesvalue_to_promise_test_ets/ETSGLOBAL;', 'checkToBigInt');

export function sleep(ms: number): Promise<void> {
    return new Promise<void>((resolve) => {
        setTimeout(resolve, ms);
    });
}

export async function sleepRetNumber(ms: number): Promise<number> {
    await sleep(ms);
    return 0xcafe;
}

export async function getPromiseNumber(): Promise<number> {
    return Promise.resolve(42);
}

export async function getPromiseString(): Promise<string> {
    return Promise.resolve(' abc ');
}

export async function getPromiseBoolean(): Promise<boolean> {
    return Promise.resolve(true);
}

export async function getPromiseBigInt(): Promise<bigint> {
    return Promise.resolve(123456789n);
}

async function main(): Promise<void> {
    await checkToPromise();
    await checkToNumber();
    await checkToString();
    await checkToBoolean();
    await checkToBigInt();
}

main();