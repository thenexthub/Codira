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

 /*---
    tags: [no-warmup]
 ---*/

function testInit() {
    let str = 'workerLocal';
    let wl = new WorkerLocal<string>(() => str);
    arktest.assertEQ(wl.get(), str);
    wl.set('main');
    arktest.assertEQ(wl.get(), 'main');
    await taskpool.execute(() => {
        arktest.assertEQ(wl.get(), str);
        wl.set('subWorker');
        arktest.assertEQ(wl.get(), 'subWorker');
        wl.delete();
        arktest.assertEQ(wl.get(), str);
    })
    arktest.assertEQ(wl.get(), 'main');
}

async function testWithoutInit() {
    let wl = new WorkerLocal<int>();
    arktest.expectError(() => {
        wl.get();
    }, new Error('WorkerLocal value not initialized. Call set() first or provide an init function.'))
    wl.set(1);
    arktest.assertEQ(wl.get(), 1);
    wl.set(wl.get() + 1);
    arktest.assertEQ(wl.get(), 2);
    await taskpool.execute(() => {
        arktest.expectError(() => {
            wl.get();
        }, new Error('WorkerLocal value not initialized. Call set() first or provide an init function.'))
        wl.set(1);
        arktest.assertEQ(wl.get(), 1);
        wl.set(wl.get() + 1);
        arktest.assertEQ(wl.get(), 2);
        wl.delete()
        arktest.expectError(() => {
            wl.get();
        }, new Error('WorkerLocal value not initialized. Call set() first or provide an init function.'))
    })
    arktest.assertEQ(wl.get(), 2);
}

class TestObject{
     constructor(){
     }

     constructor(a1:string,a2:int){
         this.a1= a1;
         this.a2=a2;
     }

     a1 = 'a1';
     a2 = 0;
}

function testWithMultiInit() {
    let wl = new WorkerLocal<TestObject>(() => new TestObject('a0', 0));
    arktest.assertEQ(wl.get().a1, 'a0');
    arktest.assertEQ(wl.get().a2, 0);
    for(let i = 0; i < 50; i++){
        arktest.assertEQ(wl.get().a1, 'a0');
        arktest.assertEQ(wl.get().a2, 0);
        for(let i = 0; i < 20; i++){
            wl.set(new TestObject('a' + i, i));
            arktest.assertEQ(wl.get().a1, 'a' + i);
            arktest.assertEQ(wl.get().a2, i);
            wl.delete();
        }
        arktest.assertEQ(wl.get().a1, 'a0');
        arktest.assertEQ(wl.get().a2, 0);
    }
}

function testWithworkerInit(){
    let initCount = 0;
    let wl =new WorkerLocal<TestObject>(() =>{
        initCount++;
        return new TestObject('a0', initCount);
    });
    let eaw = new EAWorker();
    eaw.start();
    let job = eaw.run<void>(()=>{
        let v1 = wl.get();
        arktest.assertEQ(v1.a1, 'a0');
        let v2 = wl.get();
        arktest.assertEQ(v2, v1);
    });
    job.Await();
    arktest.assertEQ(initCount, 1);
    eaw.quit();
}

function main() {
    const testSuite = new arktest.ArkTestsuite("WorkerLocalTest");
    testSuite.addTest('testInit', testInit);
    testSuite.addAsyncTest('testWithoutInit', testWithoutInit);
    testSuite.addTest('testWithMultiInit', testWithMultiInit);
    testSuite.addTest('testWithworkerInit', testWithworkerInit);
    testSuite.run();
    CoroutineExtras.stopTaskpool();
}
