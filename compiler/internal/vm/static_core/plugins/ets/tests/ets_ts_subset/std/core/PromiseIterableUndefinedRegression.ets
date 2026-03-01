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
const total = 8;

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
    if (done == total) {
        console.log('Test passed.');
    }
}

class AllUndefIterator implements Iterator<string | undefined | PromiseLike<string | undefined>> {
    private idx: number = 0;
    next(): IteratorResult<string | undefined | PromiseLike<string | undefined>> {
        if (this.idx == 0) {
            this.idx++;
            return { done: false, value: 'a' } as IteratorResult<string | undefined | PromiseLike<string | undefined>>;
        }
        if (this.idx == 1) {
            this.idx++;
            // Regression: a valid element value is undefined (done=false).
            return { done: false, value: undefined } as IteratorResult<
                string | undefined | PromiseLike<string | undefined>
            >;
        }
        if (this.idx == 2) {
            this.idx++;
            return { done: false, value: Promise.resolve<string | undefined>('b') } as IteratorResult<
                string | undefined | PromiseLike<string | undefined>
            >;
        }
        return { done: true, value: undefined } as IteratorResult<string | undefined | PromiseLike<string | undefined>>;
    }
}

class AllUndefIterable implements Iterable<string | undefined | PromiseLike<string | undefined>> {
    [Symbol.iterator](): Iterator<string | undefined | PromiseLike<string | undefined>> {
        return new AllUndefIterator();
    }
}

Promise.all<string | undefined>(new AllUndefIterable()).then<void, void>((values): void => {
    if (values.length != 3) {
        fail('Promise.all(iterable, undef) expected length 3 but got ' + values.length + '.');
        return;
    }
    if (values[0] != 'a' || values[1] != undefined || values[2] != 'b') {
        fail(
            'Promise.all(iterable, undef) unexpected values: ' +
                values[0] + ', ' + values[1] + ', ' + values[2] + '.'
        );
        return;
    }
    doneOk();
}, (e: Error): void => {
    fail('Promise.all(iterable, undef) should not reject: ' + e);
}).catch((e: Error): void => {
    fail('Promise.all(iterable, undef) caught error: ' + e);
});

Promise.all<number>([Promise.resolve<number>(1), 2, Promise.resolve<number>(3)]).then<void, void>((values): void => {
    if (values.length != 3 || values[0] != 1 || values[1] != 2 || values[2] != 3) {
        fail('Promise.all(normal) unexpected values: ' + values);
        return;
    }
    doneOk();
}, (e: Error): void => {
    fail('Promise.all(normal) should not reject: ' + e);
}).catch((e: Error): void => {
    fail('Promise.all(normal) caught error: ' + e);
});

class AllSettledUndefIterator implements Iterator<number | undefined | PromiseLike<number | undefined>> {
    private idx: number = 0;
    next(): IteratorResult<number | undefined | PromiseLike<number | undefined>> {
        if (this.idx == 0) {
            this.idx++;
            return { done: false, value: Promise.resolve<number | undefined>(10) } as IteratorResult<
                number | undefined | PromiseLike<number | undefined>
            >;
        }
        if (this.idx == 1) {
            this.idx++;
            return { done: false, value: undefined } as IteratorResult<
                number | undefined | PromiseLike<number | undefined>
            >;
        }
        return { done: true, value: undefined } as IteratorResult<number | undefined | PromiseLike<number | undefined>>;
    }
}

class AllSettledUndefIterable implements Iterable<number | undefined | PromiseLike<number | undefined>> {
    [Symbol.iterator](): Iterator<number | undefined | PromiseLike<number | undefined>> {
        return new AllSettledUndefIterator();
    }
}

Promise.allSettled<number | undefined>(new AllSettledUndefIterable()).then<void, void>((values): void => {
    if (values.length != 2) {
        fail('Promise.allSettled(iterable, undef) expected length 2 but got ' + values.length + '.');
        return;
    }
    if (values[0].status != 'fulfilled' || values[1].status != 'fulfilled') {
        fail('Promise.allSettled(iterable, undef) expected both fulfilled.');
        return;
    }
    let v0 = (values[0] as PromiseFulfilledResult<number | undefined>).value;
    let v1 = (values[1] as PromiseFulfilledResult<number | undefined>).value;
    if (v0 != 10 || v1 != undefined) {
        fail('Promise.allSettled(iterable, undef) unexpected values: ' + v0 + ', ' + v1 + '.');
        return;
    }
    doneOk();
}, (e: Error): void => {
    fail('Promise.allSettled(iterable, undef) should not reject: ' + e);
}).catch((e: Error): void => {
    fail('Promise.allSettled(iterable, undef) caught error: ' + e);
});

