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
 desc: Conditional expression compile bug

 ---*/

type TL = (() => void) | undefined

class RestOptionalLambdasInMethod {

    static foo(k1: string, k2: string, k3: string, ...lambdas: TL[]): boolean {
        return lambdas[1] == undefined
    }
    foo(k1: string, k2: string, k3: string, ...lambdas: TL[]): boolean {
        return lambdas[1] == undefined
    }
}

type TA = RestOptionalLambdasInMethod

function main(): void {
    let flag: boolean | undefined
    let test: TA = new TA
    arktest.assertTrue(test.foo('', '', '', ((): boolean | undefined => flag)() ? undefined : () => {}, undefined, undefined, () => {}))
    arktest.assertTrue(TA.foo('', '', '', ((): boolean | undefined => flag)() ? undefined : () => {}, undefined, undefined, () => {}))
}
