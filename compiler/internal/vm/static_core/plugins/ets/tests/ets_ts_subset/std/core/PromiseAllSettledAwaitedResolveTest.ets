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

let failed = false;
let done = 0;

function fail(msg: string): void {
    if (failed) {
        return;
    }
    failed = true;
    console.log('Test failed. ' + msg);
}

function doneOk(): void {
    if (failed) {
        return;
    }
    done++;
    if (done == 5) {
        console.log('Test passed.');
    }
}

async function regressionAllSettledAwaitedTyping(): Promise<void> {
    const results: PromiseSettledResult<number>[] = await Promise.allSettled([
        Promise.resolve(33.0),
        new Promise<number>((resolve) => { resolve(66.0) }),
    ]);
    let v0 = (results[0] as PromiseFulfilledResult<number>).value;
    let v1 = (results[1] as PromiseFulfilledResult<number>).value;
    if (v0 + v1 == -1) {
        console.log('unreachable');
    }
}

let awaitedAllSettledP1 = Promise.resolve(33.0);
let awaitedAllSettledP2 = new Promise<number>((resolve) => { resolve(66) });

Promise.allSettled<number>([awaitedAllSettledP1, awaitedAllSettledP2]).then<void, void>((values): void => {
    if (values.length != 2) {
        fail('Expected an array of length 2 but got length ' + values.length + '.');
        return;
    }
    if (values[0].status != 'fulfilled' || values[1].status != 'fulfilled') {
        fail('Expected both elements have status \'fulfilled\'.');
        return;
    }

    let v0 = (values[0] as PromiseFulfilledResult<number>).value;
    let v1 = (values[1] as PromiseFulfilledResult<number>).value;

    if (v0 != 33.0) {
        fail('Unexpected value of the element 0: ' + v0 + '.');
        return;
    }
    if (v1 != 66) {
        fail('Unexpected value of the element 1: ' + v1 + '.');
        return;
    }

    let sum = v0 + v1;
    if (sum != 99.0) {
        fail('Unexpected sum: ' + sum + '.');
        return;
    }

    doneOk();
}, (_: Error): void => {
    fail('The promise should not be rejected.');
}).catch((e: Error): void => {
    fail('Caught error: ' + e);
});

let awaitedAllSettledP3: Promise<number> = Promise.resolve(77.0);
Promise.allSettled<number>([awaitedAllSettledP3]).then<void, void>((values): void => {
    if (values.length != 1) {
        fail('Expected an array of length 1 but got length ' + values.length + '.');
        return;
    }
    if (values[0].status != 'fulfilled') {
        fail('Expected element 0 has status \'fulfilled\'.');
        return;
    }
    let v = (values[0] as PromiseFulfilledResult<number>).value;
    if (v != 77.0) {
        fail('Unexpected value of the element 0: ' + v + '.');
        return;
    }
    let check = v + 1;
    if (check != 78.0) {
        fail('Unexpected check value: ' + check + '.');
        return;
    }
    doneOk();
}, (_: Error): void => {
    fail('The promise should not be rejected.');
}).catch((e: Error): void => {
    fail('Caught error: ' + e);
});

class TwoNumberIterator implements Iterator<number | PromiseLike<number>> {
    next(): IteratorResult<number | PromiseLike<number>> {
        if (this.idx == 0) {
            this.idx++;
            return { done: false, value: Promise.resolve(33.0) } as IteratorResult<number | PromiseLike<number>>;
        }
        if (this.idx == 1) {
            this.idx++;
            return {
                done: false,
                value: new Promise<number>((resolve) => { resolve(66) })
            } as IteratorResult<number | PromiseLike<number>>;
        }
        return { done: true, value: undefined } as IteratorResult<number | PromiseLike<number>>;
    }
    private idx: number = 0;
}

class TwoNumberIterable implements Iterable<number | PromiseLike<number>> {
    [Symbol.iterator](): Iterator<number | PromiseLike<number>> {
        return new TwoNumberIterator();
    }
}

