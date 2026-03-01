/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

final class FinRegNode<T> extends BaseWeakRef<Any> {
    constructor(obj: Any, arg: T, token: WeakRef<Object> | undefined, finReg: FinalizationRegistry<T>) {
        super(obj);
        this.token = token;
        this.arg = arg;
        this.finReg = finReg;
    }
    public token: WeakRef<Object> | undefined;
    public arg: T;
    public finReg: FinalizationRegistry<T>;
    public prev: FinRegNode<T> | undefined = undefined;
    public next: FinRegNode<T> | undefined = undefined;
    public bucketIdx: int = -1;
}

/**
  * FinalizationRegistry provides a way to execute the provided callback when a
  * registered object gets garbage collected.
  * FinalizationRegistry is coroutine-safe class.
  * - It is safe to pass an instance to another coroutines.
  * - It is safe to call methods from several coroutines.
  *
  * Notes:
  * - Runtime doesn't provide any guarantees that the registered object will be
  * garbage collected (even if its dead).
  * - Even the object is garbage collected runtime doesn't guarantee that
  * the callback will be called.
  * - Runtime ignores any exceptions thrown in the callback.
  * - User should avoid call await in the callback.
  * - Runtime calls the callback after the object is garbage collected at some point
  * during execution.
  * - The callback may be called in any coroutine.
  */
export final class FinalizationRegistry<T> {
    /**
     * Create FinalizationRegistry object with the specific callback function.
     * One can register an object and after reclamation of the object
     * the callback function will be called.
     *
     * @param callback_value callback function which will be called
     * after registered object reclamation.
     */
    public constructor(callback: (value: T) => void) {
        this.callback = callback;
        this.workerId = FinalizationRegistry.getWorkerId();
        this.workerDomain = FinalizationRegistry.getWorkerDomain();
        this.buckets = new FixedArray<FinRegNode<T> | undefined>(FinalizationRegistry.INITIAL_BUCKETS_COUNT)
    }

    /**
     * @returns size of the FinalizationRegistry
     */
    public getSize(): int {
        let count: int = 0;
        this.mutex.lockGuard(() => {
            count = this.mapSize + this.listSize;
        });
        return count;
    }

    /**
     * Registers an object with specific callback's argument and
     * optional token - an object used to remove the entry from the registry.
     * @see unregister
     * After the object is garbage collected, runtime will call the callback
     * with the specific argument.
     *
     * @param object a tracking object
     *
     * @param callbackArg callback argument for callback method
     *
     * @param token an object used to remove the entry from the registry
     */
    public register(object: Object, callbackArg: T, token?: Object): void {
        return this.registerImpl(object, callbackArg, token);
    }

    private registerImpl(ref: Any, callbackArg: T, token?: Object): void {
        if (token != undefined) {
            // Add unregistrable node into map
            const refToken = new WeakRef<Object>(token!);
            const objectHash = Runtime.getHashCode(token!);
            let newNode = new FinRegNode<T>(ref, callbackArg, refToken, this);

            this.mutex.lock();
            try {
                const bucketIdx = this.getBucketIndex(objectHash);
                newNode.bucketIdx = bucketIdx;
                if (this.buckets[bucketIdx] != undefined) {
                    let head = this.buckets[bucketIdx]!;
                    head.prev = newNode;
                    newNode.next = head;
                }
                this.buckets[bucketIdx] = newNode;
                this.mapSize++;

                // Rehash if elements count > 0.75 of map size
                if (this.mapSize * 4 > this.buckets.length * 3) {
                    this.rehash(this.buckets.length * 4)
                }
            } finally {
                this.mutex.unlock();
            }
        } else {
            // Add non-unregistrable node into list
            let newNode = new FinRegNode<T>(ref, callbackArg, undefined, this);
            this.mutex.lock();
            try {
                newNode.next = this.nonUnregistrableList;
                if (this.nonUnregistrableList !== undefined) {
                    this.nonUnregistrableList!.prev = newNode;
                }
                this.nonUnregistrableList = newNode;
                this.listSize++;
            } finally {
                this.mutex.unlock();
            }
        }
    }

