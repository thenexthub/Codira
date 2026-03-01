/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

package std.core;

/**
 * Interface for objects with 'then' method.
 */
export interface PromiseLike<out T> {
    then<U = T, E = never>(onFulfilled: (value: T) => U|PromiseLike<U>,
            onRejected?: (error: Error) => E|PromiseLike<E>): PromiseLike<Awaited<U|E>>;
}

/**
 * Class represents the promise status.
 */
export class PromiseStatus{
    static readonly fulfilled = "fulfilled";
    static readonly rejected = "rejected";
}

/**
 * Base class for Promise.allSettled return type.
 */
class PromiseSettledResultBase {
    status: string = "";
}

/**
 * The class describes fulfillment result of Promise.allSettled's argument.
 */
export class PromiseFulfilledResult<T> extends PromiseSettledResultBase {
    constructor(){}

    constructor(value: T) {
        this.status = PromiseStatus.fulfilled;
        this.value = value;
    }

    value: T;
}

/**
 * The class describes rejection result of Promise.allSettled's argument.
 */
export class PromiseRejectedResult extends PromiseSettledResultBase {
    constructor(){
        this.reason =  new Error();
    }

    constructor(reason: Error) {
        this.status = PromiseStatus.rejected;
        this.reason = reason;
    }

    reason: Error;
}

export type PromiseSettledResult<T> = PromiseFulfilledResult<T>|PromiseRejectedResult

/**
 * Class represents a result of an asynchronous operation in the future.
 */
export final class Promise<out T> implements PromiseLike<T> {
    private constructor() {
    }

    /**
     * Creates an instance of a Promise class
     *
     * @param callback callback that is called in the constructor
     * Takes resolve and reject function that would complete the promise
     */
    constructor(callback: (resolve: (value: T|PromiseLike<T>) => void,
            reject: (error: Error) => void) => void) {
        try {
            callback((value: T|PromiseLike<T>): void => {
                this.doResolve<T>(value);
            }, (error: Error): void => {
                this.rejectImpl(error as Error, false);
            });
        } catch (error) {
            // NOTE(csaba.osztrogonac): remove as cast once e has Error type
            this.rejectImpl(error as Error, false);
        }
    }

    /**
     * Schedule callback to execute after the promise is resolved
     *
     * @param onFulfilled callback that will be called after the promise is resolved
     * @returns { Promise } new promise that will be resolved after the execution of the callback
     *     or rejected if current promise is rejected or the callback finished with an exception
     */
    then<U = T>(onFulfilled: () => U|PromiseLike<U>): Promise<Awaited<U>> {
        let promise = new Promise<Awaited<U>>();
        let fn: () => void = (): void => {
            try {
                if (this.state == Promise.STATE_RESOLVED) {
                    promise.doResolve<U>(onFulfilled());
                } else {
                    promise.rejectImpl(this.value as Error, false);
                }
            } catch (error) {
                // NOTE(csaba.osztrogonac): remove as cast once e has Error type
                promise.rejectImpl(error as Error, false);
            }
        }
        this.submitCallback(fn as Object);
        return promise;
    }

    /**
     * Schedule callbacks to execute after the promise is completed
     *
     * @param onFulfilled callback that will be called after the promise is resolved
     * @param [onRejected] callback that will be called after the promise is rejected
     * @returns { Promise } new promise that will be resolved after the execution of one of the callbacks
     *     or rejected in the callback finished with an exception
     */
    then<U = T, E = never>(onFulfilled: (value: T) => U|PromiseLike<U>,
            onRejected?: (error: Error) => E|PromiseLike<E>): Promise<Awaited<U|E>> {
        let promise = new Promise<Awaited<U|E>>();
        let fn: () => void = (): void => {
            try {
                if (this.state == Promise.STATE_RESOLVED) {
                    promise.doResolve<U>(onFulfilled(this.value as T));
                } else {
                    if (onRejected) {
                        promise.doResolve<E>(onRejected(this.value as Error));
                    } else {
                        promise.rejectImpl(this.value as Error, false);
                    }
                }
            } catch (e) {
                // NOTE(csaba.osztrogonac): remove as cast once e has Error type
                promise.rejectImpl(e as Error, false);
            }
        }
        this.submitCallback(fn as Object);
        return promise;
    }

