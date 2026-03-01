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

type CoroFunT<A, R> = (
  a0: A,
  a1: A,
  a2: A,
  a3: A,
  a4: A,
  a5: A,
  a6: A,
  a7: A,
  a8: A,
  a9: A,
  a10: A,
  a11: A,
  a12: A,
  a13: A,
  a14: A,
  a15: A
) => R;


/**
 * @class LaunchParams Class for coroutine launch params.
 * @internal
 */
export class LaunchParams {
  /** @property { boolean } abortFlag if true, aborts the program if unhandled exception were thrown inside the coroutine. */
  abortFlag: boolean = false;
  /** @property { WorkerGroupId } workerGroupId unique ID of the coroutine worker group that will host the newly created coroutine. */
  workerGroupId: WorkerGroupId = WorkerGroup.ANY_ID;
}

/**
 * Launch new coroutine with LaunchParams
 *
 * @param coroFun entrypoint of the the new coroutine.
 * @param launchParams parameters of the new coroutine.
 * @param args arguments for the function coroFun.
 *
 * @returns instance of the Job class that represents newly launched coroutine.
 *
 * @internal
 */
export function launch<R, F extends CoroFunT<never, R>>(coroFun: F, launchParams: LaunchParams, ...args: FixedArray<Any>): Job<R> {
  return launchInternal<R, F>(coroFun, args as FixedArray<Any>, launchParams);
}

/**
 * This function is used to launch new coroutines.
 *
 * @param coroFun entrypoint of the the new coroutine.
 * @param args arguments for the function coroFun.
 *
 * @returns instance of the Job class that represents newly launched coroutine.
 *
 * @internal
 */
export function launch<R, F extends CoroFunT<never, R>>(coroFun: F, ...args: FixedArray<Any>): Job<R> {
  return launchInternal<R, F>(coroFun, args as FixedArray<Any>);
}

/**
 * @enum WorkerDomain The existing worker domains. A worker domain defines a group of workers with some common
 * properties.
 *
 * @internal
 */
export enum WorkerDomain {
    // NOTE(konstanting): we need a mirror class for this to verify that the IDs match the runtime IDs
    GENERAL = 0,
    EXACT_ID,
    MAIN,
    EA,
}

/**
 * @internal
 */
export type WorkerGroupId = FixedArray<long>;
/**
 * @internal
 */
export type CoroutineId = int;
/**
 * @internal
 */
export type WorkerId = int;

/**
 * @class WorkerGroup a group of workers which a coroutine can be launched to.
 *
 * @internal
 */
export class WorkerGroup {
    // NOTE(konstanting): we need a mirror class for this to verify that the IDs match the runtime IDs
    public static readonly INVALID_ID: WorkerGroupId = [0, 0];
    public static readonly ANY_ID: WorkerGroupId = [~0, ~0];

    /**
     * This function generates the worker group ID that can be further used to launch coroutines within that group.
     *
     * @param domain the worker domain.
     * GENERAL domain includes all non-main and non-EA workers.
     * EXACT_ID domain includes exact worker id from hint.
     * MAIN domain includes only main worker.
     * EXACT_ID domain includes only workers from "hint" parameter.
     * @param hint optional worker ids, the usage is domain-specific.
     * For GENERAL domain non-main and non-EA worker IDs are ignored, all other IDs are used as is.
     * For MAIN domain hint is ignored.
     * For EXACT_ID domain hint is used as is, empty hint means ANY_ID.
     * For EA domain EA worker IDs are used as is, non-EA worker IDs are ignored
     * @returns an ID of the worker group to be used in launch()
     */
    static generateGroupId(domain: WorkerDomain, hint?: Array<WorkerId>): WorkerGroupId {
        return WorkerGroup.generateGroupIdImpl(domain.valueOf(), hint ?? Array<WorkerId>());
    }

     /**
     * The native backend for generateGroupId
     *
     * @param domain the worker domain
     * @param hint optional worker id, the usage is domain-specific (e.g. ignored for MAIN, used as is for EXACT_ID)
     *
     * @returns an ID of the worker group to be used in launch()
     */
    private static native generateGroupIdImpl(domain: int, hint: Array<int>): FixedArray<long>;
}

/**
 * Wait for the completion of an asynchronous callback and returns its result.
 *
 * @param asyncCallback Asynchronous callback function that returns a promise.
 *
 * @returns The resolved value from the promise returned by the callback.
 *
 * @throws The rejection error if the promise is rejected.
 */
export function waitForCompletion<T>(asyncCallback: () => Promise<T>): T {
    let resultPromise = asyncCallback();
    let job = new CompletableJob<T>();
    resultPromise.then((res: T) => {
        job.finish(res);
    }).catch((e: Error) => {
        job.fail(e);
    })
    return job.Await();
}

function launchInternal<R, F extends CoroFunT<never, R>>(coroFun: F, args: FixedArray<Any>, launchParams?: LaunchParams): Job<R> {
  if (launchParams) {
    return launchInternalJobNative<F, R>(coroFun, args as FixedArray<Any>, launchParams.abortFlag, launchParams.workerGroupId);
  }
  return launchInternalJobNative<F, R>(coroFun, args as FixedArray<Any>, false, WorkerGroup.ANY_ID);
}

native function launchInternalJobNative<F, R>(coroFun: F, args: FixedArray<Any>, abortFlag: boolean, gid: WorkerGroupId): Job<R>;
