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

print("create promise");
let promise = new Promise((resolve,reject) => {
    resolve("resolve promise")
})
promise.then(result => {
    print(result);
})
promise.finally(() => {
    print("execute promise.finally");
})

print("create promise1");
let promise1 = new Promise((resolve,reject) => {
    reject("reject promise1")
})
promise1.catch(error => {
    print(error);
})
promise1.finally(() => {
    print("execute promise1.finally");
})

print("create promise2");
let promise2 = new Promise((resolve,reject) => {
    resolve("resolve promise2")
})

print("create promise3");
let promise3 = new Promise((resolve,reject) => {
    resolve("resolve promise3")
})
promise2.then(result => {
    print(result);
    promise3.then(result => {
        print(result);
    })
})

async function test() {
    print("test start");
    await test1();
    print("test end");
}

async function test1() {
    print("test1 start");
    await test2();
    print("test1 end");
}

async function test2() {
    print("test2 start");
    await test3();
    print("test2 end");
}

function test3() {
    print("execute test3");
}

print("execute test");
test();