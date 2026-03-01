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
 desc:  Issue 16052. Lambda in static method segfault
 tags: []
 ---*/
 function getPI(suffix: string = ""): string { return suffix + "->" + "3.141592653589793" }


 class U {

     static returnPI_A0(): string {
         let piFunc: () => string = (): string => getPI()
         return piFunc()
     }

    static returnPI_A0_as_Const(): string {
        const piFunc: () => string = (): string => getPI()
        return piFunc()
    }

     static returnPI_A1(): string {
         let piFunc: (suffix: string) => string = (suffix: string): string => getPI(suffix)
         return piFunc("Hello")
     }

     static returnPI_A1_as_Const(): string {
         const piFunc: (suffix: string) => string = (suffix: string): string => getPI(suffix)
         return piFunc("Hello")
     }

     static returnPO_AO_Inline(): string {
         const suffix = "Hello->"
         let piFunc: () => string = (): string => { return suffix + "3.141592653589793" }
         return piFunc()
     }
 }

 function main(): void {
    arktest.assertEQ(U.returnPI_A0(), "->3.141592653589793")
    arktest.assertEQ(U.returnPI_A0_as_Const(), "->3.141592653589793")
    arktest.assertEQ(U.returnPI_A1(), "Hello->3.141592653589793")
    arktest.assertEQ(U.returnPI_A1_as_Const(), "Hello->3.141592653589793")
    arktest.assertEQ(U.returnPO_AO_Inline(), "Hello->3.141592653589793")
 }
