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
name: spec/rest-params/RestParamsTest_PassLambdas_1.ets

---*/

class Base {

}

type StringToInt = (val: String) => int;

class Testee extends Base {

    public callLambda(prefix: String, ...lValues: StringToInt[]): int {
        let sum: int = 0;
        for(let call of lValues) {
            if(call != null) sum += call(prefix);
        }
        return sum;
    }

    public callLambda(...lValues: StringToInt[]): int {
        let sum: int = 0;
        for(let call of lValues) {
            if(call != null) sum += call("Hello");
        }
        return sum;
    }


}

function main(): int {

    let test = new Testee();
    let result: int = 0;

    let l1: StringToInt = (str: String) => { return str.length as int }
    result = test.callLambda("Hello", l1, l1, (str: String) => { return str.length as int }, null as StringToInt);
    if(result != 15) return 1;

    return 0;

}
