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
desc: Issue 24747. Switch case
tags: []
---*/

type Func = () => void

let rc = 0;

class A {
  bar() {}
  foo(v: int) {
    switch (v) {
      case 2: {
        const x = 2;
        const y: number = 2.0;
        const c = "Case 2"
        const f: Func = () => { rc = v }
        f()
        break
      }

      case 1: {
        const x = 1;
        const y: number = 1.0;
        const c = "Case 1"
        const f: Func = () => { rc = v }
        f()
        break
      }

      default: {
        const x = 3;
        const y: number = 3.0;
        const c = "Case 3"
        const f: Func = () => { rc = v }
        f()
      }
    }
  }
}

function main() {
  rc = 0;
  new A().foo(-1)
  arktest.assertEQ(-1, rc)
}