    /**
     * Removes all entries with the specific token
     *
     * @param token object specified which entries should be removed
     */
    public unregister(token: Object): void {
        this.mutex.lock();
        try {
            const bucketIdx = this.getBucketIndex(Runtime.getHashCode(token));
            if (this.buckets[bucketIdx] != undefined) {
                let currNode = this.buckets[bucketIdx];
                while (currNode != undefined) {
                    const nextNode = currNode.next;
                    if (token == currNode.token!.deref()) {
                        this.deleteSingleNodeFromMap(currNode, bucketIdx);
                    }
                    currNode = nextNode;
                }
            }
        } finally {
            this.mutex.unlock();
        }
    }

    private rehash(newSizeArg: int) {
        const oldMap = this.buckets;
        const newSize = newSizeArg;
        this.buckets = new FixedArray<(FinRegNode<T> | undefined)>(newSize);
        this.mapSize = 0;
        for (let oldMapIdx = 0; oldMapIdx < oldMap.length; oldMapIdx++) {
            let nextNode: (FinRegNode<T> | undefined) = undefined;
            for (let currNode = oldMap[oldMapIdx]; currNode != undefined; currNode = nextNode) {
                nextNode = currNode!.next;
                if (currNode!.token!.deref() != undefined) {
                    const bucketIdx = this.getBucketIndex(Runtime.getHashCode(currNode!.token!.deref()!));
                    currNode!.bucketIdx = bucketIdx;
                    if (this.buckets[bucketIdx] == undefined) {
                        this.buckets[bucketIdx] = currNode;
                        currNode!.prev = undefined;
                        currNode!.next = undefined;
                    } else {
                        let head = this.buckets[bucketIdx]!;
                        head.prev = currNode;
                        currNode!.prev = undefined;
                        currNode!.next = head;
                        this.buckets[bucketIdx] = currNode;
                    }
                    this.mapSize++;
                } else {
                    // Token was deleted, so put node into non-unregistrable list
                    currNode!.prev = undefined;
                    currNode!.next = this.nonUnregistrableList;
                    this.nonUnregistrableList = currNode;
                    this.listSize++;
                }
            }
        }
    }

    private getBucketIndex(x: int): int {
        const t = x >> 31;
        return ((x ^ t) - t) & (this.buckets.length - 1);
    }

    private deleteSingleNodeFromMap(nodeToDelete: FinRegNode<T>, bucketIdx: int) {
        let prev = nodeToDelete.prev;
        let next = nodeToDelete.next;

        if (prev == undefined && next == undefined) {
            this.buckets[bucketIdx] = undefined;
        } else if (prev == undefined) {
            this.buckets[bucketIdx] = next;
            next!.prev = undefined;
        } else if (next == undefined) {
            prev!.next = undefined;
        } else {
            prev!.next = next;
            next!.prev = prev;
        }

        this.mapSize--;
    }

    private static execCleanup<T>(head: FinRegNode<T> | undefined): void {
        while (head != undefined) {
            let finReg = head.finReg;
            let arg = head.arg;
            try {
                finReg.callback(arg);
            } catch (e) {
                console.log(`Error: ` + e.toString());
                if (e instanceof Error) {
                    if (e.stack) {
                        console.log(e.stack);
                    }
                }
            }
            head = head.next;
        }
        FinalizationRegistry.finishCleanup();
    }

    private static native getWorkerId(): int;
    private static native getWorkerDomain(): int;
    private static native finishCleanup(): void;

    private callback: (value: T) => void;
    private mutex = new Mutex();
    private workerId: int;
    private workerDomain: int;

    // Map for unregistrable objects
    private buckets: FixedArray<FinRegNode<T> | undefined> = [];
    private mapSize: int = 0;

    // List for non-unregistrable objects
    private nonUnregistrableList: FinRegNode<T> | undefined = undefined;
    private listSize: int = 0;

    private nextFinReg: FinalizationRegistry<T> | undefined = undefined;
    private finalizationQueueHead: FinRegNode<T> | undefined = undefined;
    private finalizationQueueTail: FinRegNode<T> | undefined = undefined;

    private static readonly INITIAL_BUCKETS_COUNT: int = 16;
}
