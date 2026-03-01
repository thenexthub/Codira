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
package esvalue_to_promise_test_ets;

async function checkToPromise(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let sleepRetNumber = module.getProperty('sleepRetNumber');
    let p = sleepRetNumber.invoke(ESValue.wrapNumber(5000)).toPromise();
    let res = await p;

    arktest.assertTrue(res.toNumber() === 0xcafe);
}

async function checkToNumber(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let getPromiseNumber = module.getProperty('getPromiseNumber');
    let p = getPromiseNumber.invoke().toPromise();
    let res = false;
    await p.then((value: ESValue) => {
        res = value.toNumber() === 42;
    }).catch((e: Error) => {
        res = false;
    });
    arktest.assertTrue(res);
}

async function checkToString(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let getPromiseString = module.getProperty('getPromiseString');
    let p = getPromiseString.invoke().toPromise();
    let res = false;
    await p.then((value: ESValue) => {
        res = value.toString() === ' abc ';
    }).catch((e: Error) => {
        res = false;
    });
    arktest.assertTrue(res);
}

async function checkToBoolean(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let getPromiseBoolean = module.getProperty('getPromiseBoolean');
    let p = getPromiseBoolean.invoke().toPromise();
    let res = false;
    await p.then((value: ESValue) => {
        res = value.toBoolean();
    }).catch((e: Error) => {
        res = false;
    });
    arktest.assertTrue(res);
}

async function checkToBigInt(): Promise<void> {
    let module = ESValue.load('esvalue_to_promise_test');
    let getPromiseBigInt = module.getProperty('getPromiseBigInt');
    let p = getPromiseBigInt.invoke().toPromise();
    let res = false;
    await p.then((value: ESValue) => {
        res = value.toBigInt() === 123456789n;
    }).catch((e: Error) => {
        res = false;
    });
    arktest.assertTrue(res);
}
