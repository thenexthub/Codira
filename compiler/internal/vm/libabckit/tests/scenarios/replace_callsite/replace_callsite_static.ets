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
class MyClass {
  api(idx: int): int {
    return idx
  }
}

class ApiControl {
  static fixedApi(cls: MyClass, idx: int){
    return (cls.api(idx) + 1)
  }
}

function foo(){
  return -1;
}

function main() {
  let c = new MyClass();
  console.log(c.api(4));
  if(foo() > 0) {
    console.log(c.api(5));
  } else {
    console.log(c.api(3));
  }
}
