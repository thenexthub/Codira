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

let f1 = { status: 'fulfilled', value: 'abc' } as PromiseFulfilledResult<string>;
let r1 = { status: 'rejected' , reason: new Error() } as PromiseRejectedResult;

if ( f1.status !== 'fulfilled' ){
    console.log("Test failed. Expected PromiseFulfilledResult<string> status is fulfilled but got `${f1.status}`");
}
if ( f1.value !== 'abc' ){
    console.log("Test failed. Expected PromiseFulfilledResult<string> value is abc but got `${f1.value}`");
} 
if ( r1.status !== 'rejected' ){
    console.log("Test failed. Expected PromiseRejectedResult status is rejected but got `${r1.status}`");
}
