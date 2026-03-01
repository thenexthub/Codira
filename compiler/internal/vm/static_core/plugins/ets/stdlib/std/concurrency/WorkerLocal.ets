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

/**
 * This class allows to Wrap a value
 * It is usefull, so we can distinct undefined and Wrapper<undefined>
 * That way ConcurrentHashMap<Wrapper<T>>.get() where T = SMTH | undefined
 * returns undefined, if there are no value and Wrapper<T> with undefined if
 * one has stored an undefined value
 */
class Wrapper <T> {
    constructor() {}

    constructor(value: T) {
        this.value = value;
    }

    get(): T {
        return this.value;
    }

    private value: T;
}

export class WorkerLocal<T> {

    /**
    * Creates a WorkerLocal instance without initialization function.
    * Requires explicit value initialization via set() before calling get().
    * @constructor
    */
    constructor() {}
    
    /**
     * Creates a WorkerLocal instance with value initialization function.
     * Automatically initializes value on first get() call per worker.
     * @constructor
     * @param {() => T} init - Factory function for initial worker-local value
     */
    constructor(init: () => T) {
      this.init = init;
    }
  
    /**
     * Retrieves the worker-local value for current worker. Auto-initializes
     * using provided init function if value not exists.
     * 
     * @returns {T} Worker-specific stored value
     * @throws {Error} When value not initialized and no init function provided
     */
    get(): T {
        let res : Any = undefined;
        ConcurrencyHelpers.spinlockGuard(this.tableSpinlock, (): void => {
            res = this.getinner();
        });
        return res as T;
    }
 
    private getinner(): T {
        const workerId = CoroutineExtras.getWorkerId();

        let wrapped: Wrapper<T> | undefined = this.storage.get(workerId);
        if (wrapped === undefined) {
            if (this.init) {
                let value = this.init!();
                this.storage.set(workerId, new Wrapper<T>(value));
                return value;
            } else {
                throw new Error("WorkerLocal value not initialized. Call set() first or provide an init function.");
            }
        } else {
            return wrapped!.get();
        }
    }

    /**
     * Updates the worker-local value for current worker.
     * 
     * @param {T} value - New value to store for current worker
     */ 
    set(value: T):void {
        ConcurrencyHelpers.spinlockGuard(this.tableSpinlock, () => {
            const workerId = CoroutineExtras.getWorkerId();
            this.storage.set(workerId, new Wrapper<T>(value));
        });
    }

    /**
     * Delete the worker-local value for current worker.
     */ 
    delete(): void {
        ConcurrencyHelpers.spinlockGuard(this.tableSpinlock, () => {
            const workerId = CoroutineExtras.getWorkerId();
            this.storage.delete(workerId);
        });
    }
    // Asynchronous Lock may cause data refreesh failures in high-concurrency scenarios
    // Using spinlock replace it
    // NOTE(liu_yaning, #26285): current solution is temporary, the performance will be improved in future
    private tableSpinlock: Object = ConcurrencyHelpers.spinlockCreate();
    private storage = new Map<WorkerId, Wrapper<T>>();
    private init?: () => T;
}
