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
desc: 05.Generics
name: 05.generics/genericTypeArgumentWithDefaultParameterValue_05
tags: []
---*/

class Helper {}

class A<T> {
    data: T
    isData: boolean
    constructor(isData: boolean) {
        this.isData = isData
    }
}

class B<T> {
    data: T
    constructor(data: T) {
        this.data = data
    }
    foo<T>(isData: boolean = false): A<T> {
        return new A<T>(isData)
    }
}

function main(): void {
    let b = new B<String>("hello")
    let a = b.foo<Helper>();
    arktest.assertTrue( a instanceof A)
    arktest.assertEQ( a.isData, false)
    a.data = new Helper()
    arktest.assertTrue( a.data instanceof Helper)
}

