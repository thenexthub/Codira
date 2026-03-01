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

function GenerateFakeRandomInteger(): Int32Array {
  let resource: Int32Array = new Int32Array([12, 43, 56, 76, 89, 54, 45, 32, 35, 47, 46, 44, 21, 37, 84]);
  return resource;
}

function StringCalculation02() {
    let outterCount: int = 2;
    let innerCount: int = 4;
    let str1: string = "h";
    let str2: string = "j";
    let res: string = "11";
  
    let resources: Int32Array = GenerateFakeRandomInteger();
    for (let i: int = 0; i < outterCount; i++) {
        res += str2 + i;
        for (let j: int = 0; j < innerCount; j++) {
          if (resources[j % res.length.toInt() % 15].toInt() > resources[(j + resources[i % 15].toInt()) % res.length.toInt() % 15].toInt()) {
            res += str1 + j;
          } else {
            res += str1;
          }
        }
    }
  
    return res.length.toInt() - 17
}

function main() {
  arktest.assertEQ(StringCalculation02(), 0, "Wrong result at StringCalculation02");
}
