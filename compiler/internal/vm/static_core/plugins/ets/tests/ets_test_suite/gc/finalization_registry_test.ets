/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

let removed_object_callbacks: boolean[] = new boolean[5];
// Make it global beacuse optimizer is free to kill local variables
let object1: Object | null = null;
let object2: Object | null = null;
let object3: Object | null = null;
let object4: Object | null = null;
let registry: FinalizationRegistry<String> | null = null;
let unusedRegistry: FinalizationRegistry<String> | null = null;
let emptyArgCallbackRegistry: FinalizationRegistry<Int> | null = null;

function doFullGC() {
    try {
        GC.waitForFinishGC(GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE));
    } catch (e) {
        arktest.assertTrue(false, "Unexpected error during GC");
    }
}

function callback(string: String): void {
    if (string.equals("First object removed")) {
        removed_object_callbacks[0] = true;
    } else if (string.equals("Third object removed")) {
        removed_object_callbacks[2] = true;
    } else if (string.equals("Fifth object removed")) {
        removed_object_callbacks[4] = true;
    } else {
        // Must be unreachable
        arktest.assertTrue(false, "Calling unexpected callback: " + string);
    }
}

function emptyArgCallback(): void {
    removed_object_callbacks[3] = true;
}

function registerTest(registry: FinalizationRegistry<String> | null): void {
    object1 = new Object();
    let object1_token = new Object();
    let object1_callbackArg = new String("First object removed");
    object2 = new Object();
    let object2_token = new Object();
    let object2_callbackArg = new String("Second object removed, it is an error");
    object3 = new Object();
    let object3_callbackArg = new String("Third object removed");
    let unregistered_token = new Object();
    registry!.register(object1!, object1_callbackArg, object1_token);
    registry!.register(object2!, object2_callbackArg, object2_token);
    registry!.register(object3!, object3_callbackArg);

    unusedRegistry = new FinalizationRegistry<String>(callback);
    emptyArgCallbackRegistry = new FinalizationRegistry<Int>(emptyArgCallback);
    emptyArgCallbackRegistry!.register(object1!, 2, object1_token);

    // Try to unregister unknown tokens:
    registry!.unregister(unregistered_token);
    unusedRegistry!.unregister(object1!);
    unusedRegistry!.unregister(unregistered_token);

    // Check that we don't call any callbacks here
    doFullGC();

    // Unregister object2, remove unusedRegistry
    // and check that we don't call object2 callback
    unusedRegistry = null;
    registry!.unregister(object2_token);
    arktest.assertNE(object2, null, "Object 2 is alive");
    object2 = null;
    doFullGC();

    // Clear objects and check correct callback calls:
    arktest.assertNE(object1, null, "Object 1 is alive");
    arktest.assertNE(object3, null, "Object 3 is alive");
    object1 = null;
    object3 = null;
    doFullGC();

    // Test cleanup for deleted token
    object4 = new Object();
    let object4_callbackArg = new String("Fifth object removed");
    {
        let object4_token = new Object();
        registry!.register(object4!, object4_callbackArg, object4_token);
    }
    object4 = null;
    doFullGC();
}


let jobsNoToken = new Array<CompletableJob<void> | undefined>(undefined, undefined, undefined);
let resultsNoToken = new Array<int>();
let finRegNoToken = new FinalizationRegistry<int>((i: int) => {
    resultsNoToken.push(i);
    jobsNoToken[i]!.finish();
});
function registerNoTokenTest() {

    for (let seq of [[0, 1, 2], [0, 2, 1], [1, 0, 2], [1, 2, 0], [2, 0, 1], [2, 1, 0]]) {
        arktest.assertEQ(finRegNoToken.getSize(), 0);
        arktest.assertEQ(resultsNoToken.length, 0);

        // Fill finReg
        let objects = new Array<Object>({}, {}, {});
        finRegNoToken.register(objects[0], 0);
        finRegNoToken.register(objects[1], 1);
        finRegNoToken.register(objects[2], 2);

        for (let i of seq) {
            jobsNoToken[i] = new CompletableJob<void>();
        }

        // Cleanup finReg in given order
        for (let i of seq) {
            objects[i] = {};
            doFullGC();
            jobsNoToken[i]!.Await();

            arktest.assertEQ(resultsNoToken.length, 1);
            arktest.assertEQ(resultsNoToken[0], i);
            resultsNoToken.pop();
            for (let i = 0; i < 2; i++) {
                doFullGC();
                Coroutine.Schedule();
            }
            arktest.assertEQ(resultsNoToken.length, 0);
        }
    }
}