    // NOTE(audovichenko): Add '?' to onRejection param. Issue #20001
    /**
     * Schedule callback to execute after the promise is rejected
     *
     * @param onRejected callback that will be called after the promise is rejected
     * @returns { Promise } new promise that will be resolved after the execution of the callback
     *     or rejected if current promise is rejected or the callback finished with an exception
     */
    catch<U = never>(onRejected: () => U|PromiseLike<U>): Promise<Awaited<T|U>> {
        let promise = new Promise<Awaited<T|U>>();
        let fn: () => void = (): void => {
            try {
                if (this.state == Promise.STATE_REJECTED) {
                    let res: U|PromiseLike<U> = onRejected();
                    promise.doResolve<U>(res);
                } else {
                    promise.doResolve<T>(this.value as T);
                }
            } catch (e) {
                // NOTE(csaba.osztrogonac): remove as cast once e has Error type
                promise.rejectImpl(e as Error, false);
            }
        }
        this.submitCallback(fn as Object);
        return promise;
    }

    /**
     * Schedule callback to execute after the promise is rejected
     *
     * @param [onRejected] callback that will be called after the promise is rejected
     * @returns { Promise } new promise that will be resolved after the execution of the callback
     *     or rejected or the callback finished with an exception
     */
    catch<U = never>(onRejected?: (error: Error) => U|PromiseLike<U>): Promise<Awaited<T|U>> {
        let promise = new Promise<Awaited<T|U>>();
        let fn: () => void = (): void => {
            try {
                if (this.state == Promise.STATE_REJECTED) {
                    if (onRejected) {
                        let res: U|PromiseLike<U> = onRejected(this.value as Error);
                        promise.doResolve<U>(res);
                    } else {
                        promise.rejectImpl(this.value as Error, false);
                    }
                } else {
                    promise.doResolve<T>(this.value as T);
                }
            } catch (e) {
                // NOTE(csaba.osztrogonac): remove as cast once e has Error type
                promise.rejectImpl(e as Error, false);
            }
        }
        this.submitCallback(fn as Object);
        return promise;
    }

    /**
     * Schedule callback to execute after the promise is completed
     *
     * @param [onFinally] callback that will be called after the promise is completed
     * @returns { Promise } new promise that will be completed the same way as the current promise after the execution of the callback
     *     or rejected if the callback finished with an exception
     */
    finally<U = T>(onFinally?: () => U|PromiseLike<U>): Promise<Awaited<T>> {
        let promise = new Promise<Awaited<T>>();
        let fn: () => void = (): void => {
            try {
                let onFinallyResult: U|PromiseLike<U>;
                let onFinallyError: Error | undefined = undefined;

                const resolveOriginalPromise = (): void => {
                    if (this.state == Promise.STATE_RESOLVED) {
                        promise.resolveImpl(this.value as T, false);
                    } else {
                        promise.rejectImpl(this.value as Error, false);
                    }
                };

                if (onFinally) {
                    try {
                        onFinallyResult = onFinally();
                    } catch (error) {
                        // Use cast method temporarily, wait issue#22428 resolve front bug
                        onFinallyError = error as Error;
                    }
                }

                if (onFinallyError) {
                    promise.rejectImpl(onFinallyError!, false);
                } else if (onFinallyResult instanceof Promise) {
                    onFinallyResult.then(
                        () => {
                            resolveOriginalPromise();
                        },
                        (error) => {
                            promise.rejectImpl(error as Error, false);
                        }
                    );
                } else {
                    resolveOriginalPromise();
                }
            } catch (unexpectedError) {
                promise.rejectImpl(unexpectedError as Error, false);
            }
        };
        this.submitCallback(fn as Object);
        return promise;
    }

    /**
     * Create a resolved promise
     *
     * @returns { Promise<void> } resolved promise
     */
    static resolve(): Promise<void> {
        let p = new Promise<void>();
        p.resolveImpl(undefined, false);
        return p;
    }

    /**
     * Create a resolved promise
     *
     * @param value value to resolve promise with
     * @returns { Promise } resolved promise
     */
    static resolve<U>(value: U|PromiseLike<U>): Promise<Awaited<U>> {
        if (value instanceof Promise) {
            return value as Promise<Awaited<U>>;
        }
        let p = new Promise<Awaited<U>>();
        p.doResolve(value);
        return p;
    }

