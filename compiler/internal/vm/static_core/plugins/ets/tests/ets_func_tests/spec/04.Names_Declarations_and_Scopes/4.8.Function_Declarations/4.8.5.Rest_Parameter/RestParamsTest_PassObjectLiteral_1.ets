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
name: spec/rest-params/RestParamsTest_PassObjectLiteral_1.ets

---*/

interface Supplier {
    get(): int;
}

class SupplierImpl implements Supplier {
    override get(): int {
        return 3;
    }
}

class SupplierImplForShadow implements Supplier {
    override get(): int {
        return 2;
    }
}

type SupplierN = Supplier|null;

class Base {
    public spread(): int {
        return 1;
    }

    public call(...values: SupplierN[]): int {
        let sum: int = 0;
        for(let call of values) {
            if(call != null) sum += call.get();
        }
        return sum;
    }
}

class TupleT {
    index: int;
    name: String = "";
}

class Testee extends Base {

    public spread(prefix: String, ...values: Object[]): int {
        return 42;
    }


}

function main(): int {

    let test = new Testee();
    let result: int = 0;

    let tu: TupleT = {index: 11, name: "Jane"};
    result = test.spread("Single Tuple", tu);
    if(result != 42) return 1;

    return 0;

}
