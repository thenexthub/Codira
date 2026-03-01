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

import { launch } from "std/concurrency"

/**
 * @class provides api for sync primitives (see SyncPrimitives.ets)
 *
 * @internal
 */
final class ConcurrencyHelpers {
    // Mutex api
    public static native mutexCreate(): Object;
    public static native mutexLock(mutex: Object): void;
    public static native mutexUnlock(mutex: Object): void;
    // Event api
    public static native eventCreate(): Object;
    public static native eventWait(event: Object): void;
    public static native eventFire(event: Object): void;
    // CondVar api
    public static native condVarCreate(): Object;
    public static native condVarWait(condVar: Object, mutex: Object): void;
    public static native condVarNotifyOne(condVar: Object, mutex: Object): void;
    public static native condVarNotifyAll(condVar: Object, mutex: Object): void;
    // Spinlock api
    public static native spinlockCreate(): Object;
    public static spinlockGuard(spinlock: Object, callback: () => void) {
        ConcurrencyHelpers.spinlockGuard(spinlock, callback as Object);
    }

    public static lockGuard(mutex: Object, callback: () => void): void {
        ConcurrencyHelpers.mutexLock(mutex);
        try {
            callback();
        } finally {
            ConcurrencyHelpers.mutexUnlock(mutex);
        }
    }

    private static native spinlockGuard(spinlock: Object, callback: Object): void;
}

class NativeAsyncWorkHelper {
    static queue(executeCb: long, completeCb: long, data: long): void {
        NativeAsyncWorkHelper.queueInternal(executeCb, completeCb, data);
    }
    static async queueInternal(executeCb: long, completeCb: long, data: long) {
        let p = launch<void, (nativeCb: long, data: long, needNativeScope: boolean) => void>(NativeAsyncWorkHelper.asyncWorkNativeInvokeInternal, executeCb, data, true);
        p.Await();
        NativeAsyncWorkHelper.asyncWorkNativeInvokeInternal(completeCb, data, true);
    }
    static native asyncWorkNativeInvoke(nativeCb: long, data: long, needNativeScope: boolean): void;
    static asyncWorkNativeInvokeInternal(nativeCb: long, data: long, needNativeScope: boolean) {
        NativeAsyncWorkHelper.asyncWorkNativeInvoke(nativeCb, data, needNativeScope);
    }

    static sendEvent(workerId: int, executeCb: long, data: long, timeout: long): void {
        NativeAsyncWorkHelper.sendEventInternal(workerId, executeCb, data, timeout);
    }
    static async sendEventInternal(workerId: int, executeCb: long, data: long, timeout: long) {
        let launchParams: LaunchParams = {
            abortFlag: false,
            workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.EXACT_ID, [workerId])
        };
        if (timeout > 0) {
            setTimeout(() => {
                launch<void, (nativeCb: long, data: long, needNativeScope: boolean) => void>(NativeAsyncWorkHelper.eventNativeInvokeInternal, launchParams, executeCb, data, true);
            }, timeout.toInt());
        } else {
            launch<void, (nativeCb: long, data: long, needNativeScope: boolean) => void>(NativeAsyncWorkHelper.eventNativeInvokeInternal, launchParams, executeCb, data, true);
        }
    }
    static eventNativeInvokeInternal(nativeCb: long, data: long, needNativeScope: boolean) {
        NativeAsyncWorkHelper.eventNativeInvoke(nativeCb, data, needNativeScope);
    }
    static native eventNativeInvoke(nativeCb: long, data: long, needNativeScope: boolean): void;
}

final class AtomicInt {
    constructor(v: int) {
        this.value = v;
    }

    public native set(v: int): void;
    public native get(): int;
    
    // Atomically sets the value to the given value and returns the previous value.
    public native exchange(v: int): int;
    // Returns the previous value of the atomic integer.
    public native compareAndSwap(expected: int, newValue: int): int;

    public native fetchAndAdd(v: int): int;
    public native fetchAndSub(v: int): int;

    private value: int;
}
