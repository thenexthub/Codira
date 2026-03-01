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
desc: Issue 15278  TypeError Member type must be the same for all union objects.
tags: []
---*/

interface Common {
    apply(data: number[])
}

class McOne implements Common {
    apply(data: number[]) {}
}

class McTwo implements Common {
    apply(data: number[]) {}
}

class McThree implements Common {
    apply(data: number[]) {}
}

type UniversalArray = McOne | McTwo | McThree

function callMe(data: number[], ua: UniversalArray) {
    ua.apply(data)
}

function main() {
    let ua: UniversalArray  = new McOne
    let data = [1.0, 2.9]

    callMe(data, ua)
}


