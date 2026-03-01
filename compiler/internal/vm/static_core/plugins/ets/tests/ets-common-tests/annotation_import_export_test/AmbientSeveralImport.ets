/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

/*---
  files: [AmbientAnnotationsSrc.ets]
---*/

import {ClassAuthor, Color} from "./AmbientAnnotationsSrc"

@interface ClassAuthor {
   authorName: string = "Jim"
   authorAge: number = -35
   testBool: boolean = false
   favorColor: Color = Color.BLUE
   color: Color[] = [Color.RED, Color.BLUE]
   reviewers: string[] = ["Bob", "Jim", "Tom"]
   reviewersAge: double[] = [18, 21, 32]
   testBools: boolean[] = [false, true, false]
   mutiArray: number[][] = [
       [1, 2, 3],
       [4, +5, 6],
       [7, 8, -9]
   ]
}

@ClassAuthor()
class MyClass {
   @ClassAuthor()
   foo(){}
}

@ClassAuthor()
function main(){}