//========================= CTOR tests ===========================================
let is_empty = false;
let is_param = false;

let registryEmpty: FinalizationRegistry<String> | null = null;
let registryParams: FinalizationRegistry<String> | null = null;

function finalizationEmptyCtorTestHelper(): void {
    registryEmpty = new FinalizationRegistry<String>((): void => {
        is_empty = true;
    });

    let o2 = new Object();

    registryEmpty!.register(o2, "", o2);

}

function finalizationRegistryConstructorTestParamsHelper(): void {  // void {
    let o1 = new Object();

    registryParams = new FinalizationRegistry<String>((string: String) => {
        is_param = true;
    });
    registryParams!.register(o1, "RegisterObject", o1);
}

function ctorTests() {
    finalizationEmptyCtorTestHelper();
    finalizationRegistryConstructorTestParamsHelper();

    doFullGC();
    Coroutine.Schedule();

    arktest.assertTrue(is_empty, "The callback with empty ctor was not called");
    arktest.assertTrue(is_param, "The callback with parameters ctor was not called");

}
//==================== GetSize Tests

function getSizeTests(): void {

    let finreg = new FinalizationRegistry<String>((string: String) => { console.log("Size callback"); });
    arktest.assertEQ(0, finreg!.getSize(), "Start size of finreg is not empty");

    let o1 = new Object();
    let o2 = new Object();
    let rem_token = new Object();

    finreg!.register(o1!, "first_object", o1!);
    finreg!.register(o2!, "second_object", o2!);

    //add two objects - size must be size == 2
    arktest.assertEQ(finreg!.getSize(), 2, "finalizationGetSizeTest: size after register invalid");
    //add for  objects with same token - size must be size == 4 + 2
    finreg!.register(new Object, "first_dummy", rem_token);
    finreg!.register(new Object, "second_dummy", rem_token);

    arktest.assertEQ(finreg!.getSize(), 2 + 2, "finalizationGetSizeTest: size is invalid");

    finreg!.unregister(rem_token);
    arktest.assertEQ(finreg!.getSize(), 2, "finalizationGetSizeTest: size after register invalid");

    finreg!.unregister(o2!);
    finreg!.unregister(o1!);
    arktest.assertEQ(0, finreg.getSize(), "finalizationGetSizeTest: Size is invalid");

}
//=========================== Unregister tests ====================================
let done_callbacks: boolean[] = new boolean[2];
let registryUnreg: FinalizationRegistry<String> | null = null;
let tokenUse: Object | null = null;
let tokenUnuse1: Object | null = null;
let tokenUnuse2: Object | null = null;
let counterUnreg = 0;

function callBackUnreg(string: String): void {
    counterUnreg = counterUnreg + 1;
}


function unregisterHelper(reg: FinalizationRegistry<string> | null) {

    for (let i = 0; i < done_callbacks.length; i++) {
        done_callbacks[i] = false;
    }

    let o1 = new Object();
    let o2 = new Object();

    reg!.register(o2, "second", o2);
    reg!.register(o1, "first", o1);

    reg!.unregister(o2);


}

function unregisterHelper1() {
    registryUnreg!.register(new Object(), "First Unused_Object", tokenUnuse1!);
    registryUnreg!.register(new Object(), "First_use_Object", tokenUse!);

    registryUnreg!.register(new Object(), "Second_use_Object", tokenUse!);
    registryUnreg!.register(new Object(), "Second Unused Object", tokenUnuse1!);

    registryUnreg!.unregister(tokenUnuse1!);

}

function unregisterHelper2() {
    registryUnreg!.register(new Object(), "Third_Use_Object", tokenUse!);
    registryUnreg!.register(new Object(), "Fourth_Use_Object", tokenUse!);

    registryUnreg!.register(new Object(), "Third_Unused_Object", tokenUnuse2!)
    registryUnreg!.register(new Object(), "Fourth_Unused_Object", tokenUnuse2!);

    registryUnreg!.unregister(tokenUnuse2!);
}

