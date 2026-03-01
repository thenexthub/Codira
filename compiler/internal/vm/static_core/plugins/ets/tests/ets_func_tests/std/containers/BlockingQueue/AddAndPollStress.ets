/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

function main(): int {
    let BlockingQueueTestsuite = new arktest.ArkTestsuite("BlockingQueueAddAndPollStressTest")

    BlockingQueueTestsuite.addAsyncTest("addAndPoll", addAndPoll)
    let res = BlockingQueueTestsuite.run()
    CoroutineExtras.stopTaskpool()
    return res
}

async function addAndPoll() {
    let queueTestIntA = new containers.ArrayBlockingQueue<number>(10000)
    let queueTestIntL = new containers.LinkedBlockingQueue<number>(10000)

    // Add elements
    let addSome = (testBQ: containers.BlockingQueue<number>, start: int) => {
        for (let i = start; i < start + 1000; i++) {
            arktest.assertTrue(testBQ.add(i))
        }
    }

    // Remove elements
    let pollSome = (testBQ: containers.BlockingQueue<number>, start: int) => {
        for (let i = start; i < start + 1000; i++) {
            testBQ.poll()
        }
    }

    // Test body
    let addAndPollStressTest = (testBQ: containers.BlockingQueue<number>) => {
        let arrPromise = new Array<Promise<Any>>(20)
        for (let i = 0; i < 10; i++) {
            arrPromise[i] = taskpool.execute(addSome, testBQ, i * 1000)
        }
        for (let i = 10; i < 20; i++) {
            arrPromise[i] = taskpool.execute(pollSome, testBQ, i * 1000)
        }
        for (let i = 0; i < 20; i++) {
            await arrPromise[i]
        }
        arktest.assertEQ(testBQ.capacity, 10000)
    }

    // Run test with ArrayBlockingQueue
    addAndPollStressTest(queueTestIntA)

    // Run test with LinkedBlockingQueue
    addAndPollStressTest(queueTestIntL)
}
