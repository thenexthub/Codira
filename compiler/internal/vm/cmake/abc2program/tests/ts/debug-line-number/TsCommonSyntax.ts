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

class A {
}

class B {
  x = 1;
}

class C extends A {
}

class D {
  get value(): number {
    return 42;
  }
  set value(v) {
  }
}

class E {
  static x = 1;
}

class F {
  add(): number {
    return 0;
  }
}

class G {
  @TimeLogger
  add(a: number, b: number): number {
    return a + b;
  }
}

// enumeration
class EnumTestWithoutInit{
    value: number;
    constructor() { this.value = 0; }
    Index (a: number, b: number): number {
        enum Sys {
            Index,
            Ablility,
            Callee
        }
        this.value += 1;
        return 0;
    }
}

class EnumTestWithNumber{
    value: number;
    constructor() { this.value = 0; }
    Index (a: number, b: number): number {
        enum Sys {
            Index = 0,
            Ablility = 1,
            Callee = 2
        }
        this.value += 1;
        return 0;
    }
}

class EnumTestWithString{
    value: number;
    constructor() { this.value = 0; }
    Index (a: number, b: number): number {
        enum Sys {
            Index = 'Index',
            Ablility = 'Ablility',
            Callee = 'Callee'
        }
        this.value += 1;
        return 0;
    }
}
