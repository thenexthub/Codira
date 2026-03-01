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

package std.core;

type UncaughtErrorListener = StdProcess.UncaughtErrorListener;

/**
 * @class Stores EventListener that invokes when unhandled exception occurs
 */
export class ProcessErrorHandler {
    private constructor() {}

    /**
     * Constructs handler by passing EventListener
     *
     * @param cb inner callback
     *
     */
    public constructor(cb: UncaughtErrorListener) {
        this.callback = cb;
    }

    /**
     * Calls a custom handler on uncaught error and then exits
     *
     * @param error Uncaught error
     */
    public handleUncaughtError(error: Object): void {
        if (error instanceof Error) {
            error = error as Error;
        } else {
            console.error('Error type is not recognized:', error);
            let procManager = new StdProcess.ProcessManager();
            procManager.exit(1);
        }
        ConcurrencyHelpers.mutexLock(this.mutex);
        try {
            this.callback(error);
        } catch (e) {
            console.error('Error while invoking callback:', e);
            let procManager = new StdProcess.ProcessManager();
            procManager.exit(1);
        }
        ConcurrencyHelpers.mutexUnlock(this.mutex);
    }

    /**
     * Resets inner EventListener
     *
     * @param cb new callback
     *
     */
    public reset(cb: UncaughtErrorListener) {
        ConcurrencyHelpers.lockGuard(this.mutex, () => {
            this.callback = cb;
        });
    }

    private mutex: Object = ConcurrencyHelpers.mutexCreate();
    private callback: UncaughtErrorListener = (error: Object) => {};
};