let regUnreg: FinalizationRegistry<string> | null = null;

function unregisterTests() {

    regUnreg = new FinalizationRegistry<String>((string: String) => {
        if (string.equals("first")) {
            done_callbacks[0] = true;
        } else if (string.equals("second")) {
            done_callbacks[1] = true;
        }
        else {
            arktest.assertTrue(false, "This code not must be called ");
        }
    });

    unregisterHelper(regUnreg!);
    doFullGC();
    Coroutine.Schedule();

    arktest.assertTrue(done_callbacks[0], "Callback first must  called! ");
    arktest.assertFalse(done_callbacks[1], "Callback second should not  be called! ");

    tokenUse = new Object();
    tokenUnuse1 = new Object();
    tokenUnuse2 = new Object();

    registryUnreg = new FinalizationRegistry<String>((string: String) => {
        counterUnreg++;
    });

    unregisterHelper1();
    unregisterHelper2();

    doFullGC();
    Coroutine.Schedule();

    arktest.assertEQ(4, counterUnreg, "Counter for calling of unregister is not valid");

}

// If weak reference is dead, it's finalization registry will be placed to finalization queue
// This test tests such case:
// 1) Queue during first GC: finReg1->finReg2
// 2) Queue during second GC: finReg2->finReg1
function FinRegReenqueueTest(): void {
    const registerGarbage = (finReg1: FinalizationRegistry<int>, finReg2: FinalizationRegistry<int>, count: int, tokens: Array<Object>) => {
        for (let i = 1; i < count; i++) {
            finReg1.register({}, i, tokens[i]);
        }
        finReg2.register({}, 0, tokens[0]);
    }

    const COUNT: int = 1000;
    let tokens = new Array<Object>();
    for (let i = 0; i < COUNT; ++i) {
        tokens.push({});
    }

    let jobs = new Array<CompletableJob<void>>();
    let finReg1 = new FinalizationRegistry<int>((i: int) => {
        jobs[i]!.finish();
    });
    let finReg2 = new FinalizationRegistry<int>((i: int) => {
        jobs[i]!.finish();
    });

    for (let i = 0; i < COUNT; ++i) {
        jobs.push(new CompletableJob<void>());
    }

    // Register a lot of garbage in finReg1 and one garbage in finReg2
    registerGarbage(finReg1, finReg2, COUNT, tokens);
    doFullGC();
    for (let i = 0; i < COUNT; ++i) {
        jobs[i]!.Await()
    }

    for (let i = 0; i < COUNT; ++i) {
        jobs[i] = new CompletableJob<void>();
    }

    // Register a lot of garbage in finReg2 and one garbage in finReg1
    registerGarbage(finReg2, finReg1, COUNT, tokens);
    doFullGC();
    for (let i = 0; i < COUNT; ++i) {
        jobs[i]!.Await()
    }
}

function main(): void {
    for (let i = 0; i < removed_object_callbacks.length; i++) {
        removed_object_callbacks[i] = false;
    }
    registry = new FinalizationRegistry<String>(callback);
    registerTest(registry);
    // Check that we successfully remove FinalizationRegistry instance
    // elemets and this cause no error during collection.
    doFullGC();
    // We need to switch from main coroutine to coroutine created by FinalizationRegistry because 
    // if main coro is executed first, the test will return false
    Coroutine.Schedule()

    // Check that all callbacks have been called correctly.
    arktest.assertTrue(removed_object_callbacks[0], "First callback hasn't been called");
    arktest.assertFalse(removed_object_callbacks[1], "Second callback has been called");
    arktest.assertTrue(removed_object_callbacks[2], "Third callback hasn't been called");
    arktest.assertTrue(removed_object_callbacks[3], "Forth callback hasn't been called");
    arktest.assertTrue(removed_object_callbacks[4], "Fifth callback hasn't been called");
    arktest.assertNE(registry, null, "Instance still alive");

    ctorTests();
    getSizeTests();
    unregisterTests();

    registerNoTokenTest();
    FinRegReenqueueTest();
}
