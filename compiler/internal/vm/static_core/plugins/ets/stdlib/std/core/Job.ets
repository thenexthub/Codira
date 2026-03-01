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

enum JobStatus { RUNNING = 0, FINISHED = 1, FAILED = 2 }

/**
 * Object of Job class is used to represent a task
 * that was launched using std.concurrency.launch function.
 */
export class Job<T> {

    protected constructor() {}


    /**
     * This method causes current coroutine to wait for completion of
     * the job, and returns awaited object in case of successful completion.
     *
     * @returns unwrapped value.
     */
    public native Await(): T

    private val: T
    private mutex: Mutex = new Mutex()
    private event: Event = new Event()
    private status : int = JobStatus.RUNNING
}

/**
 * Object of CompletableJob class is inherited from Job class.
 * It allows to finish or fail the task by developer explicitly.
 */
export final class CompletableJob<T> extends Job<T>
{

    public constructor() {}

    /**
     * This method causes current coroutine to wait for completion of
     * the job, and returns awaited object in case of successful completion.
     *
     * @returns unwrapped value.
     */
    public Await() : T {
        return super.Await();
    }

    /**
     * This method is used to finish the job with undefined value.
     */
    public finish() {
        this.finishImpl(undefined);
    }

    /**
     * This method is used to finish the job with a value.
     * It can be called from any coroutine.
     *
     * @param value value to finish the job with.
     */
    public finish<T>(value: T) {
        this.finishImpl<T>(value);
    }

    /**
     * This method is used to fail the job with empty Error.
     */
    public fail() {
        this.failImpl(new Error());
    }

    /**
     * This method is used to fail the job with some specific error.
     * It can be called from any coroutine.
     *
     * @param error value to fail the job with.
     */
    public fail(error: Error) {
        this.failImpl(error);
    }

    private native finishImpl<U>(value: U): void
    private native failImpl(error: Any): void
}
