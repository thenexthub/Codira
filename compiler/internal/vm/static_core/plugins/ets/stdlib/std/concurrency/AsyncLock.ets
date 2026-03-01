/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

package std.concurrency;

type AsyncLockCallback<T> = () => T | Promise<T>;

export enum AsyncLockMode {
    UNLOCKED = 0,
    SHARED = 1,
    EXCLUSIVE = 2,
}

export class AsyncLock {

    /**
     * Creates instance of AsyncLock class
     */
    constructor() {
        this(AsyncLock.generateAnonymousLockName());
    }

    protected constructor(name: string) {
        this.name = name;
        asyncLockManager.setElem(this);
    }

    /**
    * Find or create an instance of AsyncLock using the specified name.
    *
    * @param { string } name - name of the lock to find or create.
    * @returns { AsyncLock } Returns an instance of AsyncLock.
    */
    public static request(name: string): AsyncLock {
        return asyncLockManager.request(name);
    }

    /**
     * Perform an operation with the acquired lock exclusively.
     * The method acquires the lock first, then calls the callback, and then releases the lock.
     * The callback is called asynchronously in the same thread where lockAsync was called.
     *
     * @param { AsyncLockCallback<T> } callback - function to call when the lock gets acquired.
     * @returns { Promise<T> } Promise that will be resolved after the callback gets executed.
     */
    public lockAsync<T>(callback: AsyncLockCallback<T>): Promise<T> {
        return this.lockAsync(callback, AsyncLockMode.EXCLUSIVE, new AsyncLockOptions());
    }

    /**
     * Perform an operation with the acquired lock.
     * The method acquires the lock first, then calls the callback, and then releases the lock.
     * The callback is called asynchronously in the same thread where lockAsync was called.
     *
     * @param { AsyncLockCallback<T> } callback - function to call when the lock gets acquired.
     * @param { AsyncLockMode } mode - mode of the lock operation.
     * @returns { Promise<T> } Promise that will be resolved after the callback gets executed or rejected.
     */
    public lockAsync<T>(callback: AsyncLockCallback<T>, mode: AsyncLockMode): Promise<T> {
        return this.lockAsync(callback, mode, new AsyncLockOptions());
    }

