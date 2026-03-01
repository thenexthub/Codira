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
desc: A rest parameter allows to make a function or method that take an unbounded
  number of arguments. A rest parameter is marked with ... symbol before parameter
  name.
name: spec/rest-params/Issue14187.ets
tags:
- compile-only
- negative
---*/

interface Supplier {
    get(): int;
}

class SupplierImpl implements Supplier {
    override get(): int {
        return 3;
    }
}

class Base {
    public spread(): int {
        return 1;
    }
    public call(...values: Supplier[]): int {
        return 13;
    }
}

class Testee extends Base {
    public call(prefix: String, ...values: Supplier[]): int {
        return 0;
    }
}

function main(): int {
    let test = new Testee();

    let result: int = test.spread();
    if(result != 1) return 1;

    result = test.call(new SupplierImpl());
    if(result != 3) // return 1;

    let si: Supplier = new SupplierImpl();
    result = test.call("Hello", si, new SupplierImpl(), null as Supplier);


    return 0;

}