Promise.allSettled<number>(new TwoNumberIterable()).then<void, void>((values): void => {
    if (values.length != 2) {
        fail('Expected an array of length 2 but got length ' + values.length + '.');
        return;
    }
    if (values[0].status != 'fulfilled' || values[1].status != 'fulfilled') {
        fail('Expected both elements have status \'fulfilled\'.');
        return;
    }
    let v0 = (values[0] as PromiseFulfilledResult<number>).value;
    let v1 = (values[1] as PromiseFulfilledResult<number>).value;
    if (v0 != 33.0 || v1 != 66) {
        fail('Unexpected values from Iterable overload: ' + v0 + ', ' + v1 + '.');
        return;
    }
    doneOk();
}, (_: Error): void => {
    fail('The promise should not be rejected.');
}).catch((e: Error): void => {
    fail('Caught error: ' + e);
});

let mixErr = new Error('mix rejected');
let mixP1 = Promise.resolve(10.0);
let mixP2 = Promise.reject<number>(mixErr);
let mixP3 = new Promise<number>((resolve) => { resolve(30) });
Promise.allSettled<number>([mixP1, mixP2, mixP3]).then<void, void>((values): void => {
    if (values.length != 3) {
        fail('Expected an array of length 3 but got length ' + values.length + '.');
        return;
    }
    if (values[0].status != 'fulfilled' || values[1].status != 'rejected' || values[2].status != 'fulfilled') {
        fail('Unexpected status for mix case.');
        return;
    }
    let v0 = (values[0] as PromiseFulfilledResult<number>).value;
    let v2 = (values[2] as PromiseFulfilledResult<number>).value;
    if (v0 != 10.0 || v2 != 30) {
        fail('Unexpected fulfilled values for mix case: ' + v0 + ', ' + v2 + '.');
        return;
    }
    let reason = (values[1] as PromiseRejectedResult).reason;
    if (reason != mixErr) {
        fail('Unexpected rejection reason for mix case.');
        return;
    }
    doneOk();
}, (_: Error): void => {
    fail('The promise should not be rejected.');
}).catch((e: Error): void => {
    fail('Caught error: ' + e);
});

class MixIterator implements Iterator<number | PromiseLike<number>> {
    next(): IteratorResult<number | PromiseLike<number>> {
        if (this.idx == 0) {
            this.idx++;
            return { done: false, value: Promise.resolve(10.0) } as IteratorResult<number | PromiseLike<number>>;
        }
        if (this.idx == 1) {
            this.idx++;
            return {
                done: false,
                value: Promise.reject<number>(mixErr),
            } as IteratorResult<number | PromiseLike<number>>;
        }
        if (this.idx == 2) {
            this.idx++;
            return {
                done: false,
                value: new Promise<number>((resolve) => {
                    resolve(30);
                }),
            } as IteratorResult<number | PromiseLike<number>>;
        }
        return { done: true, value: undefined } as IteratorResult<number | PromiseLike<number>>;
    }
    private idx: number = 0;
}

class MixIterable implements Iterable<number | PromiseLike<number>> {
    [Symbol.iterator](): Iterator<number | PromiseLike<number>> {
        return new MixIterator();
    }
}

Promise.allSettled<number>(new MixIterable()).then<void, void>((values): void => {
    if (values.length != 3) {
        fail('Expected an array of length 3 but got length ' + values.length + '.');
        return;
    }
    if (values[0].status != 'fulfilled' || values[1].status != 'rejected' || values[2].status != 'fulfilled') {
        fail('Unexpected status for Iterable mix case.');
        return;
    }
    let v0 = (values[0] as PromiseFulfilledResult<number>).value;
    let v2 = (values[2] as PromiseFulfilledResult<number>).value;
    if (v0 != 10.0 || v2 != 30) {
        fail('Unexpected fulfilled values for Iterable mix case: ' + v0 + ', ' + v2 + '.');
        return;
    }
    let reason = (values[1] as PromiseRejectedResult).reason;
    if (reason != mixErr) {
        fail('Unexpected rejection reason for Iterable mix case.');
        return;
    }
    doneOk();
}, (_: Error): void => {
    fail('The promise should not be rejected.');
}).catch((e: Error): void => {
    fail('Caught error: ' + e);
});
