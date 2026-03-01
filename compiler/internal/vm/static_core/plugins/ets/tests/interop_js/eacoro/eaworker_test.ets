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

function RunTasksTest(): void {
    let mainId = CoroutineExtras.getWorkerId();
    let w = new EAWorker(true);
    w.start();
    let p1 = w.run<int>(():int => {
        let runId = CoroutineExtras.getWorkerId();
        arktest.assertNE(runId, mainId);
        return runId;
    });
    let p2 = w.run<int>(():int => {
        let runId = CoroutineExtras.getWorkerId();
        arktest.assertNE(runId, mainId);
        return runId;
    });
    w.join();
    let runId1 = p1.Await();
    let runId2 = p2.Await();
    arktest.assertEQ(runId1, runId2);
}

function RunTasksWithJsCallTest(): void {
    let mainId = CoroutineExtras.getWorkerId();
    let w = new EAWorker(true);
    w.start();
    let callback = ():int => {
        let module = JSRuntime.loadModule("./eaworker_test_helpers_js");
        let method = JSRuntime.getPropertyJSValue(module, 'returnTwo');
        let val = JSRuntime.invoke(module, method);
        try {
            arktest.assertEQ(val, 2);
        }
        catch(e) {
            console.log("RunTasksWithJsCallTest failed");
            return 1;
        }
        return 0;
    };

    let p1 = w.run<int>(callback);
    let p2 = w.run<int>(callback);
    w.join();
    let result1 = p1.Await();
    let result2 = p2.Await();
    arktest.assertEQ(result1, 0);
    arktest.assertEQ(result2, 0);
}

function RunTasksWithJsAsyncCallTest(): void {
    let w = new EAWorker(true);
    w.start();
    let callback = ():int => {
        let module = JSRuntime.loadModule("./eaworker_test_helpers_js");
        let modified = false;
        let method = JSRuntime.getPropertyJSValue(module, 'asyncWithAwait');
        let promise = (ESValue.wrap(JSRuntime.invoke(module, method))).toPromise();
        let promise1 = promise.then((x:Object): void => {
            modified = true;
        });

        waitForCompletion(() => promise1);

        try {
            let value = JSRuntime.getPropertyInt(JSRuntime.getGlobal(), 'value');
            arktest.assertEQ(value, 7);
            arktest.assertEQ(modified, true);
        }
        catch(error) {
            console.log("Value modifications failed");
            return 1;
        }

        return 0;
    };

    let p = w.run<int>(callback);
    w.join();
    let result = p.Await();
    arktest.assertEQ(result, 0);
}

function CreateEAWorkerWithoutInterop(): void {
    let eaw = new EAWorker();
    eaw.start();
    let res = false;
    try {
        let callback = () => {
            let module = JSRuntime.loadModule('./eaworker_test_helpers_js');
            let val = JSRuntime.getPropertyJSValue(module, 'returnTwo');
        };
        eaw.run<void>(callback).Await();
    } catch(e) {
        res = true;
        eaw.join();
    }
    arktest.assertEQ(res, true);
}

async function asyncCall(task: () => void) {
    task();
}

function RunSetTimeoutTest(): void {
    let job = new CompletableJob<void>();

    let w = new EAWorker(true);
    w.start();
    let p = w.run<Any>(()=>{
        let p1 = asyncCall(() => {});

        let p2 = p1.then<Any, Any>((value: Any) => {
            setTimeout(() => {
                job.finish();
            }, 10);

        });
    });
    w.join();

    job.Await();
}

function RunNewCoroInSetTimeoutTest(): void {
    let job = new CompletableJob<void>();

    let w = new EAWorker(true);
    w.start();
    let p = w.run<Any>(()=>{
        let p1 = asyncCall(() => {
        });
        let p2 = p1.then<Any, Any>((value: Any) => {
            setTimeout(() => {
                let p3 = asyncCall(()=>{
                    job.finish();
                });
            }, 10);
        });
    });
    w.join();

    job.Await();
}
