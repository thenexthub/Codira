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

function asyncLockTimeoutTest(){
    let tag = 'asyncLock'
    let count: int = 0
    let lock = AsyncLock.request(tag)
    lock.lockAsync(async () => {
        await new Promise<number>((resolve) => {
            setTimeout(() => {
                resolve(1)
            }, 200)
        })
    }).then(() => {
        console.log('first long time task')
        count++;
    }).catch((e: Error) => {
        console.log('first long time task catch fail:' + e)
    })
    lock.lockAsync(async () => {
        console.log('second normal task')
    })
    let options = new AsyncLockOptions()
    options.timeout = 100
    lock.lockAsync(async () => {
        arktest.assertTrue(false)
    }, AsyncLockMode.EXCLUSIVE, options).catch((e: Error) => {
        console.log('third timeout task: ' + e)
        count++;
    })

    await new Promise<number>((resolve) => {
        setTimeout(() => {
            resolve(1)
        }, 500)
    })
    arktest.assertEQ(count, 2)
}

function main() {
    let asyncLockTestsuite = new arktest.ArkTestsuite("asyncLockTestsuite");
    asyncLockTestsuite.addTest('asyncLockTimeoutTest',asyncLockTimeoutTest); 
    return asyncLockTestsuite.run();
}