let settledErr = new Error('settled reject');
Promise.allSettled<number>([
    Promise.resolve<number>(1),
    Promise.reject<number>(settledErr)
]).then<void, void>((values): void => {
    if (values.length != 2) {
        fail('Promise.allSettled(normal) expected length 2 but got ' + values.length + '.');
        return;
    }
    if (values[0].status != 'fulfilled' || values[1].status != 'rejected') {
        fail('Promise.allSettled(normal) unexpected status.');
        return;
    }
    let v0 = (values[0] as PromiseFulfilledResult<number>).value;
    let r1 = (values[1] as PromiseRejectedResult).reason;
    if (v0 != 1 || r1 != settledErr) {
        fail('Promise.allSettled(normal) unexpected values.');
        return;
    }
    doneOk();
}, (e: Error): void => {
    fail('Promise.allSettled(normal) should not reject: ' + e);
}).catch((e: Error): void => {
    fail('Promise.allSettled(normal) caught error: ' + e);
});

class AnyUndefFirstIterator implements Iterator<string | undefined | PromiseLike<string | undefined>> {
    private idx: number = 0;
    next(): IteratorResult<string | undefined | PromiseLike<string | undefined>> {
        if (this.idx == 0) {
            this.idx++;
            return { done: false, value: undefined } as IteratorResult<
                string | undefined | PromiseLike<string | undefined>
            >;
        }
        if (this.idx == 1) {
            this.idx++;
            return { done: false, value: Promise.resolve<string | undefined>('x') } as IteratorResult<
                string | undefined | PromiseLike<string | undefined>
            >;
        }
        return { done: true, value: undefined } as IteratorResult<string | undefined | PromiseLike<string | undefined>>;
    }
}

class AnyUndefFirstIterable implements Iterable<string | undefined | PromiseLike<string | undefined>> {
    [Symbol.iterator](): Iterator<string | undefined | PromiseLike<string | undefined>> {
        return new AnyUndefFirstIterator();
    }
}

Promise.any<string | undefined>(new AnyUndefFirstIterable()).then<void, void>((value): void => {
    if (value != undefined) {
        fail('Promise.any(iterable, undef) expected undefined but got ' + value + '.');
        return;
    }
    doneOk();
}, (e: Error): void => {
    fail('Promise.any(iterable, undef) should not reject: ' + e);
}).catch((e: Error): void => {
    fail('Promise.any(iterable, undef) caught error: ' + e);
});

Promise.any<string>(['a', Promise.resolve('b')]).then<void, void>((value): void => {
    if (value != 'a') {
        fail('Promise.any(normal) expected a but got ' + value + '.');
        return;
    }
    doneOk();
}, (e: Error): void => {
    fail('Promise.any(normal) should not reject: ' + e);
}).catch((e: Error): void => {
    fail('Promise.any(normal) caught error: ' + e);
});

class RaceUndefFirstIterator implements Iterator<number | undefined | PromiseLike<number | undefined>> {
    private idx: number = 0;
    next(): IteratorResult<number | undefined | PromiseLike<number | undefined>> {
        if (this.idx == 0) {
            this.idx++;
            return { done: false, value: undefined } as IteratorResult<
                number | undefined | PromiseLike<number | undefined>
            >;
        }
        if (this.idx == 1) {
            this.idx++;
            return { done: false, value: Promise.resolve<number | undefined>(123) } as IteratorResult<
                number | undefined | PromiseLike<number | undefined>
            >;
        }
        return { done: true, value: undefined } as IteratorResult<number | undefined | PromiseLike<number | undefined>>;
    }
}

class RaceUndefFirstIterable implements Iterable<number | undefined | PromiseLike<number | undefined>> {
    [Symbol.iterator](): Iterator<number | undefined | PromiseLike<number | undefined>> {
        return new RaceUndefFirstIterator();
    }
}

Promise.race<number | undefined>(new RaceUndefFirstIterable()).then<void, void>((value): void => {
    if (value != undefined) {
        fail('Promise.race(iterable, undef) expected undefined but got ' + value + '.');
        return;
    }
    doneOk();
}, (e: Error): void => {
    fail('Promise.race(iterable, undef) should not reject: ' + e);
}).catch((e: Error): void => {
    fail('Promise.race(iterable, undef) caught error: ' + e);
});

let raceResolveLater: ((v: string) => void) | null = null;
let raceP2 = new Promise<string>((resolve: (v: string) => void): void => {
    raceResolveLater = resolve;
});
Promise.race<string>([Promise.resolve<string>('fast'), raceP2]).then<void, void>((value: Object): void => {
    if (value != 'fast') {
        fail('Promise.race(normal) expected fast but got ' + value + '.');
        return;
    }
    doneOk();
}, (e: Error): void => {
    fail('Promise.race(normal) should not reject: ' + e);
}).catch((e: Error): void => {
    fail('Promise.race(normal) caught error: ' + e);
});

raceResolveLater!('slow');

