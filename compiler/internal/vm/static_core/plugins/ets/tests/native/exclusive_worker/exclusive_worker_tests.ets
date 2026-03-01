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

import { launch } from "std/concurrency"

let workerId = -1;
let counter = 0;

function setWorkerId(eworkerId: int)
{
    workerId = eworkerId;
}

function checkWorkerId(): boolean
{
    return CoroutineExtras.getWorkerId() == workerId;
}

// CallMethod
function call() {
    return checkWorkerId();
}

// AsyncCallMethod
function asyncCall(): boolean {
    let l = async (): Promise<boolean> => {
        return Promise.resolve(call());
    }
    return waitForCompletion<boolean>(l);
}

// LaunchCallMethod
function launchCall(): boolean {
    let p = launch<boolean, () => boolean>(call);
    return p.Await();
}

// ConcurrentWorkerAndRuntimeDestroy
class Event {
    constructor() {
        this.promise = new Promise<Number>((resolve: (val: Number) => void) => {
            this.resolveFn = resolve;
        })
    }

    public wait() {
        waitForCompletion(() => this.promise);
    }

    public fire() {
        this.resolveFn!(0);
    }

    private promise: Promise<Number>;
    private resolveFn: ((val: Number) => void) | null = null;
}

let mainEvent = new Event();
let eWorkerAsyncEvent = new Event();
let eWorkerLaunchEvent = new Event();

function mainRoutine() {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    let l = () => {
        eWorkerLaunchEvent.wait();
        mainEvent.fire();
    }
    launch<void, () => void>(l);
}

function eWorkerRoutine(): boolean {
    async () => {
        eWorkerAsyncEvent.wait();
    }();

    let l = () => {
        eWorkerLaunchEvent.fire();
        mainEvent.wait();
        eWorkerAsyncEvent.fire();
    }
    launch<void, () => void>(l);
    return true;
}

function add(a: int) {
    counter += a;
}

function clearCounter() {
    counter = 0;
}

function scheduleACoroutine(): boolean {
    clearCounter();
    launch<void, ()=>void>(():void => {
        add(5);
    });
    Coroutine.Schedule();
    return counter == 5 && call();
}

async function asyncAdd(a: int) {
    launch<void, ()=>void>(():void => {
        add(6);
    });
    counter += a;
}

function scheduleAJCoroutine(): boolean {
    clearCounter();
    asyncAdd(6);
    Coroutine.Schedule();
    return counter == 12 && call();
}

function launchCallWithoutAwait(): Job<boolean> {
    return launch<boolean, ()=>boolean>(call) as Job<boolean>;
}

// recursive launch
function recursiveLaunch(): boolean {
    let res : Job<Job<boolean>> = launch<Job<boolean>, ()=>Job<boolean>>(launchCallWithoutAwait) as Job<Job<boolean>>;
    return res.Await().Await();
}

async function callAsync(): Promise<boolean> {
    return checkWorkerId();
}

async function asyncCallWithoutAwait(): Promise<Promise<boolean>> {
    return callAsync();
}

// recursive async functions
function recursiveAsync(): boolean {
    let res : Promise<Promise<boolean>> = asyncCallWithoutAwait();
    return waitForCompletion<boolean>(async(): Promise<boolean> => await res);
}

function launchAsyncFunctions(): Promise<boolean> {
    return callAsync();
}

function ACoroutineCallAsyncFunctions(): boolean {
    let res : Job<Promise<boolean>> = launch<Promise<boolean>, ()=>Promise<boolean>>(launchAsyncFunctions) as Job<Promise<boolean>>
    return waitForCompletion<boolean>(() => res.Await());
}

async function asyncFunctionLaunchCall() {
    return launch<boolean, ()=>boolean>(call);
}

function asyncFunctionLaunchACoroutine(): boolean {
    let res : Promise<Job<boolean>> = asyncFunctionLaunchCall();
    return waitForCompletion<Job<boolean>>(() => res).Await();
}

// oom
function body(): Object {
    return new Object();
}

function testOOM(): boolean {
    try {
        const veryBigNumberOfCoroutines: int = 500000000;
        for (let i = 0; i < veryBigNumberOfCoroutines; ++i) {
            launch<Object, ()=>Object>(body);
        }
    } catch (e) {
        if (e instanceof CoroutinesLimitExceedError) {
            StdDebug.Logger.log("Successfully caught: " + e);
            return true;
        } else {
            StdDebug.Logger.log("Wrong exception thrown: " + e);
            return false;
        }
    }
    StdDebug.Logger.log("No exception thrown but should be!");
    return false;
}


function test_stack_overflow(): boolean {
    try {
        launch<Object | null, ()=>Object | null>(coro_stack_overflow).Await();
        console.println("No exceptions thrown by coro_stack_overflow() but should be!")
        return false;
    } catch (e) {
        if (!(e instanceof StackOverflowError)) {
            console.println("Expected StackOverflowError but another exception has been thrown!");
            return false;
        }
        return true;
    }
}

function stack_eater(): void {
    stack_eater();
}

function coro_stack_overflow(): Object | null {
    stack_eater();
    return null;
}

function coro_error(): Object {
    throw new Error();
}

function test_error(): boolean {
    try {
        let res = launch<Object, ()=>Object>(coro_error);
        res.Await();
        console.println("No exceptions thrown by coro_error() but should be!")
        return false;
    } catch (e) {
        if (!(e instanceof Error)) {
            console.println("Expected Error but another exception has been thrown!");
            return false;
        }
        return true;
    }
}

// throw exceptions
function throwExceptions(): boolean {
    if (!test_stack_overflow()) {
        return false;
    }
    if (!test_error()) {
        return false;
    }
    return true;
}

let e2m: containers.LinkedBlockingQueue<boolean> = new containers.LinkedBlockingQueue<boolean>();
let m2e: containers.LinkedBlockingQueue<boolean> = new containers.LinkedBlockingQueue<boolean>();

function EACoroutineSendToMain(): boolean {
    launch<void, ()=>void>(():void => {
        e2m.push(true);
    });
    return m2e.pop();
}

function mainSendToEACoroutine(){
    if (e2m.pop()) {
        m2e.push(true);
    }
}