    /**
     * Create a rejected promise
     *
     * @returns { Promise<void> } rejected promise
     */
    static reject(): Promise<void> {
        let p = new Promise<void>();
        p.rejectImpl(new Error(), false);
        return p;
    }

    /**
     * Create a rejected promise
     *
     * @param { Error } value value to reject promise with
     * @returns { Promise } rejected promise
     */
    static reject<U = never>(error: Error): Promise<Awaited<U>> {
        let p = new Promise<Awaited<U>>();
        p.rejectImpl(error, false);
        return p;
    }

    /**
     * Create a promise that will be resolved after all promises are resolved
     *
     * @param promises array of promise-like objects
     * @returns { Promise } promise that will be resolved after all promises are resolved with an array of results
     *     or rejected if at least one promise is rejected with the rejection result
     */
    static all<U>(promises: FixedArray<U|PromiseLike<U>|undefined>): Promise<Array<Awaited<U>>> {
        return new Promise<Array<Awaited<U>>>((resolve: (value: Array<Awaited<U>>) => void, reject: (error: Error) => void): void => {
            let resolvedCnt = 0;
            const values: Array<Awaited<U>> = new Array<Awaited<U>>(promises.length);
            
            if (promises.length === 0) {
                resolve(values);
                return;
            }

            for (let i = 0; i < promises.length; ++i) {
                const idx = i;
                Promise.toPromiseLike<U>(promises[idx]).then<void, void>(
                    (value: Awaited<U>): void => {
                        values[idx] = value;
                        ++resolvedCnt;
                        if (resolvedCnt === promises.length) {
                            resolve(values);
                        }
                    }, 
                    (error: Error): void => {
                        reject(error);
                    }
                );
            }
        });
    }

    /**
     * Create a promise that will be resolved after all promises are resolved
     *
     * @param promises iterable object with promise-like objets
     * @returns { Promise } promise that will be resolved after all promises are resolved with an array of results
     *     or rejected if at least one promise is rejected with the rejection result
     */
    static all<U>(promises: Iterable<U|PromiseLike<U>>): Promise<Array<Awaited<U>>> {
        try {
            return Promise.all<U>(Promise.toArray(promises));
        } catch (e) {
            // NOTE(csaba.osztrogonac): remove as cast once e has Error type
            return Promise.reject<Array<Awaited<U>>>(e as Error);
        }
    }

    /**
     * Create a promise that will be resolved after all promises are completed
     *
     * @param promises array of promise-like objects
     * @returns { Promise } promise that will be resolved after all promises are completed with an array of results
     */
    static allSettled<U>(promises: FixedArray<U|PromiseLike<U>|undefined>): Promise<PromiseSettledResult<Awaited<U>>[]> {
        return new Promise<PromiseSettledResult<Awaited<U>>[]>(
            (resolve: (value: PromiseSettledResult<Awaited<U>>[]) => void, _reject: (error: Error) => void): void => {
                let settledCnt = 0;
                const results: PromiseSettledResultBase[] = new PromiseSettledResultBase[promises.length];

                const getResults = (): PromiseSettledResult<Awaited<U>>[] => {
                    return (results as Object) as PromiseSettledResult<Awaited<U>>[];
                };

                if (promises.length === 0) {
                    resolve(getResults());
                    return;
                }

                const settle = (): void => {
                    ++settledCnt;
                    if (settledCnt === promises.length) {
                        resolve(getResults());
                    }
                };

                const fulfill = (value: Awaited<U>, idx: int): void => {
                    results[idx] = new PromiseFulfilledResult<Awaited<U>>(value);
                    settle();
                };

                const rejectOne = (error: Error, idx: int): void => {
                    results[idx] = new PromiseRejectedResult(error);
                    settle();
                };

                for (let i = 0; i < promises.length; ++i) {
                    const idx = i;
                    try {
                        Promise.toPromiseLike<U>(promises[idx]).then<void, void>(
                            (value: Awaited<U>): void => {
                                fulfill(value, idx);
                            },
                            (error: Error): void => {
                                rejectOne(error, idx);
                            }
                        );
                    } catch (e) {
                        rejectOne(e as Error, idx);
                    }
                }
            }
        );
    }

