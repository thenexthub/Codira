/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

{% for c in cases %}

/*---
desc: >-
      Note. None of the operations above invokes a static initialization recursively if the static initialization
      of the same entity is not complete.
---*/

let strseq = ""
function seq(i: string) {
  strseq += i
}

class A {
  static {
    seq("A")
  }
}

class B extends A {
  static {
    seq("B")
    throw new Error("initialization has stopped")
  }
}

class C extends B {
  static s: int
  f: int = 0
  static {
    seq("C")
  }
  static foo() {}
  foo() {}
}

class D extends C {
  static {
    seq("D")
  }
}

let firstrun = true

function main() {
  let res = false
  try {
    {{c}}
  } catch (e) {
    res = true
  }
  arktest.assertTrue(res)
  arktest.assertEQ(strseq, "AB")
  firstrun = false
}
{% endfor %}
