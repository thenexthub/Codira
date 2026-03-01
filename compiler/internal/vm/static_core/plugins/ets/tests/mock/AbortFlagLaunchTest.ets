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

function Failing(): int {
    throw new Error('Failed');
}

function SetTimeoutTest(): void {
    setTimeout(Failing);
}

function AbortTest(): void {
    const params: LaunchParams = {abortFlag: true,  workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.MAIN)};
    try {
        let j = launch<int, () => int>(Failing, params);
        j.Await();
    } catch (e) {
    }
}

function NoAbortTest(): void {
    const params: LaunchParams = {abortFlag: false,  workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.MAIN)};
    try {
        let j = launch<int, () => int>(Failing, params);
        j.Await();
    } catch (e) {
    }
}

export function main(args: FixedArray<string>): int {
    let type: string = args[0];
    if (type == 'SetTimeout') {
        SetTimeoutTest();
    } else if (type == 'Abort') {
        AbortTest();
    } else if (type == 'NoAbort') {
        NoAbortTest();
    } else {
        let procManager = new StdProcess.ProcessManager();
        procManager.exit(2);
    }
    return 0;
}