    /**
     * Create a promise that will be resolved after all promises are completed
     *
     * @param promises iterable object of promise-like objects
     * @returns { Promise } promise that will be resolved after all promises are completed with an array of results
     */
    static allSettled<U>(promises: Iterable<U|PromiseLike<U>>): Promise<PromiseSettledResult<Awaited<U>>[]> {
        try {
            return Promise.allSettled<U>(Promise.toArray(promises));
        } catch (e) {
            // NOTE(csaba.osztrogonac): remove as cast once e has Error type
            return Promise.reject<PromiseSettledResult<Awaited<U>>[]>(e as Error);
        }
    }

    /**
     * Create a promise that will be resolved after all promises are completed and at least one is resolved
     *
     * @param promises array of promise-like objects
     * @returns { Promise } promise that will be resolved if at least one promise is resolved with the result of resolved promise
     * or rejected with AggregateError if all promises are rejected
     */
    static any<U>(promises: FixedArray<U|PromiseLike<U>|undefined>): Promise<Awaited<U>> {
        return new Promise<Awaited<U>>((resolve: (value: Awaited<U>) => void, reject: (error: Error) => void): void => {
            let resolvedCnt = 0;
            let rejectedCnt = 0;
            const errors: Error[] = new Error[promises.length];

            if (promises.length === 0) {
                reject(new AggregateError(errors, "All promises are rejected"));
                return;
            }

            const rejectImpl = (error: Error, idx: int): void => {
                ++rejectedCnt;
                if (error == undefined || error instanceof Error) {
                    errors[idx] = error;
                } else {
                    errors[idx] = new Error(error!.toString());
                }
                if (rejectedCnt === errors.length) {
                    reject(new AggregateError(errors, "All promises are rejected"));
                }
            };

            for (let i = 0; i < promises.length; ++i) {
                const idx = i;
                try {
                    Promise.toPromiseLike<U>(promises[idx]).then<void, void>(
                        (value: Awaited<U>): void => {
                            ++resolvedCnt;
                            if (resolvedCnt === 1) {
                                resolve(value);
                            }
                        },
                        (error: Error): void => {
                            rejectImpl(error, idx);
                        }
                    );
                } catch (e) {
                    rejectImpl(e as Error, i);
                }
            }
        });
    }

    /**
     * Create a promise that will be resolved after all promises are completed and at least one is resolved
     *
     * @param promises iterable object of promise-like objects
     * @returns { Promise } promise that will be resolved if at least one promise is resolved with the result of resolved promise
     *     or rejected with AggregateError if all promises are rejected
     */
    static any<U>(promises: Iterable<U|PromiseLike<U>>): Promise<Awaited<U>> {
        try {
            return Promise.any<U>(Promise.toArray(promises));
        } catch (e) {
            // NOTE(csaba.osztrogonac): remove as cast once e has Error type
            return Promise.reject<U>(e as Error);
        }
    }

    /**
     * Create a promise that will be completed after at least one promise is completed
     *
     * @param promises array of promise-like objects
     * @returns { Promise } promise that will completed with the same value as one of the promises in the argument
     */
    static race<U>(promises: FixedArray<U|PromiseLike<U>|undefined>): Promise<Awaited<U>> {
        return new Promise<Awaited<U>>((resolve: (value: Awaited<U>) => void, reject: (error: Error) => void): void => {
            let settled = false;
            for (let i = 0; i < promises.length; ++i) {
                const idx = i;
                Promise.toPromiseLike<U>(promises[idx]).then<void, void>(
                    (value: Awaited<U>): void => {
                        if (!settled) {
                            resolve(value);
                            settled = true;
                        }
                    }, 
                    (error: Error): void => {
                        if (!settled) {
                            reject(error);
                            settled = true;
                        }
                    }
                );
            }
        });
    }

    /**
     * Create a promise that will be completed after at least one promise is completed
     *
     * @param promises iterable object of promise-like objects
     * @returns { Promise } promise that will completed with the same value as one of the promises in the argument
     */
    static race<U>(promises: Iterable<U|PromiseLike<U>>): Promise<Awaited<U>> {
        try {
            return Promise.race<U>(Promise.toArray(promises));
        } catch (e) {
            // NOTE(csaba.osztrogonac): remove as cast once e has Error type
            return Promise.reject<U>(e as Error);
        }
    }