    /**
     * Perform an operation with the acquired lock.
     * The method acquires the lock first, then calls the callback, and then releases the lock.
     * The callback is called asynchronously in the same thread where lockAsync was called.
     * An optional timeout value can be provided in {@link AsyncLockOptions}. In this case, lockAsync will reject the
     * resulting promise with a BusinessError instance if the lock is not acquired before timeout exceeds.
     * The error message, in this case, will contain the held and waited locks information and possible deadlock
     * warnings.
     *
     * @param { AsyncLockCallback<T> } callback - function to call when the lock gets acquired.
     * @param { AsyncLockMode } mode - mode of the lock operation.
     * @param { AsyncLockOptions<U> } options - lock operation options.
     * @returns { Promise<T | U> } Promise that will be resolved after the callback gets executed or rejected in case
     *     timeout exceeded.
     */
    public lockAsync<T, U>(callback: AsyncLockCallback<T>, mode: AsyncLockMode, options: AsyncLockOptions<U>): Promise<T> {
        let callerCID = CoroutineExtras.getCoroutineId();
        let asyncLockInfo = new AsyncLockInfo(this.name, mode, callerCID);
        let wrapped = (l: AsyncLock) => { return l.wrapper<T, U>(callback, callerCID, mode, options, asyncLockInfo); }
        if (this.canAcquireLock(mode, callerCID, asyncLockInfo)) {
            wrapped = (l: AsyncLock) => { return l.syncwrapper<T, U>(callback, options,  asyncLockInfo); }
        } else {
            ConcurrencyHelpers.mutexLock(this.mutex);
            this.pending.add(asyncLockInfo);
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
        return AsyncLock.launchPromise(wrapped, this);
    }

    private canAcquireLock<T>(mode: AsyncLockMode, callerCID: int, asyncLockInfo: AsyncLockInfo) : boolean {
        ConcurrencyHelpers.mutexLock(this.mutex);
        try {
            if (this.holderCID == 0 && this.mode == AsyncLockMode.UNLOCKED) {
                this.counter = 1;
                this.held.add(asyncLockInfo);
                this.holderCID = callerCID;
                this.mode = mode;
                return true;
            }
            if (mode == AsyncLockMode.SHARED && this.mode == AsyncLockMode.SHARED) {
                this.counter++;
                this.held.add(asyncLockInfo);
                this.mode = mode;
                return true;
            }
            return false;
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
    }

    private syncwrapper<T,U>(callback: AsyncLockCallback<T>, options : AsyncLockOptions<U>, asyncLockInfo: AsyncLockInfo) : Promise<Awaited<T>> {
        return Promise.resolve<T>(callback()).finally(() => {
             if (options.signal != null && options.signal!.aborted == true) {
                this.fireCondEvent();
                throw new Error('Aborted');
            }
            this.unlock(asyncLockInfo);
        });

    }

    private static launchPromise<T>(f: Function, ...args: FixedArray<Any>): Promise<T> {
        let resolver: (((v: T | PromiseLike<T>) => void) | undefined) = undefined;
        let rejecter: (((error: Error) => void) | undefined) = undefined;
        let p = new Promise<T>((res, rej) => {
            resolver = res
            rejecter = rej;
        })
        let cb = () => {
            try {
                let v = f.unsafeCall(...args) as T
                resolver!(v)
            } catch (e) {
                // NOTE(csaba.osztrogonac): remove as cast once e has Error type
                rejecter!(e as Error)
            }
        }
        launchSameWorker(cb)
        return p;
    }

    private wrapper<T, U>(callback: AsyncLockCallback<T>, callerCID: int, mode: AsyncLockMode, options: AsyncLockOptions<U>, asyncLockInfo : AsyncLockInfo) : Promise<Awaited<T>> {

        this.lock<U>(callerCID, mode,  asyncLockInfo, options);
        return Promise.resolve<T>(callback()).finally(() => {if (options.signal != null && options.signal!.aborted == true ) {
            this.fireCondEvent();
            throw new Error('Aborted');
        }
            this.unlock(asyncLockInfo);
        });
    }

    private lock<U>(callerCID: int, mode: AsyncLockMode, asyncLockInfo: AsyncLockInfo, options: AsyncLockOptions<U>): void {
        let isDone = false;
        let timeout = options.timeout;
        let isTimeoutExec: boolean = false;

        if (timeout) {
           setTimeout(() => {
              this.queryAllDeadlocks();
              this.fireCondEvent();
              isTimeoutExec = true;
           }, timeout);
        }

        while (isDone == false) {

             if (options.signal != null && options.signal!.aborted == true) {
                 this.fireCondEvent();
                 throw new Error('Aborted');
            }
            if(isTimeoutExec == true) {
                throw new Error (this.deadLockPrinter.toString('lockAsync timed out'));
            }

            if(options.isAvailable) {
                throw new Error("AsyncLock is not available");
            }

            ConcurrencyHelpers.mutexLock(this.mutex);
            if (this.holderCID == 0) {
                this.holderCID = callerCID;
                this.mode = mode;
                this.counter = 1;
                this.pending.delete(asyncLockInfo);
                this.held.add(asyncLockInfo);
                isDone = true;
            } else if (mode == AsyncLockMode.SHARED && this.mode == AsyncLockMode.SHARED) {
                this.counter++;
                this.pending.delete(asyncLockInfo);
                this.held.add(asyncLockInfo);
                isDone = true;
            }

            if (isDone == false) {
                ConcurrencyHelpers.condVarWait(this.condVar, this.mutex);
            }
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
    }

    private fireCondEvent()
    {
        ConcurrencyHelpers.mutexLock(this.mutex);
        ConcurrencyHelpers.condVarNotifyAll(this.condVar, this.mutex);
        ConcurrencyHelpers.mutexUnlock(this.mutex);
    }

    private unlock(asyncLockInfo: AsyncLockInfo): void {
        ConcurrencyHelpers.mutexLock(this.mutex);
        this.held.delete(asyncLockInfo);
        this.counter--;
        if (this.counter == 0) {
            this.holderCID = 0;
            this.mode = AsyncLockMode.UNLOCKED;
            ConcurrencyHelpers.condVarNotifyAll(this.condVar, this.mutex);
        }
        ConcurrencyHelpers.mutexUnlock(this.mutex);
    }

    /**
     * Query information about the specified lock.
     *
     * @param { string } name - name of the lock.
     * @returns { AsyncLockState } Returns an instance of AsyncLockState.
     */
    public static query(name: string): AsyncLockState {
        try {
            return asyncLockManager.query(name);
        } catch (e) {
            throw e;
        }
    }

    /**
     * Query information about all locks.
     *
     * @returns { AsyncLockState[] } Returns an array of AsyncLockState.
     */
    public static queryAll(): AsyncLockState[] {
        return asyncLockManager.queryAll();
    }

    /**
     * Query information about the lock.
     *
     * @returns { AsyncLockState } Returns an instance of AsyncLockState.
     */
    public query(): AsyncLockState {
        ConcurrencyHelpers.mutexLock(this.mutex);
        let state = new AsyncLockState(this.held, this.pending);
        ConcurrencyHelpers.mutexUnlock(this.mutex);
        return state;
    }

    private queryDeadlock(deadLockPrinter : DeadLockInfo) {
        ConcurrencyHelpers.mutexLock(this.mutex);
        this.held.forEach((v : AsyncLockInfo) => {
            this.pending.forEach((p : AsyncLockInfo) => {
                if (v.contextId != p.contextId) {
                    deadLockPrinter.add(v, p);
                }
            })
        });
        ConcurrencyHelpers.mutexUnlock(this.mutex);
    }

    private  queryAllDeadlocks() {
        let arr = asyncLockManager.queryAllLocks() ;
        for (let i = 0; i < arr.length; ++i) {
            arr[i].queryDeadlock(this.deadLockPrinter);
        }
    }

    private static generateAnonymousLockName() {
        return "__AnonymousLockName__" + Number.toString(AsyncLock.LastAnonymousId++);
    }

    public readonly name: string;
    private static LastAnonymousId: int = 0;
    private holderCID: int = 0;
    private mode: AsyncLockMode = AsyncLockMode.UNLOCKED;
    private counter: int = 0;
    private mutex: Object = ConcurrencyHelpers.mutexCreate();
    private held: Set<AsyncLockInfo> = new Set<AsyncLockInfo>();
    private pending: Set<AsyncLockInfo> = new Set<AsyncLockInfo>();
    private condVar: Object = ConcurrencyHelpers.condVarCreate();
    private deadLockPrinter: DeadLockInfo = new DeadLockInfo();
}


type StoreType  = [string, int, int];

class DeadLockInfo {
    public  constructor() {}

    public toString(head : string ) : string {
        return head + this.toStringHelper();
    }

    public add(h: AsyncLockInfo, p : AsyncLockInfo) {
        this.arr.push([h.name, h.contextId, p.contextId]);
    }

    private toStringHelper()
    {
        let sb =  new StringBuilder();

        for (let i  = 0; i < this.arr.length; ++i) {
            sb.append(this.helper(this.arr[i]).toString());
        }
        return sb;
    }
    private helper(t : StoreType)
    {
        let sb  = new StringBuilder();

        sb.append('\n');
        sb.append('!!! DEADLOCK WARNING !!!');
        sb.append(t[1]);
        sb.append(' <--  {');
        sb.append(t[0]);
        sb.append('} -- WAITED BY TID: ');
        sb.append(t[2]);
        return sb;
    }

    private arr: Array<StoreType>  = new Array<StoreType>;
}

final class AsyncLockHelper extends AsyncLock {
    constructor(name: String) {
        super(name);
    }
}
