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

function getArray(): FixedArray<(string | null)> {
    let a: FixedArray<(string | null)> = [null, null, 'Hello World!']
    return a
}

const string1 = 'New String 1!'
const string2 = 'New String 2!'

function getNewString1(): string {
    return string1
}

function getNewString2(): string {
    return string2
}

function checkArray(array: FixedArray<(string | null)>): boolean {
    return array[0] == string1 && array[1] == null && array[2] == string2
}

function getObject(): Object {
    return new Object()
}