    private static toArray<U>(values: Iterable<U>): FixedArray<U | undefined> {
        let it = values.$_iterator();
        let v = it.next();
        if (v.done) {
            return new FixedArray<U | undefined>(0);
        }
        let array : FixedArray<U | undefined> = new FixedArray<U | undefined>(1);
        array[0] = v.value;
        let len = 1;
        while (true) {
            v = it.next();
            if (v.done) {
                return Promise.trim(array, len);
            }
            array = Promise.ensureCapacity(array, len + 1);
            array[len] = v.value;
            ++len;
        }
    }

    private static trim<U>(array: FixedArray<U>, length: int): FixedArray<U | undefined> {
        if (array.length == length) {
            return array;
        }
        let result : FixedArray<U | undefined> = new FixedArray<U | undefined>(length);
        for (let i = 0; i < length; ++i) {
            result[i] = array[i];
        }
        return result;
    }

    private static ensureCapacity<U>(queue: FixedArray<U>, size: int): FixedArray<U | undefined> {
        if (size <= queue.length) {
            return queue;
        }
        let newQueue : FixedArray<U | undefined> = new FixedArray<U | undefined>(size * 2);
        for (let i = 0; i < queue.length; ++i) {
            newQueue[i] = queue[i];
        }
        return newQueue;
    }

    private subscribeOnAnotherPromise<U>(internalPromise: PromiseLike<U>): void {
        this.mutex.lock();
        if(this.state != Promise.STATE_PENDING) {
            this.mutex.unlock();
            return;
        }
        this.state = Promise.STATE_LINKED;
        this.mutex.unlock();
        let thisPromise = this;
        let fn: () => void = ():void => {
            try{
                internalPromise.then<void, void>((value: U): void => {
                    thisPromise.resolveImpl(value, true);
                }, (error: Error): void => {
                    thisPromise.rejectImpl(error, true);
                });
            } catch(error) {
                // NOTE(csaba.osztrogonac): remove as cast once e has Error type
                thisPromise.rejectImpl(error as Error, true);
            }
        }
        if(internalPromise instanceof Promise) {
            fn();
        } else {
            // To guarantee that the then method of a thenable executes asynchronously.
            Promise.resolve().then(fn);
        }
    }

    private doResolve<U>(value: U|PromiseLike<U>): void {
        if (value instanceof PromiseLike) {
            if (value === this) {
                this.rejectImpl(new TypeError("Cannot reolve the promise by self"), false);
            }
            this.subscribeOnAnotherPromise<U>(value as PromiseLike<U>);
        } else {
            this.resolveImpl(value, false);
        }
    }

    private static toPromiseLike<U>(value: U|PromiseLike<U>|undefined): PromiseLike<Awaited<U>> {
        if (value instanceof Promise) {
            let p = value as Promise<Awaited<U>>;
            return p;
        } else {
            let p = Promise.resolve<U>(value as U);
            return p;
        }
    }

    /**
     * Suspend coroutine until the promise is completed
     *
     * @returns promise completion value
     */
    public native awaitResolution(): T;

    private native resolveImpl<U>(value: U, wasLinked: boolean): void;
    private native rejectImpl(error: Object, wasLinked: boolean): void;
    private native submitCallback(callback: Object): void;

    private static STATE_PENDING = 0;
    private static STATE_LINKED = 1;
    private static STATE_RESOLVED = 2;
    private static STATE_REJECTED = 3;
    private static EMPTY_QUEUE = new Object[0];

    // Order of fields should be the following
    // 1. Reference fields
    // 2. Primitive fields in mostly size decreasing order
    // filling alignment holes
    private value: Any = undefined;
    private mutex = new Mutex();
    private event = new Event();
    private callbackQueue?: Object[];
    private workerDomainQueue?: int[];
    private interopObject?: Object;
    private linkedPromise?: Object;
    private queueSize: int = 0;
    private state: int = Promise.STATE_PENDING;
}

export type NullablePromise<out T> = Promise<T> | undefined;
